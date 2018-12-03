#include "UdpSocket.h"
#include <arpa/inet.h>

UdpSocket::UdpSocket()
{

}

UdpSocket::~UdpSocket()
{
    if(m_buf != nullptr)
        delete []m_buf;
}

void UdpSocket::init(const string ser_ip, const int ser_port, const string cli_ip, const int cli_port)
{
    /// ser addr
    memset(&m_seraddr, 0, sizeof(sockaddr_in));
    m_seraddr.sin_family = AF_INET;
//    m_seraddr.sin_addr.s_addr = htonl(INADDR_ANY/* 本地地址*/);    ///网络字节序转换
    m_seraddr.sin_addr.s_addr = inet_addr(ser_ip.c_str());
    m_seraddr.sin_port = htons(ser_port);    ///端口号

    /// client addr
    memset(&m_cliaddr, 0, sizeof(sockaddr_in));
    m_cliaddr.sin_family = AF_INET;
//    m_cliaddr.sin_addr.s_addr = htonl(INADDR_ANY/* 本地地址*/);    ///网络字节序转换
    m_cliaddr.sin_addr.s_addr = inet_addr(cli_ip.c_str());
    m_cliaddr.sin_port = htons(cli_port);    ///端口号

    this->setBuflen(1024);
}

void UdpSocket::setBuflen(const int len)
{
    m_buflen = len;
    if(m_buf != nullptr)
        delete []m_buf;
    m_buf = new char[m_buflen];
}

/**
 * @brief UdpSocket::runServer
 *      运行一个 udp server 端
 * @return  成功返回0，失败返回-1
 */
int UdpSocket::runServer()
{
    /// 1. 初始化 server socket
    m_fd = socket(AF_INET/* ipv4 */, SOCK_DGRAM /* udp */, 0);
    if(m_fd < 0) {
        printf("[server] create socket failed ! \n");
        return -1;
    }
    /// 2. socket 地址配置

    /// 3. bind
    int _ret = bind(m_fd, (sockaddr*)&m_seraddr, sizeof(m_seraddr));
    if( _ret<0 ) {
        printf("[server] socket bind failed ! \n");
        return -1;
    }
    int _count = 0;
    while(1) {
        /// no data, will blocking
        /// 接收 client 端的数据
        printf("[server] udp server run ...\n");
        socklen_t _len = sizeof(m_cliaddr);
        _count = recvfrom(m_fd, m_buf, m_buflen, 0, (struct sockaddr*)&m_cliaddr, &_len);
        if(_count < 0) {
            printf("[server] receive data failed ! \n");
            return -1;
        }
        printf("[server] receive info : %s \n", m_buf);
        printf("[server] receive info : %d \n", sizeof(m_buf));
        memset(m_buf, 0, m_buflen);
    }
    return 0;
}

/**
 * @brief UdpSocket::runClient
 *      运行一个 udp client 端
 * @return 成功返回0，失败返回-1
 */
int UdpSocket::runClient()
{
    /// 1. 初始化 server socket
    m_fd = socket(AF_INET/* ipv4 */, SOCK_DGRAM /* udp */, 0);
    if(m_fd < 0) {
        printf("create socket failed ! \n");
        return -1;
    }
    /// 2. socket 地址配置

    /// 3. 发送信息
    sprintf(m_buf, "udp client run ... \n");
    socklen_t _len = sizeof(m_seraddr);
    sendto(m_fd, m_buf, m_buflen, 0, (struct sockaddr*)&m_seraddr, _len);
    return 0;
}

/**
 * @brief UdpSocket::clientSend
 *      client 发送信息
 * @param buf : 信息
 * @param len : 信息长度
 * @return 成功返回0，失败返回-1
 */
int UdpSocket::clientSend(const char *buf, const int len)
{
    if(m_fd<0 || m_seraddr == nullptr) {
        printf("[clientSend] udp client not init! \n");
        return -1;
    }
    socklen_t _socklen = sizeof(m_seraddr);
    sendto(m_fd, buf, len, 0, (struct sockaddr*)&m_seraddr, _socklen);
    return 0;
}
