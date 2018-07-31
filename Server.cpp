//
// Created by sunshine on 18-7-30.
//

#include <fcntl.h>
#include <sys/epoll.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Server.h"
#include "config/Config.h"
#include "MessageDispatcher.h"

Server::Server() {

}

Server::~Server() {

}

void Server::setNonBlocking(int sock) {
    int flags;
    flags = fcntl(sock, F_GETFL);

    if (flags < 0) {
        std::cout << "[fcntl(sock, F_GETFL)] wrong" << std::endl;
        exit(0);
    }

    flags = flags | O_NONBLOCK;

    if (fcntl(sock, F_SETFL, flags) < 0) {
        std::cout << "[fcntl(sock, F_SETFL, flags)] wrong" << std::endl;
        exit(0);
    }
}

void Server::s_epoll_create(int size) {
    this->epollFd = epoll_create(size);
}

void Server::s_socket() {
    this->listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->listenFd == -1) {
        std::cout << "[socket(AF_INET, SOCK_STREAM, 0)] wrong" << std::endl;
        exit(0);
    };
}

int Server::s_epoll_ctl(int __op, int __fd, struct epoll_event *__event) {
    return epoll_ctl(this->epollFd, __op, __fd, __event);
}

void Server::s_bind() {
    struct sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(ServerPort);

    bind(this->listenFd, (struct sockaddr *) &serverAddr, sizeof(serverAddr));
}

void Server::s_listen() {
    listen(this->listenFd, LISTENQ);

}

void Server::s_epoll_wait() {
    int sockFd;
    ssize_t n;
    struct sockaddr_in clientAddr;
    std::map<int, ClientInfo>::iterator it_onlineList;
    std::map<int, MessagePackage>::iterator it_sendMessagePackageList;


    this->nfds = epoll_wait(this->epollFd, this->events, 20, 0);

    for (int i = 0; i < this->nfds; ++i) {
        if (this->events[i].data.fd == this->listenFd) {
            this->connFd = accept(this->listenFd, (struct sockaddr *) &clientAddr, &(this->clientLen));
            //
            this->onlineList[this->connFd] = {clientAddr, time(0), time(0)};
            if (this->connFd < 0) {
                std::cout << "[Server::s_epoll_wait()] accept(...) wrong" << std::endl;
                exit(0);
            }
            this->setNonBlocking(this->connFd);
            std::cout << "[Server::s_epoll_wait()] client connect, from " << inet_ntoa(clientAddr.sin_addr) << " on port "
                      << ntohs(clientAddr.sin_port) << std::endl;
            //print login info
            it_onlineList = this->onlineList.find(this->connFd);
            if (it_onlineList != this->onlineList.end()) {
                std::cout << "[Server::s_epoll_wait()] login time: ";
                this->printLocalTime(it_onlineList->second.loginTime);
                std::cout << std::endl;
            }

            this->event.data.fd = this->connFd;
            this->event.events = EPOLLIN | EPOLLET;
            this->s_epoll_ctl(EPOLL_CTL_ADD, this->connFd, &(this->event));

        } else if (this->events[i].events & EPOLLIN) {
            if ((sockFd = this->events[i].data.fd) < 0) continue;
            //
            if ((n = read(sockFd, &msgPkg, (size_t) sizeof(struct MessagePackage))) < 0) {
                if (errno == ECONNRESET) {
                    close(sockFd);
                    this->events[i].data.fd = -1;
                } else {
                    std::cout << "[Server::s_epoll_wait()] read data error" << std::endl;
                }
            } else if (n == 0) { //client close
                it_onlineList = this->onlineList.find(sockFd);
                if (it_onlineList != onlineList.end()) {
                    std::cout << "[Server::s_epoll_wait()] bye, from " << inet_ntoa(it_onlineList->second.sockAddr.sin_addr) << " on port "
                              << ntohs(it_onlineList->second.sockAddr.sin_port) << std::endl;
                    std::cout << "[Server::s_epoll_wait()] login time: ";
                    this->printLocalTime(it_onlineList->second.loginTime);
                    std::cout << std::endl;
                }
                close(sockFd);
                this->event.data.fd = sockFd;
                this->s_epoll_ctl(EPOLL_CTL_DEL, sockFd, &(this->event));
                this->deleteClientData(sockFd);
                this->events[i].data.fd = -1;
            } else {
                this->updateAliveTime(sockFd, this->onlineList);
                std::cout << "[Server::s_epoll_wait()] get message: " << this->msgPkg.msg << std::endl;
                MessageDispatcher::dispatch(sockFd, msgPkg, sendMessagePackageList);
            }
            this->event.data.fd = sockFd;
            this->event.events = EPOLLOUT | EPOLLET;
            this->s_epoll_ctl(EPOLL_CTL_MOD, sockFd, &(this->event));

        } else if (this->events[i].events & EPOLLOUT) {
            sockFd = this->events[i].data.fd;
            it_sendMessagePackageList = this->sendMessagePackageList.find(sockFd);
            if (it_sendMessagePackageList != this->sendMessagePackageList.end()) {
                write(it_sendMessagePackageList->first, &(it_sendMessagePackageList->second),
                      sizeof(struct MessagePackage));
                std::cout << "[Server::s_epoll_wait()] written data: " << it_sendMessagePackageList->second.msg << std::endl;
                this->updateAliveTime(sockFd, this->onlineList);
                this->sendMessagePackageList.erase(it_sendMessagePackageList);
                this->event.data.fd = sockFd;
                this->event.events = EPOLLIN | EPOLLET;
                this->s_epoll_ctl(EPOLL_CTL_MOD, sockFd, &(this->event));
            } else {
                this->event.data.fd = sockFd;
                this->event.events = EPOLLOUT | EPOLLET;
                this->s_epoll_ctl(EPOLL_CTL_MOD, sockFd, &(this->event));
            }
        }
    }


}

