#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <QString>
#include <string>
#include <vector>
#include <fcntl.h>

static const int MY_EXIT_FAILURE = -1;
static const int MY_EXIT_WAIT = 1;
static const int MY_EXIT_SUCCESS = 0;

bool isQuot(char c) {
    return c == '\"' || c == '\'' || c == '`';
}

std::vector<std::string> split(std::string str, char separator) {
    str += separator;
    std::vector<std::string> result;
    std::string curString;
    char lookForEnd = 0;
    for (int i = 0; i < (int)str.length(); i++) {
        if (lookForEnd != 0) {
            if (str[i] == lookForEnd) lookForEnd = 0;
            curString += str[i];
        } else if (isQuot(str[i])) {
            lookForEnd = str[i];
            curString += str[i];
        } else if (str[i] == separator) {
            if (curString != "") {
                if (curString.length() > 2 && isQuot(curString[0]) &&
                        isQuot(curString[curString.length() - 1]))
                    curString = curString.substr(1, (int)curString.length() - 2);
                result.push_back(curString);
            }
            curString = "";
        } else { curString += str[i]; }
    }
    return result;
}

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


int makeNonblocking(int fd) {
    int flags;
    if ((flags = fcntl(fd, F_GETFL, 0)) < 0) {
        error("Error in fcntl() (F_GETFL)", false);
        return MY_EXIT_FAILURE;
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) {
        error("Error in fcntl() (F_SETFL)", false);
        return MY_EXIT_FAILURE;
    }
    return MY_EXIT_SUCCESS;
}

#endif // UTILS_H
