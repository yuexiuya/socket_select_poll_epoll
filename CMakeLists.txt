PROJECT(socket_Demo)
cmake_minimum_required(VERSION 2.8.8 FATAL_ERROR)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++0x -O2 -g3 -fPIC")

SET(SOCKET_MODE 0)

if(SOCKET_MODE EQUAL 0) #udp socket
    ADD_SUBDIRECTORY(src/udp_socket)
endif(SOCKET_MODE EQUAL 0)