void Server::init() {
    this->s_epoll_create(EpollSize);
    this->s_socket();
    this->setNonBlocking(this->listenFd);

    this->event.data.fd = this->listenFd;
    this->event.events = EPOLLIN | EPOLLET;

    this->s_epoll_ctl(EPOLL_CTL_ADD, this->listenFd, &(this->event));
    this->s_bind();
    this->s_listen();
    while (true) {
        this->s_epoll_wait();
        this->scanAndKickOff();
    }
}

void Server::printLocalTime(const time_t &t) {
    tm *localTime;
    localTime = localtime(&t);
    std::cout << localTime->tm_year + 1900 << "/" << localTime->tm_mon + 1 << "/" << localTime->tm_mday << " "
              << localTime->tm_hour << ":" << localTime->tm_min << ":" << localTime->tm_sec;
}

void Server::updateAliveTime(int sockFd, std::map<int, ClientInfo> &onlineList) {
    std::map<int, ClientInfo>::iterator it;
    it = onlineList.find(sockFd);
    if (it!=onlineList.end()){
        it->second.lastAliveTime = time(0);
    }
}

void Server::scanAndKickOff() {
    time_t now = time(0);
    std::map<int, ClientInfo>::iterator it;
    it = this->onlineList.begin();
    while (it != this->onlineList.end()) {
        if (now - it->second.lastAliveTime >= Timeout) {
            //
            MessagePackage messagePackage = MessageDispatcher::createMessage(0, "bye");
            write(it->first, &messagePackage, sizeof(MessagePackage));
            //超时离线
            close(it->first);
            this->event.data.fd = it->first;
            this->s_epoll_ctl(EPOLL_CTL_DEL, it->first, &(this->event));
            //删除onlineList和sendMessagePackageList
            this->deleteClientData(it->first);
            std::cout << "[Server::sanAndKickOff()] kick " << it->first << std::endl;
        }
        it++;
    }
}

void Server::deleteClientData(int sockFd) {
    std::map<int, ClientInfo>::iterator it;
    it = this->onlineList.find(sockFd);
    if (it != this->onlineList.end()) {
        this->onlineList.erase(it);
    }

    std::map<int, MessagePackage>::iterator it1;
    it1 = this->sendMessagePackageList.find(sockFd);
    if (it1 != this->sendMessagePackageList.end()) {
        this->sendMessagePackageList.erase(sockFd);
    }
}
