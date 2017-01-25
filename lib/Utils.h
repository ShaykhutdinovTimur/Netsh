#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <iostream>
#include <unistd.h>


void error(std::string reason, bool mustExit = true) {
    std::cerr << reason << std::endl;
    if (mustExit) {
        exit(EXIT_FAILURE);
    }
}

ssize_t writeAll(int fd, const void *buf, size_t count) {
    size_t processed = 0;
    while (processed < count) {
        ssize_t writed = write(fd, buf + processed, count - processed);
        if (writed < 0 && errno != EINTR) {
            return -1;
        } else {
            processed += writed;
        }
    }
    return processed;
}

#endif // UTILS_H
