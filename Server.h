//
// Created by sunshine on 18-7-30.
//

#ifndef INC_2018073001EPOLL_SERVER_SERVER_H
#define INC_2018073001EPOLL_SERVER_SERVER_H

#include <sys/epoll.h>
#include <netinet/in.h>
#include <map>
#include "Struct.h"

/**
 * 使用边缘触发
 * 使用半双工通信
 */
class Server {
public:
    Server();
    ~Server();

    void init();
private:
    int epollFd, listenFd, connFd;
    socklen_t clientLen;
    struct MessagePackage msgPkg;
    int nfds;
    struct epoll_event event, events[20];
    //
    std::map<int, ClientInfo > onlineList; //在线列表
    std::map<int, MessagePackage> sendMessagePackageList; //待发送消息列表
    //设置fd非阻塞
    void setNonBlocking(int sock);

    void s_epoll_create(int size);
    void s_socket();
    int s_epoll_ctl(int __op, int __fd, struct epoll_event * __event);
    void s_bind();
    void s_listen();
    void s_epoll_wait();

    void printLocalTime(const time_t &t);
    void updateAliveTime(int sockFd, std::map<int, ClientInfo> &onlineList); //更新存活时间
    void scanAndKickOff(); //长时间不在线的客户端踢下线
    void deleteClientData(int sockFd); //清理客户端数据

};


#endif //INC_2018073001EPOLL_SERVER_SERVER_H
