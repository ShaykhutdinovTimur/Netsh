#ifndef EPOLLWRAP
#define EPOLLWRAP

#include <QMap>
#include <unistd.h>
#include <sys/epoll.h>
#include <iostream>
#include "lib/Socket.h"

class EpollWrap {

private:
    int const MAX_EVENTS = 50;
    bool running;
    Socket epollFd;
    QMap<Socket, std::function<void(int)>> callbacks;

    int epollCtl(Socket fd, int events, int CTL_OPTION) {
        struct epoll_event event;
        memset(&event, 0, sizeof(epoll_event));
        event.data.fd = fd;
        event.events = events;
        if (epoll_ctl(epollFd, CTL_OPTION, fd, &event) == EPOLL_EXIT_FAILURE){
            return EPOLL_EXIT_FAILURE;
        }
        return EPOLL_EXIT_SUCCESS;
    }

public:

    static int EPOLL_EXIT_FAILURE = -1;
    static int EPOLL_EXIT_SUCCESS = 0;

    EpollWrap() {
        epollFd = new Socket(epoll_create1(0));
        running = false;
    }

    void startListening() {
        running = true;
        struct epoll_event events[MAX_EVENTS];
        while (running) {
            int static const NO_TIMEOUT = EPOLL_EXIT_FAILURE;
            int eventsCount = epoll_wait(epollFd, events, MAX_EVENTS, NO_TIMEOUT);
            for (int i = 0; i < eventsCount; i++) {
                callbacks[events[i].data.fd](events[i].events);
            }
        }
    }

    void stopListening() {
        running = false;
    }

    ~EpollWrap() {
        QMap<int, std::function<void(int)>> temp = callbacks;
        for (auto cur = temp.begin(); cur != temp.end(); cur++) {
            cur.value()(EPOLLRDHUP);
        }
    }

    int add(Socket fd, std::function<void(int)> callback, int events) {
        callbacks.insert(fd, callback);
        return epollCtl(fd, events, EPOLL_CTL_ADD);
    }

    int remove(Socket fd) {
        if (isListening(fd)) {
            callbacks.remove(fd);
        }
        int ignoredEvents = 0;
        return epollCtl(fd, ignoredEvents, EPOLL_CTL_DEL);
    }

    int modify(Socket fd, int events) {
        return epollCtl(fd, events, EPOLL_CTL_MOD);

    }

    bool isListening(Socket fd) {
        return callbacks.count(fd) == 1;
    }
};

#endif // EPOLLWRAP

