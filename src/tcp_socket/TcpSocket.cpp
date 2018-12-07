#include "TcpSocket.h"
#include <thread>

TcpSocket::TcpSocket()
{

}

TcpSocket::~TcpSocket()
{
    if(m_serbuf == nullptr) {
        delete []m_serbuf;
    }
    if(m_clibuf == nullptr) {
        delete []m_clibuf;
    }
}

/**
 * @brief TcpSocket::init
 * @param ser_ip : ip地址
 * @param ser_port : 端口号
 * @return 成功返回0，失败返回-1
 */
int TcpSocket::initServer(const string ser_ip, const int ser_port, const int bufsize)
{
    printf("[ Server ] \n");
    /// 分配 buffer
    m_serlen = bufsize;
    m_serbuf = new char[m_serlen];

    /// 1.初始化 server socket
    m_serfd = socket(AF_INET/* ipv4 */, SOCK_STREAM/* tcp*/, 0);
    if(m_serfd < 0) {
        printf("[ Server ] create socket failed ! \n");
        return -1;
    }
    /// 2. 配置 socket 地址
    struct sockaddr_in _sersock;
    memset(&_sersock, 0, sizeof(sockaddr_in));
    _sersock.sin_family = AF_INET/* ipv4 */;
    _sersock.sin_port = htons(ser_port)/* 端口号*/;
    _sersock.sin_addr.s_addr = inet_addr(ser_ip.c_str());

    /// 3. bind
    int _ret = bind(m_serfd, (sockaddr*)&_sersock, sizeof(_sersock));
    if( _ret<0 ) {
        printf("[ Server ] socket bind failed ! \n");
        return -1;
    }

    /// 4.listen
    _ret = listen(m_serfd, 10/* 最大可同时连接的数量*/);
    if( _ret<0 ) {
        printf("[ Server ] socket listen failed ! \n");
        return -1;
    }

    /// 5.accept
    printf("[ Server ] accept block.... ! \n");
    m_confd = accept(m_serfd, NULL, NULL);
    if( m_confd<0 ) {
        printf("[ Server ] socket accept failed ! \n");
        return -1;
    }

    /// 6.建立事件循环
    while(1) {
        printf("[ Server ] recv block.... ! \n");
        int _ret = recv( m_confd, (void*)m_serbuf, m_serlen, 0);
        sig_serverMsg(m_serbuf, strlen(m_serbuf));
        if(_ret < 0)
        {
            printf( "[ Server ]  is empty !!!\n" );
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }
        else if(_ret == 0)
        {
            printf( "[ Server ]  disconnect  !!!\n" );
            close( m_serfd );
            m_serfd = -1;
        }
        else
        {
            printf( "[ Server ] data = %s  !!!\n", m_serbuf);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

/**
 * @brief TcpSocket::initClient
 *
 * @param local_ip
 * @param local_port
 * @param bufsize
 * @return 成功返回0，失败返回-1
 */
int TcpSocket::initClient(const string ser_ip, const int ser_port, const int bufsize)
{
    printf("[ Client ] \n");
    /// 分配 buffer
    m_clilen = bufsize;
    m_clibuf = new char[m_clilen];

    /// 1.初始化 client socket
    m_clifd = socket(AF_INET/* ipv4 */, SOCK_STREAM/* tcp*/, 0);
    if(m_clifd < 0) {
        printf("[ Client ] create socket failed ! \n");
        return -1;
    }
    /// 2. 配置 socket 地址
    struct sockaddr_in _sersock;
    memset(&_sersock, 0, sizeof(sockaddr_in));
    _sersock.sin_family = AF_INET/* ipv4 */;
    _sersock.sin_port = htons(ser_port)/* 端口号*/;
    _sersock.sin_addr.s_addr = inet_addr(ser_ip.c_str());

    /// 3. connect
    int _serfd = connect(m_clifd, (sockaddr*)&_sersock, sizeof(_sersock));
    if( _serfd<0 ) {
        printf("[ Client ] socket connect failed ! \n");
        return -1;
    }

    /// 6.建立事件循环
    while(1) {
        printf("[ Client ] recv block.... ! \n");
        int _ret = recv(m_clifd, (void*)m_clibuf, m_clilen, 0);
        sig_clientMsg(m_clibuf, strlen(m_clibuf));
        if(_ret < 0)
        {
            printf("[ Client ] buf is empty !!!\n" );
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }
        else if(_ret == 0)
        {
            printf( "[ Client ] buf disconnect  !!!\n" );
            close( m_clifd );
            m_clifd = -1;
        }
        else
        {
            printf( "[ Client ] data = %s  !!!\n", m_clibuf);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

    }
}

/**
 * @brief TcpSocket::severSend
 *      tcp server  向 client 发送信息
 * @param buf : 信息
 * @param len : 信息长度
 * @return 成功返回0,失败返回-1
 */
int TcpSocket::severSend(const char *buf, const int len)
{
    if(m_confd < 0)
        return -1;
    send(m_confd, buf, len, 0);
    return 0;
}

/**
 * @brief TcpSocket::clientSend
 *      tcp client  向  server 发送信息
 * @param buf : 信息
 * @param len : 信息长度
 * @return 成功返回0,失败返回-1
 */
int TcpSocket::clientSend(const char *buf, const int len)
{
    if(m_clifd < 0)
        return -1;
    send(m_clifd, buf, len, 0);
    return 0;
}
