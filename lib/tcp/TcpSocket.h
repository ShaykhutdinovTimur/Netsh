#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include <string.h>
#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <QString>
#include <memory>

class TcpSocket;

#include "TcpServer.h"
#include "lib/Socket.h"
#include "lib/epoll/EpollWrap.h"
#include "lib/Utils.h"

class TcpSocket {

    static const int BUFFER_SIZE = 65536;
    Socket fd;
    Socket argFd;
    bool execed;
    EpollWrap* epoll;
    size_t buffersize;
    char buffer[BUFFER_SIZE];

    int flush() {
        size_t count = 0;
        while (count != buffersize) {
            int written = ::write(fd, buffer + count, buffersize - count);
            if (written == MY_EXIT_FAILURE) {
                if (errno != EAGAIN) {
                    return MY_EXIT_FAILURE;
                } else {
                    break;
                }
            } else {
                count += written;
            }
        }
        for (size_t i = count; i < buffersize; i++) {
            buffer[i - count] = buffer[i];
        }
        buffersize -= count;
        if (buffersize == 0) {
            return MY_EXIT_SUCCESS;
        }
        return MY_EXIT_WAIT;
    }


public:

    TcpSocket(EpollWrap* serv, Socket socket) {
        epoll = serv;
        fd = socket;
        buffersize = 0;
        execed = false;
    }

    ~TcpSocket() {}

    int read(QString& str) {
        char data[BUFFER_SIZE];
        int size;
        while (true) {
            size = ::read(fd, data, BUFFER_SIZE);
            if (size == MY_EXIT_SUCCESS) {
                return MY_EXIT_SUCCESS;
            } else if (size == MY_EXIT_FAILURE) {
                if (errno != EAGAIN) {
                    error("reading failed", false);
                    str = "";
                    return MY_EXIT_FAILURE;
                } else {
                    return MY_EXIT_WAIT;
                }
            } else {
                data[size] = 0;
                str.append(data);
            }
        }
    }

    int write(const char * data, size_t size) {
        if (buffersize + size > BUFFER_SIZE) {
            return MY_EXIT_FAILURE;
        }
        for (size_t i = 0; i < size; i++) {
            buffer[buffersize + i] = data[i];
        }
        buffersize += size;
        epoll->modify(fd, EPOLLOUT | EPOLLIN | EPOLLET | EPOLLRDHUP);
        return MY_EXIT_SUCCESS;
    }

    int gfd() {
        return fd;
    }

    void setArgFd(int fd) {
        argFd = Socket(fd);
    }

    void setArg(const void* buf, size_t size) {
        writeAll(argFd, buf, size);
    }

    void setExecuted() {
        execed = true;
    }

    bool isExeced() {
        return execed;
    }

    friend class TcpServer;
};


#endif // TCPSOCKET_H
