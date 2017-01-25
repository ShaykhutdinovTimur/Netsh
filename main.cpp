#include <QCoreApplication>
#include "lib/Utils.h"
#include "lib/Socket.h"
#include "lib/epoll/EpollWrap.h"
#include "lib/Daemonisation.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    pid_t pid = makeSelfDaemon();
    writePid(pid, "/home/timur/diap");
    return a.exec();
}

