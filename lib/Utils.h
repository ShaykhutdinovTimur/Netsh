#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


void error(std::string s) {
    std::cerr << s << std::endl;
    exit(EXIT_FAILURE);
}

void become_daemon() {
    int pid = fork();
    if (pid < 0)
        error("Error in fork():\n" + std::string(strerror(errno)));
    if (pid != 0)
        exit(EXIT_SUCCESS);
    setsid();
    pid = fork();
    if (pid < 0) error("Error in fork():\n" + std::string(strerror(errno)));
    if (pid != 0)
        exit(EXIT_SUCCESS);
    pid = getpid();
    chdir("/");
    umask(0);
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    int fd = open("/tmp/netsh.pid", O_WRONLY | O_CREAT);
    if (fd < 0)
        error("Error in open():\n" + std::string(strerror(errno)));
    pid = getpid();
    dprintf(fd, "%d\n", pid);
    close(fd);
}

#endif // UTILS_H
