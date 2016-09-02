#include "qtservice.h"
#include "process.h"
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
        setServiceDescription(SERVICE_DESC);
        setStartupType(QtServiceController::AutoStartup);
//        setServiceFlags(QtServiceBase::CanBeSuspended);
    }

protected:
    void start()
    {
        logMessage("Spawn Service Start.");

        QCoreApplication *app = application();

        QString strDoTask = QString(R"("%1/%2")")
                        .arg(QCoreApplication::applicationDirPath())
                        .arg("tasks.cmd");
        RunAsInteractiveSystem((wchar_t *)strDoTask.toStdWString().data());

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
