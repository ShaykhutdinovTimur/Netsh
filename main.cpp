#include <QCoreApplication>
#include "lib/Utils.h"
#include "lib/Socket.h"
#include "lib/epoll/EpollWrap.h"
#include "lib/tcp/TcpServer.h"
#include "lib/tcp/TcpSocket.h"
#include "lib/logic/ExecServer.h"

int main(int argc, char *argv[])
{
    int port = 8888;
    if (argc < 2 || (sscanf(argv[1], "%d\n", &port) < 0)) {
       error("Usage: " + std::string(argv[0]) + " [port]");
    }
    ExecServer execServer = ExecServer(port);
    execServer.start();
    return 0;
}

