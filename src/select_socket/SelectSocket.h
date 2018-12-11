#ifndef SELECTSOCKET_H
#define SELECTSOCKET_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <boost/signals2.hpp>
#include <vector>

using namespace std;

/** Socket Select 模式 : non-block;　1 server -> n clients*/
class SelectSocket {
public:
    SelectSocket();
    ~SelectSocket();
    SelectSocket(const SelectSocket& rhs)=delete;
    SelectSocket& operator=(const SelectSocket& rhs)=delete;

    /// this function will block
    int initServer(const string ser_ip, const int ser_port, const int bufsize = 1024);

    int initClient(const string ser_ip, const int ser_port, const int bufsize = 1024);

    void setSockArraySize(const int size);

//    /// callback
    boost::signals2::signal<void (const char* buf, const int len)> sig_serverMsg;
    boost::signals2::signal<void (const char* buf, const int len)> sig_clientMsg;

    int severSend(const char* buf, const int len);

    int clientSend(const char* buf, const int len);

private:
    int m_serlen = 0;
    char *m_serbuf = nullptr;
    int m_serfd = -1;  /// server socket 描述符
    int m_socklen = -1;
    int *m_sockArray = nullptr;
    int m_maxfd = 0;    /// select监听的client中，maxfd的最大值
    vector<int> m_clientList; /// 客户端监听client的列表

    int m_clilen = 0;
    char *m_clibuf = nullptr;
    int m_clifd = -1;  /// client socket 描述符

};

#endif //SELECTSOCKET_H
