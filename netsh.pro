QT += core
QT -= gui

TARGET = netsh
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

HEADERS += \
    lib/epoll/EpollWrap.h \
    lib/Socket.h \
    lib/Utils.h \
    lib/tcp/TcpServer.h \
    lib/tcp/TcpSocket.h

