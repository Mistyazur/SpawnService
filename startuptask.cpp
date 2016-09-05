#include "startuptask.h"
#include "process.h"

#include <QCoreApplication>
#include <QDebug>

StartupTask::StartupTask(QObject *parent) : QThread(parent)
{
}

void StartupTask::run()
{
    // Script path of startup task

    QString strStartupTask = QString(R"("%1/%2")")
            .arg(QCoreApplication::applicationDirPath())
            .arg("StartupTasks.cmd");

    // Run task scrpit at every logon

    DWORD dwPrevError = ERROR_SUCCESS;

    for (;;) {
        msleep(100);

        DWORD dwActiveUserSessionId = WTSGetActiveConsoleSessionId();
        if (dwActiveUserSessionId == 0xFFFFFFFF)
            continue;

        HANDLE hActiveUserToken = NULL;
        if (!WTSQueryUserToken(dwActiveUserSessionId, &hActiveUserToken)) {
            dwPrevError = ::GetLastError();
            continue;
        }
        ::CloseHandle(hActiveUserToken);

        if (dwPrevError == ERROR_NO_TOKEN) {
            // Run as system with UI
            RunAsInteractiveSystem(strStartupTask.toStdWString().c_str());

            // Reset previous error
            dwPrevError = ERROR_SUCCESS;
        }
    }
}

