#ifndef EXECSERVER_H
#define EXECSERVER_H

#include <functional>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include "lib/epoll/EpollWrap.h"
#include "lib/Socket.h"
#include "lib/tcp/TcpServer.h"
#include "lib/tcp/TcpSocket.h"
#include <cstring>
#include <string>
#include <a.out.h>
#include <sys/wait.h>
#include <iostream>
#include <error.h>


class ExecServer {
    QMap<TcpSocket*, QString> commands;
    TcpServer tcpServer;

    void newRequest(TcpSocket* socket, EventType type) {
        if (type == ERROR || type == HUP) {
            commands.remove(socket);
        } else if (type == NEWDATA) {
            QString inf;
            socket->read(inf);
            commands[socket].append(inf);
            if (isValidCommand(commands[socket])) {                
                /*
                 * piped run and answer to client
                 */
            }
        }
    }

    bool isValidCommand(QString& str) {
        return str.contains("\n", Qt::CaseInsensitive) > 0;
    }

    void inline becomeNotGroupLeader() {
        pid_t pid = fork();
        if (pid < 0) {
            error("can't fork from initial process");
        }
        if (pid > 0) {
            exit(MY_EXIT_SUCCESS);
        }
    }

    void inline becomeSessionLeader() {
        becomeNotGroupLeader();
        pid_t pidSession = setsid();
        if (pidSession < 0) {
            error("can't get new session");
        }
    }

    pid_t makeSelfDaemon() {
        becomeSessionLeader();
        pid_t pidMy = fork();
        if (pidMy < 0) {
            error("can't fork in new session");
        }
        if (pidMy > 0) {
            exit(MY_EXIT_SUCCESS);
        }
        return getpid();
    }

    void writePid(pid_t pid, const char * fileName) {
        int fd = open(fileName, O_WRONLY | O_CREAT);
        if (fd < 0) {
            error("can't open file to write pid");
        }
        char bufPid[20];
        sprintf(bufPid, "%d", pid);
        int pidLen = strlen(bufPid);
        if (writeAll(fd, bufPid, pidLen) < 0) {
            error("can't write to file");
        }
        if (close(fd) < 0) {
            error(std::string("can't close file ") + fileName);
        }
    }


public:
    ExecServer(int port) :
        tcpServer(port, bind(&ExecServer::newRequest, this, std::placeholders::_1, std::placeholders::_2))
    {
        pid_t pid = makeSelfDaemon();
        writePid(pid, "/tmp/netsh.pid");
    }

    void start() {
        tcpServer.start();
    }

    void stop() {
        tcpServer.stop();
    }
};

#endif // EXECSERVER_H
