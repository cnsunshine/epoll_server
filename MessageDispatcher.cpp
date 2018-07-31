//
// Created by sunshine on 18-7-31.
//

#include <cstring>
#include "Struct.h"
#include "MessageDispatcher.h"

MessageDispatcher::MessageDispatcher() {

}

MessageDispatcher::~MessageDispatcher() {

}

void MessageDispatcher::dispatch(int sockFd, MessagePackage messagePackage, std::map<int, MessagePackage> &sendMessagePackageList) {
    switch (messagePackage.code){
        case 100:
            sendMessagePackageList[sockFd] = MessageDispatcher::createMessage(100, "hello");
            break;
        case 101: //设置
            break;
        default:
            sendMessagePackageList[sockFd] = MessageDispatcher::createMessage(0, "bye");
            break;
    }
}

MessagePackage MessageDispatcher::createMessage(int code, char *msg) {
    MessagePackage messagePackage;
    messagePackage.code = code;
    strcpy(messagePackage.msg, msg);
    messagePackage.timestamp = time(0);
    return messagePackage;
}
