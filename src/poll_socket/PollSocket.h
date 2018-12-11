#ifndef POLLSOCKET_H
#define POLLSOCKET_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<poll.h>
#include <iostream>
#include <boost/signals2.hpp>
#include <vector>

using namespace std;

/** Poll Select 模式 : */
/**
poll函数的优缺点
优点：
（1）不要求计算最大文件描述符+1的大小。
（2）应付大数量的文件描述符时比select要快。
（3）没有最大连接数的限制是基于链表存储的。
缺点：
（1）大量的fd数组被整体复制于内核态和用户态之间，而不管这样的复制是不是有意义。
（2）同select相同的是调用结束后需要轮询来获取就绪描述符。
*/
class PollSocket {
public:
    PollSocket();
    ~PollSocket();
    PollSocket(const PollSocket& rhs)=delete;
    PollSocket& operator=(const PollSocket& rhs)=delete;

    /// this function will block
    int initServer(const string ser_ip, const int ser_port, const int bufsize = 1024);

    int initClient(const string ser_ip, const int ser_port, const int bufsize = 1024);

//    /// callback
    boost::signals2::signal<void (const char* buf, const int len)> sig_serverMsg;
    boost::signals2::signal<void (const char* buf, const int len)> sig_clientMsg;

    int severSend(const char* buf, const int len);

    int clientSend(const char* buf, const int len);

    void setSbuflen(int sbuflen);

    void setCbuflen(int cbuflen);

    void setMaxfd(int maxfd);

private:
    int m_sbuflen = 0;
    char *m_serbuf = nullptr;
    int m_serfd = -1;  /// server socket 描述符
    int m_maxfd = 1024;    /// select监听的client中，maxfd的最大值
    int m_nfds = 0;     /// select监听的client中，有效的client
    struct pollfd *m_pollClients = nullptr; /// 客户端监听client的列表

    int m_cbuflen = 0;
    char *m_clibuf = nullptr;
    int m_clifd = -1;  /// client socket 描述符

};

#endif //POLLSOCKET_H
