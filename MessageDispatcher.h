//
// Created by sunshine on 18-7-31.
//

#ifndef INC_2018073001EPOLL_SERVER_MESSAGEDISPATCH_H
#define INC_2018073001EPOLL_SERVER_MESSAGEDISPATCH_H


#include <map>
#include "Struct.h"

class MessageDispatcher {
public:
    MessageDispatcher();
    ~MessageDispatcher();


    //处理函数
    static void dispatch(int sockFd, MessagePackage messagePackage, std::map<int, MessagePackage> &sendMessagePackageList);
    static MessagePackage createMessage(int code, char *msg);
};


#endif //INC_2018073001EPOLL_SERVER_MESSAGEDISPATCH_H
