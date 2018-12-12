#include "EpollSocket.h"
#include <thread>

EpollSocket::EpollSocket()
{
    this->setEvlen(m_evlen);
}

EpollSocket::~EpollSocket()
{
    if(m_serbuf != nullptr) {
        delete []m_serbuf;
        m_serbuf = nullptr;
        m_cbuflen = -1;
    }
    if(m_clibuf != nullptr) {
        delete []m_clibuf;
        m_clibuf = nullptr;
        m_cbuflen = -1;
    }
    if(m_events != nullptr) {
        delete []m_events;
        m_events = nullptr;
        m_evlen = -1;
    }
}

/**
 * @brief TcpSocket::init
 * @param ser_ip : ip地址
 * @param ser_port : 端口号
 * @return 成功返回0，失败返回-1
 */
int EpollSocket::initServer(const string ser_ip, const int ser_port, const int bufsize)
{
    printf("[ Server ] init \n");
    /// 分配 buffer
    setSbuflen(bufsize);

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

    /// 5. 建立 select - epoll　工作模式
    /// 创立 epoll fd
    int _epollfd = epoll_create(m_evlen);
    if(_epollfd < 0) {
        printf("[ Server ] epoll_create error ! \n");
        return -1;
    }
    /// 向 epoll　注册
    struct epoll_event _ev;
    _ev.events = EPOLLIN;
    _ev.data.fd = m_serfd;
    int _ctlfd = epoll_ctl(_epollfd, EPOLL_CTL_ADD, m_serfd, &_ev);
    if(_ctlfd < 0) {
        printf("[ Server ] epoll_ctl error ! \n");
        return -1;
    }

    int _nfds = 0;
    while(1) {
        /// epoll
        _nfds = epoll_wait(_epollfd, m_events, m_evlen, -1 /* 阻塞式等待*/);
        if(_nfds < 0)
            continue;
        for(int i=0; i<_nfds; i++) {
            ///如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。
            if( (m_events[i].data.fd == m_serfd) && (m_events[i].events & EPOLLIN) ) {
                struct sockaddr_in _client; /// 这里可以获取到 client 的地址
                socklen_t _client_len = sizeof(_client);
                int _confd = accept(m_serfd, (sockaddr *)&_client, &_client_len);
                if( _confd < 0 ) {
                    printf("[ Server ] socket accept failed ! \n");
                    continue;
                } else {
                    printf("[ Server ] get a new client : %s \n", inet_ntoa(_client.sin_addr));
                    _ev.events = EPOLLIN | EPOLLET;
                    _ev.data.fd = _confd;
                    epoll_ctl(_epollfd,EPOLL_CTL_ADD,_confd,&_ev);
                }
            }///如果是已经连接的用户，并且收到数据，那么进行读入
            else if( (m_events[i].data.fd != m_serfd) && (m_events[i].events & EPOLLIN) ) {
                if(m_events[i].data.fd < 0)
                    continue;
                int _ret = recv( m_events[i].data.fd, (void*)m_serbuf, m_sbuflen, 0);
                if(_ret > 0)
                {
                     printf( "[ Server ] src : %d, data = %s  !!!\n", m_events[i].data.fd, m_serbuf);
                }
                else
                {
                    printf( "[ Server ] disconnect !!!\n");
                    close(m_events[i].data.fd);
                    epoll_ctl(_epollfd, EPOLL_CTL_DEL, m_events[i].data.fd,NULL);
                }
            } ///如果有数据发送
            else if( (m_events[i].data.fd != m_serfd) && (m_events[i].events & EPOLLOUT) ) {

            }
        }
    }
    close(m_serfd);
    return 0;
}

/**
 * @brief EpollSocket::initClient
 *      初始化客户端
 * @param ser_ip : server ip
 * @param ser_port : server port
 * @param bufsize : buf len
 * @return 成功返回0，失败返回-1
 */
int EpollSocket::initClient(const string ser_ip, const int ser_port, const int bufsize)
{
    printf("[ Client ] init \n");
    /// 分配 buffer
    m_cbuflen = bufsize;
    m_clibuf = new char[m_cbuflen];

    /// 1.初始化 client socket
    m_clifd = socket(AF_INET/* ipv4 */, SOCK_STREAM/* tcp*/, 0);
    if(m_clifd < 0) {
        printf("[ Client ] create socket failed ! \n");
        return -1;
    }
    sig_clientfd(m_clifd);
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
        int _ret = recv(m_clifd, (void*)m_clibuf, m_cbuflen, 0);
//        sig_clientMsg(m_clibuf, strlen(m_clibuf));
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


int EpollSocket::severSend(const char *buf, const int len)
{

}

/**
 * @brief EpollSocket::clientSend
 *      tcp client  向  server 发送信息
 * @param sockfd : client socket fd
 * @param buf : 信息
 * @param len : 信息长度
 * @return 成功返回0,失败返回-1
 */
int EpollSocket::clientSend(const int sockfd, const char *buf, const int len)
{
    return send(sockfd, buf, len, 0);
}



/**
 * @brief EpollSocket::setSbuflen
 *      设置　server 端 buf 长度
 * @param sbuflen : len
 */
void EpollSocket::setSbuflen(int sbuflen)
{
    m_sbuflen = sbuflen;
    if(m_serbuf != nullptr) {
        delete []m_serbuf;
        m_serbuf = nullptr;
    }
    m_serbuf = new char[m_sbuflen];
    memset(m_serbuf, 0, m_sbuflen);
}

/**
 * @brief EpollSocket::setSbuflen
 *      设置　client 端 buf 长度
 * @param cbuflen : len
 */
void EpollSocket::setCbuflen(int cbuflen)
{
    m_cbuflen = cbuflen;
    if(m_clibuf != nullptr) {
        delete []m_clibuf;
        m_clibuf = nullptr;
    }
    m_clibuf = new char[m_cbuflen];
    memset(m_clibuf, 0, m_cbuflen);
}

/**
 * @brief EpollSocket::setEvlen
 *      设置　可监听的　client 的最大值
 * @param evlen
 */
void EpollSocket::setEvlen(int evlen)
{
    m_evlen = evlen;
    if(m_events != nullptr) {
        delete []m_events;
        m_events = nullptr;
    }
    m_events = new epoll_event[m_evlen];
    memset(m_events, -1, m_evlen);
}
