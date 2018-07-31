//
// Created by sunshine on 18-7-30.
//

#ifndef INC_2018073001EPOLL_SERVER_STRUCT_H
#define INC_2018073001EPOLL_SERVER_STRUCT_H

#include <netinet/in.h>
#pragma pack(1)

struct MessagePackage {
    int code; //业务代码
    time_t timestamp;
    char msg[1000]; //信息体
};
#pragma pack()

struct ClientInfo {
    sockaddr_in sockAddr;
    time_t loginTime;
    time_t lastAliveTime;
};


#endif //INC_2018073001EPOLL_SERVER_STRUCT_H
