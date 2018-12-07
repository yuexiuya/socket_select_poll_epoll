#ifndef TCPSOCKET_H
#define TCPSOCKET_H

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

using namespace std;

/** TcpSocket 单对单连接 */
class TcpSocket {
public:
    TcpSocket();
    ~TcpSocket();
    TcpSocket(const TcpSocket& rhs)=delete;
    TcpSocket& operator=(const TcpSocket& rhs)=delete;

    /// this function will block
    int initServer(const string ser_ip, const int ser_port, const int bufsize = 1024);

    int initClient(const string ser_ip, const int ser_port, const int bufsize = 1024);

    /// callback
    boost::signals2::signal<void (const char* buf, const int len)> sig_serverMsg;
    boost::signals2::signal<void (const char* buf, const int len)> sig_clientMsg;

    int severSend(const char* buf, const int len);

    int clientSend(const char* buf, const int len);
private:
    int m_serlen = 0;
    char *m_serbuf;
    int m_serfd = -1;  /// server socket 描述符
    int m_confd = -1; /// server 端信息发送描述符

    int m_clilen = 0;
    char *m_clibuf;
    int m_clifd = -1;  /// client socket 描述符

};

#endif
