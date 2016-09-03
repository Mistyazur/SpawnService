#include "qtservice.h"
#include "startuptask.h"
#include <QCoreApplication>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>

#define SERVICE_NAME	"Spawn Service"
#define SERVICE_DESC	"Service spawn procecsses in user session with SYSTEM privilege"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QFile *logFile = nullptr;
    if (logFile == nullptr) {
        WCHAR szAppFileName[MAX_PATH];
        GetModuleFileName(NULL, szAppFileName, MAX_PATH);

        QString &appFileDirName = QString::fromWCharArray(szAppFileName).replace("\\", "/");
        QString &appFileDir = appFileDirName.left(appFileDirName.lastIndexOf("/"));
        QString logName = appFileDir + "/"
                + QCoreApplication::applicationName()
                + QDateTime::currentDateTime().toString("_MMddyyyy-hhmmss")
                + ".log";
        logFile = new QFile(logName);
        if (!logFile->open(QFile::ReadWrite)) {
            delete logFile;
            return;
        }
    }

    QTextStream ts(logFile);
    QByteArray &localMsg = msg.toLocal8Bit();
    QString &logTime = QDateTime::currentDateTime().toString("[MM/dd hh:mm:ss.zzz]");
    switch (type) {
    case QtDebugMsg:
        ts<<"[D]"<<logTime<<" "<<localMsg.constData()<<"\r\n";
        break;
    case QtInfoMsg:
        ts<<"[I]: "<<logTime<<" "<<localMsg.constData()<<"\r\n";
        break;
    case QtWarningMsg:
        ts<<"[W]: "<<logTime<<" "<<localMsg.constData()<<"\r\n";
        break;
    case QtCriticalMsg:
        ts<<"[C]: "<<logTime<<" "<<localMsg.constData()<<"\r\n";
        break;
    case QtFatalMsg:
        ts<<"[F]: "<<logTime<<" "<<localMsg.constData()<<"\r\n";
        abort();
    }
}

class SpawnService : public QtService<QCoreApplication>
{
public:
    SpawnService(int argc, char **argv)
        : QtService<QCoreApplication>(argc, argv, SERVICE_NAME)
    {
        setServiceDescription(SERVICE_DESC);
        setStartupType(QtServiceController::AutoStartup);

//        QtServiceBase::Default				0x00	The service can be stopped, but not suspended.
//        QtServiceBase::CanBeSuspended			0x01	The service can be suspended.
//        QtServiceBase::CannotBeStopped		0x02	The service cannot be stopped.
//        QtServiceBase::NeedsStopOnShutdown	0x04	(Windows only) The service will be stopped before the system shuts down.
//        												Note that Microsoft recommends this only for services that must absolutely clean up during shutdown,
//        												because there is a limited time available for shutdown of services.
//        setServiceFlags(QtServiceBase::NeedsStopOnShutdown);
    }

protected:
    void start()
    {
        logMessage("Spawn Service Start.");

        // Thread do task at every logon

        StartupTask *pst = new StartupTask();
        pst->start();
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

    void processCommand(int code)
    {
    }
private:
};


int main(int argc, char *argv[])
{
    qInstallMessageHandler(myMessageOutput);

    SpawnService service(argc, argv);
    return service.exec();
}
