//
// Created by sunshine on 18-7-30.
//

#ifndef INC_2018073001EPOLL_SERVER_CONFIG_H
#define INC_2018073001EPOLL_SERVER_CONFIG_H

const int ServerPort = 6666;
const int EpollSize = 100; //set EpollSize > 20
const int Timeout = 20; //客户端掉线超时

#define LISTENQ 20

#endif //INC_2018073001EPOLL_SERVER_CONFIG_H
