#ifndef SOCKET_H
#define SOCKET_H

#include <memory>
#include <unistd.h>
#include <fcntl.h>
#include "Utils.h"

using std::shared_ptr;

//exellent socket wrapper proposed by Aganov Artur
//well appricated by Ivan Sorokin

class Socket {
    struct SubSocket {
        int socket;
        SubSocket(int sock) {
            socket = sock;
        }
        ~SubSocket() {
            close(socket);
        }
    };

    shared_ptr<SubSocket> socket_ptr;


public:
    Socket(int sock) {
        socket_ptr = std::make_shared<SubSocket>(sock);
    }
    Socket() {
        socket_ptr = nullptr;
    }
    void operator= (const Socket &rhs) {
        socket_ptr = (rhs.socket_ptr);
    }
    operator int () const {
        return socket_ptr->socket;
    }

    int makeNonblocking() {
        int flags;
        if ((flags = fcntl(socket_ptr->socket, F_GETFL, 0)) < 0) {
            error("Error in fcntl() (F_GETFL)", false);
            return MY_EXIT_FAILURE;
        }
        flags |= O_NONBLOCK;
        if (fcntl(socket_ptr->socket, F_SETFL, flags) < 0) {
            error("Error in fcntl() (F_SETFL)", false);
            return MY_EXIT_FAILURE;
        }
        return MY_EXIT_SUCCESS;
    }
};

#endif // SOCKET_H
