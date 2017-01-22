#include <QCoreApplication>
#include "lib/Utils.h"
#include "lib/Socket.h"
#include "lib/epoll/EpollWrap.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    return a.exec();
}

