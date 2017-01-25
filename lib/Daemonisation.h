#ifndef DAEMONISATION_H
#define DAEMONISATION_H

#include "Utils.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


void inline becomeNotGroupLeader() {
    pid_t pid = fork();
    if (pid < 0) {
        error("can't fork from initial process");
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
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
        exit(EXIT_SUCCESS);
    }
    return getpid();
}

void writePid(pid_t pid, const char * fileName) {
    int fd = open(fileName, O_RDWR | O_CREAT);
    if (fd < 0) {
        error("can't open file to write pid");
    }
    char bufPid[20];
    sprintf(bufPid, "%d", pid);
    int pidLen = strlen(bufPid);
    if (writeAll(fd, bufPid, pidLen + 1) < 0) {
        error("can't write to file");
    }
    if (close(fd) < 0) {
        error(std::string("can't close file ") + fileName);
    }
}



#endif // DAEMONISATION_H
