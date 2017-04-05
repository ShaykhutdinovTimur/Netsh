#include <QCoreApplication>
#include "lib/Utils.h"
#include "lib/Socket.h"
#include "lib/epoll/EpollWrap.h"
#include "lib/tcp/TcpServer.h"
#include "lib/tcp/TcpSocket.h"

#include <stdio.h>
#include <memory.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <netinet/in.h>

#include <string>
#include <iostream>
#include <set>
#include <map>
#include <vector>


QMap<TcpSocket*, QString> commands;
std::map<int, int> wait_child;


int exec_command(std::string command, int curstdin, int curstdout, int cfd) {
    int pid = fork();
    if (pid < 0) error("Error in fork():\n" + std::string(strerror(errno)));
    if (pid == 0) {
        close(STDIN_FILENO); dup(curstdin);
        close(STDOUT_FILENO); dup(curstdout);
        close(curstdin);
        close(curstdout);
        close(cfd);
        std::vector<std::string> args = split(command, ' ');
        char **argv = new char*[args.size() + 1];
        for (int i = 0; i < (int)args.size(); i++) {
            if (isQuot(args[i][0]) && (args[i][0] == args[i][(int)args[i].size() - 1])) {
                args[i] = args[i].substr(1, (int)args[i].size() - 2);
            }
            argv[i] = new char[args[i].size() + 1];
            strcpy(argv[i], args[i].c_str());
            argv[i][(int)args[i].size()] = 0;
        }
        argv[(int)args.size()] = 0;
        execvp(argv[0], argv);
        exit(0);
    }
    return pid;
}

std::string pipedRun(std::vector<std::string> commands, TcpSocket* socket) {
    int p[2];
    pipe2(p, O_CLOEXEC | O_NONBLOCK);
    socket->setArgFd(p[1]);
    int curstdin = p[0];
    for (int i = 0; i < (int)commands.size(); i++) {
        int nextstdin, curstdout;
        if (i + 1 < (int)commands.size()) {
            pipe2(p, O_CLOEXEC);
            nextstdin = p[0];
            curstdout = p[1];
        } else {
            nextstdin = -1;
            curstdout = socket->gfd();
        }
        int chpid = exec_command(commands[i], curstdin, curstdout, socket->gfd());
        std::cerr << "\"" << commands[i] << "\"" << " is executing in " << chpid << " process" << std::endl;
        close(curstdin);
        if (i + 1 < (int)commands.size()) {
            close(curstdout);
        } else wait_child[chpid] = socket->gfd();
        curstdin = nextstdin;
    }
    return "";
}

void newRequest(TcpSocket* socket, EventType type) {
    if (type == ERROR || type == HUP) {
        commands.remove(socket);
    } else if (type == NEWDATA) {
        QString inf;
        socket->read(inf);
        std::cout << inf.toStdString() << " input appended\n";
        if (socket->isExeced()) {
            socket->setArg(inf.toStdString().c_str(), inf.length());
        } else {
            commands[socket].append(inf);
            if (isValidCommand(commands[socket])) {
                socket->setExecuted();
                std::cout << commands[socket].toStdString() << " input runned\n";
                std::string commandsStr = commands[socket].toStdString().substr(0, commands[socket].length() - 1);
                std::string result = pipedRun(split(commandsStr, '|'), socket);
                commands[socket].clear();
                socket->write(result.data(), result.length());
            }
        }
    }
}


void inline becomeNotGroupLeader() {
    pid_t pid = fork();
    if (pid < 0) {
        error("can't fork from initial process");
    }
    if (pid > 0) {
        exit(MY_EXIT_SUCCESS);
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
        exit(MY_EXIT_SUCCESS);
    }
    return getpid();
}

void writePid(pid_t pid, const char * fileName) {
    int fd = open(fileName, O_WRONLY | O_CREAT);
    if (fd < 0) {
        error("can't open file to write pid");
    }
    char bufPid[20];
    sprintf(bufPid, "%d", pid);
    int pidLen = strlen(bufPid);
    if (writeAll(fd, bufPid, pidLen) < 0) {
        error("can't write to file");
    }
    if (close(fd) < 0) {
        error(std::string("can't close file ") + fileName);
    }
}

void chld_handler(int signum, siginfo_t *info, void*) {
    assert(signum == SIGCHLD);
    int pid = info->si_pid;
    waitpid(pid, NULL, 0);
    std::cout << pid << " has been closed" << std::endl;
    if (wait_child.count(pid) > 0) {        
        int cfd = wait_child[pid];
        std::cout << cfd << " has been closed by handler of process with pid=" << pid << "\n";
        close(cfd);
        wait_child.erase(pid);
    }
}

void make_sigact() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = chld_handler;
    if (sigaction(SIGCHLD, &sa, NULL) < 0) error("Error in sigaction()");
}

int main(int argc, char *argv[])
{
    int port = 8888;
    /*if (argc < 2 || (sscanf(argv[1], "%d\n", &port) < 0)) {
       error("Usage: " + std::string(argv[0]) + " [port]");
    }*/
    TcpServer tcpServer(port, bind(newRequest, std::placeholders::_1, std::placeholders::_2));
    make_sigact();
    //pid_t pid = makeSelfDaemon();
    //writePid(pid, "/tmp/netsh.pid");
    tcpServer.start();
    return 0;
}

