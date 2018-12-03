#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <string>
#include <memory.h>
#include <boost/signals2.hpp>

using namespace std;
using namespace boost;

class UdpSocket {
public:
    UdpSocket();
    ~UdpSocket();
    UdpSocket(const UdpSocket& rhs)=delete;
    UdpSocket& operator=(const UdpSocket& rhs)=delete;

    /// register

    void init(const string ser_ip, const int ser_port,
              const string cli_ip, const int cli_port);

    void setBuflen(const int len);

    /// server
    int runServer();
//    signals2::signal<void (const char* buf)> sig_serverMsg;     /// server 接收到信息的回调
    int serverSend(const char* buf);
    /// client
    int runClient();
//    signals2::signal<void (const char* buf)> sig_clientMsg;     /// client 接收到信息的回调
    int clientSend(const char* buf, const int len);

private:
    string m_ip;
    int m_fd = -1;      /** socket 描述符*/
    int m_buflen = 1024;    /** 缓冲区长度*/
    char *m_buf = new char[m_buflen];
    struct sockaddr_in m_seraddr;  /** server sock连接地址*/
    struct sockaddr_in m_cliaddr;   /** client sock连接地址*/
};

#endif
