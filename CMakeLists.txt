cmake_minimum_required(VERSION 3.29)
project(sternap)

set(CMAKE_CXX_STANDARD 14)

include_directories(Server)

add_executable(sternap
    Server/MainServer.cpp
    Server/MainServer.h)
