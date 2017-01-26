#ifndef TCPSERVER_H
#define TCPSERVER_H

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
class TcpServer;
#include "lib/tcp/TcpSocket.h"

enum EventType {NEWDATA, ERROR, HUP};
typedef std::shared_ptr<TcpSocket> sockptr;

class TcpServer {

    static const int QUEUE_SIZE = 10;
    QMap<int, sockptr> sockets;
    EpollWrap epoll;
    Socket serverSocket;

    void connectionHandler(std::function<void(TcpSocket*, EventType)> newData, Socket fd, int event) {
        if (event & EPOLLRDHUP) {
            epoll.remove(fd);
            return;
        }
        if (event & EPOLLERR) {
            error("error on TCPsocket", false);
            epoll.remove(fd);
            return;
        }
        if (event & EPOLLIN) {
            while (true) {
                struct sockaddr in_addr;
                socklen_t in_len;
                Socket infd;

                in_len = sizeof in_addr;
                infd = accept(fd, &in_addr, &in_len);

                if (infd == MY_EXIT_FAILURE) {
                    if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                        break;
                    } else {
                        error("accept error", false);
                        break;
                    }
                }

                if (infd.makeNonblocking() == MY_EXIT_FAILURE) {
                    error("can't make socket nonblocking", false);
                    continue;
                }
                std::function<void(int)> handlerFunction =
                        bind(&TcpServer::dataHandler, this, newData, infd, std::placeholders::_1);
                sockets[infd] = sockptr(new TcpSocket(&(this->epoll), infd));
                epoll.add(infd, handlerFunction, EPOLLIN | EPOLLET | EPOLLRDHUP);
            }
        }
    }

    int createAndBind(int port) {
        struct addrinfo hints;
        struct addrinfo *result, *rp;
        int s;
        int sfd;

        memset(&hints, 0, sizeof (struct addrinfo));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        std::string portstr = std::to_string(port);
        s = getaddrinfo (NULL, portstr.c_str(), &hints, &result);
        if (s != 0) {
            return MY_EXIT_FAILURE;
        }

        for (rp = result; rp != NULL; rp = rp->ai_next) {
            sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (sfd == MY_EXIT_FAILURE)
                continue;
            int t = 1;
            if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &t, sizeof(t)) == MY_EXIT_FAILURE) {
                error("can't make socket reusable", false);
                continue;
            }
            if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) {
                break;
            }
            close(sfd);
        }
        if (rp == NULL) {
            return MY_EXIT_FAILURE;
        }
        freeaddrinfo(result);

        return sfd;
    }

    void inline deleteSocket(Socket fd) {
        epoll.remove(fd);
        sockets.remove(fd);
    }

    void dataHandler(std::function<void(TcpSocket*, EventType)> dataHandler, Socket fd, int event) {
        if (event & EPOLLRDHUP) {
            dataHandler(&*sockets[fd], HUP);
            deleteSocket(fd);
        } else if (event & EPOLLERR) {
            dataHandler(&*sockets[fd], ERROR);
            deleteSocket(fd);
        } else {
            if (event & EPOLLIN) {
                dataHandler(&*sockets[fd], NEWDATA);
            }
            if (event & EPOLLOUT) {
                if (sockets[fd]->flush() == 0) {
                    deleteSocket(fd);
                }
            }
        }
    }

public:

    TcpServer(int port, std::function<void(TcpSocket*, EventType)> newData) {
        Socket tcpfd = createAndBind(port);
        if (tcpfd == MY_EXIT_FAILURE) {
            error("can't create or bind socket, perhaps port busy");
        }
        if (tcpfd.makeNonblocking() == MY_EXIT_FAILURE) {
            error("can't make server socket nonblocking");
        }
        if (listen(tcpfd, QUEUE_SIZE) == MY_EXIT_FAILURE) {
            error("listen error");
        }
        std::function<void(int)> handlerFunction =
                bind(&TcpServer::connectionHandler, this, newData, tcpfd, std::placeholders::_1);
        epoll.add(tcpfd, handlerFunction, EPOLLIN | EPOLLET | EPOLLRDHUP);
    }

    ~TcpServer();


    void start() {
        epoll.startListening();
    }

    void stop() {
        epoll.stopListening();
    }
};



#endif // TCPSERVER_H
