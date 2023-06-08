cmake_minimum_required(VERSION 3.25)
project(BookStore-cpp)

set(CMAKE_CXX_STANDARD 17)

add_executable(untitled main.cpp http_tcpServer.h http_tcpServer.cpp Base64.h)
