cmake_minimum_required(VERSION 3.8)
project(2018073001epoll_server)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp Server.cpp Server.h config/Config.h Struct.h MessageDispatcher.cpp MessageDispatcher.h)
add_executable(2018073001epoll_server ${SOURCE_FILES})