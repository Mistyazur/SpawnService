#include "qtservice.h"
#include "createsysinteractiveprocess.h"
#include <QCoreApplication>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

#define SERVICE_NAME	"Spawn Service"
#define SERVICE_DESC	"Service spawn procecsses in user session with SYSTEM privilege"

class SpawnService : public QtService<QCoreApplication>
{
public:
    SpawnService(int argc, char **argv)
        : QtService<QCoreApplication>(argc, argv, SERVICE_NAME)
    {
        logMessage("Spawn Service Init: begin");
//        setServiceDescription(SERVICE_DESC);
//        setStartupType(QtServiceController::AutoStartup);
//        setServiceFlags(QtServiceBase::CanBeSuspended);
        logMessage("Spawn Service Init: end");
    }

protected:
    void start()
    {
        try
        {
            logMessage("Spawn Service Start: begin");
//            QCoreApplication *app = application();

            QString strDoTask = QString(R"("%1/%2")").arg(QCoreApplication::applicationDirPath()).arg("tasks.cmd");
            CreateSysInteractiveProcess((wchar_t *)strDoTask.toStdWString().data());
            logMessage("Spawn Service Start: end");

//            app->exec();
        }
        catch(...)
        {
            QFile f("D:/sp.txt");
            if(f.open(QFile::ReadWrite | QFile::Append))
            {
                QTextStream s(&f);
                s << QDateTime::currentDateTime().toString() << " start catch\r\n";
                f.close();
            }

            logMessage("Critical", Warning);
        }

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
    try
    {
        SpawnService service(argc, argv);
        return service.exec();
    }
    catch(...)
    {
        QFile f("D:/sp.txt");
        if(f.open(QFile::ReadWrite | QFile::Append))
        {
            QTextStream s(&f);
            s << QDateTime::currentDateTime().toString() << " main catch\r\n";
            f.close();
        }
    }
}
