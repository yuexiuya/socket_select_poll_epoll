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
/**
select的优缺点
优点：
（1）select的可移植性好，在某些unix下不支持poll.
（2）select对超时值提供了很好的精度，精确到微秒，而poll式毫秒。
缺点：
（1）单个进程可监视的fd数量被限制，默认是1024。
（2）需要维护一个用来存放大量fd的数据结构，这样会使得用户空间和内核空间在传递该结构时复制开销大。
（3）对fd进行扫描时是线性扫描，fd剧增后，IO效率降低，每次调用都对fd进行线性扫描遍历，随着fd的增加会造成遍历速度慢的问题。
（4）select函数超时参数在返回时也是未定义的，考虑到可移植性，每次超时之后进入下一个select之前都要重新设置超时参数。
*/
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
