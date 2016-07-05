#include <QCoreApplication>
#include <QProcess>
#include <Windows.h>
#include "qtservice.h"

DWORD getPidInSession(const wchar_t *process, DWORD sessionId)
{
    DWORD dwRet = 0;
    QProcess p;
    QString cmd = QString("TASKLIST /FI \"IMAGENAME eq %1\" /FI \"SESSION eq %2\" /FO \"list\"").arg(QString::fromWCharArray(process)).arg(sessionId);

    p.start(cmd);
    if (p.waitForFinished())
    {
        QString output(p.readAllStandardOutput());
        QStringList lineList = output.split("\r\n", QString::SkipEmptyParts);
        foreach(QString line, lineList)
        {
            if (line.startsWith("PID", Qt::CaseInsensitive))
            {
                QRegExp rx("PID.*(\\d+)");
                if (rx.indexIn(line) != -1)
                    dwRet = rx.cap(1).toInt();
            }
        }
    }

    return dwRet;
}

BOOL CreateProcessInUserSession(wchar_t *process)
{
    DWORD dwPid = getPidInSession(TEXT("winlogon.exe"), WTSGetActiveConsoleSessionId());

    // obtain a handle to the winlogon process
    HANDLE hProcess = ::OpenProcess(MAXIMUM_ALLOWED, FALSE, dwPid);

    // obtain a handle to the access token of the winlogon process
    HANDLE hToken = NULL;
    if (!::OpenProcessToken(hProcess, TOKEN_DUPLICATE, &hToken))
    {
        CloseHandle(hProcess);
        return FALSE;
    }

    // Security attibute structure used in DuplicateTokenEx and CreateProcessAsUser
    // I would prefer to not have to use a security attribute variable and to just
    // simply pass null and inherit (by default) the security attributes
    // of the existing token. However, in C# structures are value types and therefore
    // cannot be assigned the null value.
    SECURITY_ATTRIBUTES sa = {0};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);

    // copy the access token of the winlogon process; the newly created token will be a primary token
    HANDLE hUserTokenDup = NULL;
    if (!DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, &sa, SecurityIdentification, TokenPrimary, &hUserTokenDup))
    {
        CloseHandle(hProcess);
        CloseHandle(hToken);
        return FALSE;
    }

    // By default CreateProcessAsUser creates a process on a non-interactive window station, meaning
    // the window station has a desktop that is invisible and the process is incapable of receiving
    // user input. To remedy this we set the lpDesktop parameter to indicate we want to enable user
    // interaction with the new process.
    STARTUPINFO si = {0};
    si.cb = sizeof(STARTUPINFO);
    si.lpDesktop = (LPWSTR)TEXT("winsta0\\default"); // interactive window station parameter; basically this indicates that the process created can display a GUI on the desktop
    PROCESS_INFORMATION pi = {0};

    // flags that specify the priority and creation method of the process
    int dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;

    // create a new process in the current user's logon session
    bool result = CreateProcessAsUser(hUserTokenDup,        // client's access token
                                    NULL,                   // file to execute
                                    process,        		// command line
                                    &sa,                 	// pointer to process SECURITY_ATTRIBUTES
                                    &sa,                 	// pointer to thread SECURITY_ATTRIBUTES
                                    FALSE,                  // handles are not inheritable
                                    dwCreationFlags,        // creation flags
                                    NULL,            		// pointer to new environment block
                                    NULL,                   // name of current directory
                                    &si,                 	// pointer to STARTUPINFO structure
                                    &pi		           		// receives information about new process
                                    );

    // invalidate the handles
    CloseHandle(hProcess);
    CloseHandle(hToken);
    CloseHandle(hUserTokenDup);

    return TRUE;
}

class SpawnService : public QtService<QCoreApplication>
{
public:
    SpawnService(int argc, char **argv)
        : QtService<QCoreApplication>(argc, argv, "Spawn Service")
    {
        setServiceDescription("Service spawn procecsses in user session with SYSTEM privilege");
        setServiceFlags(QtServiceBase::CanBeSuspended);
    }

protected:
    void start()
    {
        QCoreApplication *app = application();

        QString strCmd = QString("\"%1/%2\"").arg(QCoreApplication::applicationDirPath()).arg("tasks.cmd");
        wchar_t szCmd[1024] = {0};
        strCmd.toWCharArray(szCmd);
        CreateProcessInUserSession(szCmd);

        app->exec();
    }

    void stop()
    {
    }

    void pause()
    {
    }

    void resume()
    {
    }
private:
};

int main(int argc, char *argv[])
{
    SpawnService service(argc, argv);
    return service.exec();
}
