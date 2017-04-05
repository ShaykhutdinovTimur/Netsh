#ifndef SOCKET_H
#define SOCKET_H

#include <memory>
#include <unistd.h>
#include <fcntl.h>
#include "Utils.h"

using std::shared_ptr;

class Socket {

    struct SubSocket {
        int socket;
        SubSocket(int sock) {
            socket = sock;
            std::cout << socket << " opened\n";
        }
        ~SubSocket() {
            std::cout << socket << " closed\n";
            close(socket);
        }
    };

    shared_ptr<SubSocket> socket_ptr;


public:
    Socket(const Socket &rhs) {
        socket_ptr = rhs.socket_ptr;
    }

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
};

#endif // SOCKET_H
