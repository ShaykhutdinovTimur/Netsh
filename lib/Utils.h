#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <iostream>
#include <unistd.h>


static const int MY_EXIT_FAILURE = -1;
static const int MY_EXIT_WAIT = 1;
static const int MY_EXIT_SUCCESS = 0;

void error(std::string reason, bool mustExit = true) {
    std::cerr << reason << std::endl;
    if (mustExit) {
        exit(MY_EXIT_FAILURE);
    }
}

ssize_t writeAll(int fd, const void *buf, size_t count) {
    size_t processed = 0;
    while (processed < count) {
        ssize_t writed = write(fd, buf + processed, count - processed);
        if (writed < 0) {
            if (errno != EINTR) {
                return MY_EXIT_FAILURE;
            }
        } else {
            processed += writed;
        }
    }
    return processed;
}

#endif // UTILS_H
