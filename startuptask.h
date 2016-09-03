#ifndef STARTUPTASK_H
#define STARTUPTASK_H

#include <QThread>

#include <Windows.h>

class StartupTask : public QThread
{
    Q_OBJECT
public:
    explicit StartupTask(QObject *parent = 0);

signals:

public slots:

protected:
    void run();
private:

};

#endif // STARTUPTASK_H
