#include "createsysinteractiveprocess.h"
#include <TlHelp32.h>
#include <Wtsapi32.h>
#include <UserEnv.h>

//static DWORD GetPidInSession(const wchar_t *process, DWORD sessionId)
//{
//    DWORD dwRet = 0;
//    QProcess p;
//    QString cmd = QString("TASKLIST /FI \"IMAGENAME eq %1\" /FI \"SESSION eq %2\" /FO \"list\"").arg(QString::fromWCharArray(process)).arg(sessionId);

//    p.start(cmd);
//    if (p.waitForFinished())
//    {
//        QString output(p.readAllStandardOutput());
//        QStringList lineList = output.split("\r\n", QString::SkipEmptyParts);
//        foreach(QString line, lineList)
//        {
//            if (line.startsWith("PID", Qt::CaseInsensitive))
//            {
//                QRegExp rx("PID.*(\\d+)");
//                if (rx.indexIn(line) != -1)
//                    dwRet = rx.cap(1).toInt();
//            }
//        }
//    }

//    return dwRet;
//}

static DWORD GetSessionProcessId(const wchar_t *process, DWORD sessionId)
{
    HANDLE hProcessSnap;
    DWORD result = NULL;
    DWORD sid = NULL;
    PROCESSENTRY32 pe32 = {sizeof(pe32), };

    // Take a snapshot of all processes in the system.
    hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hProcessSnap)
        return NULL;

    // Retrieve information about the first process,
    // and exit if unsuccessful
    if (::Process32First(hProcessSnap, &pe32))
    {
        do
        {
            if (0 == _wcsicmp(process, pe32.szExeFile))
            {
                ::ProcessIdToSessionId(pe32.th32ProcessID, &sid);
                if (sid == sessionId)
                {
                    result = pe32.th32ProcessID;
                    break;
                }
            }
        } while (::Process32Next(hProcessSnap, &pe32));
    }

    ::CloseHandle(hProcessSnap);

    return result;
}

BOOL CreateSysInteractiveProcess(wchar_t *process)
{
    BOOL bResult = FALSE;
    SECURITY_ATTRIBUTES sa = {0};
    HANDLE hActiveUserToken = NULL;
    HANDLE hActiveUserTokenDup = NULL;
    LPVOID lpUserEnvBlock = NULL;
    HANDLE hProcess = NULL;
    HANDLE hSysInteractiveToken = NULL;
    HANDLE hSysInteractiveTokenDup = NULL;
    STARTUPINFO si = {};
    PROCESS_INFORMATION pi = {};

    // Security attibute structure used in DuplicateTokenEx and CreateProcessAsUser
    // I would prefer to not have to use a security attribute variable and to just
    // simply pass null and inherit (by default) the security attributes
    // of the existing token. However, in C# structures are value types and therefore
    // cannot be assigned the null value.

    sa.nLength = sizeof(SECURITY_ATTRIBUTES);

    // Get active user token.

    DWORD dwActiveUserSessionId = WTSGetActiveConsoleSessionId();
    while (!WTSQueryUserToken(dwActiveUserSessionId, &hActiveUserToken))
    {
        int error = ::GetLastError();
        if (error == ERROR_NO_TOKEN)
            ::Sleep(1000);
        else
            goto CLEANUP;
    }

    // Copy active user token.

    if (!DuplicateTokenEx(hActiveUserToken, MAXIMUM_ALLOWED, &sa, SecurityIdentification, TokenPrimary, &hActiveUserTokenDup))
        goto CLEANUP;

    // Create active user environment.

    if (!CreateEnvironmentBlock(&lpUserEnvBlock, hActiveUserTokenDup, false))
        goto CLEANUP;

    // Get pid of the system process which has interactive access(in user session).

    DWORD dwWinLogonPid;
    do
    {
        dwWinLogonPid = GetSessionProcessId(TEXT("winlogon.exe"), dwActiveUserSessionId);
        ::Sleep(1000);
    } while (dwWinLogonPid == NULL);

    // Obtain a handle to the winlogon process.

    hProcess = ::OpenProcess(MAXIMUM_ALLOWED, FALSE, dwWinLogonPid);
    if (hProcess == NULL)
        goto CLEANUP;

    // Obtain a handle to the access token of the winlogon process.

    if (!::OpenProcessToken(hProcess, TOKEN_DUPLICATE, &hSysInteractiveToken))
        goto CLEANUP;

    // Copy the access token of the winlogon process.
    // The newly created token will be a primary token.

    if (!DuplicateTokenEx(hSysInteractiveToken, MAXIMUM_ALLOWED, &sa, SecurityIdentification, TokenPrimary, &hSysInteractiveTokenDup))
        goto CLEANUP;

    // By default CreateProcessAsUser creates a process on a non-interactive window station, meaning
    // the window station has a desktop that is invisible and the process is incapable of receiving
    // user input. To remedy this we set the lpDesktop parameter to indicate we want to enable user
    // interaction with the new process.

    si.cb = sizeof(STARTUPINFO);
    si.lpDesktop = (LPWSTR)TEXT("winsta0\\default"); // interactive window station parameter; basically this indicates that the process created can display a GUI on the desktop

    // Flags that specify the priority and creation method of the process.

    int dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE | CREATE_UNICODE_ENVIRONMENT;

    // Create a new process in the current user's logon session.

    bResult = CreateProcessAsUser(hSysInteractiveTokenDup,
                                      NULL,
                                      process,
                                      &sa,
                                      &sa,
                                      FALSE,
                                      dwCreationFlags,
                                      lpUserEnvBlock,
                                      NULL,
                                      &si,
                                      &pi
                                      );

CLEANUP:
    if (hActiveUserToken != NULL)
        CloseHandle(hActiveUserToken);

    if (hActiveUserTokenDup != NULL)
        CloseHandle(hActiveUserTokenDup);

    if (lpUserEnvBlock != NULL)
        DestroyEnvironmentBlock(lpUserEnvBlock);

    if (hProcess != NULL)
        CloseHandle(hProcess);

    if (hSysInteractiveToken != NULL)
        CloseHandle(hSysInteractiveToken);

    if (hSysInteractiveTokenDup != NULL)
        CloseHandle(hSysInteractiveTokenDup);

    return bResult;
}
