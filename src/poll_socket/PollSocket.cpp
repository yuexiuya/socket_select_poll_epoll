#include "PollSocket.h"
#include <thread>

PollSocket::PollSocket()
{
    m_pollClients = new pollfd[m_maxfd];
    memset(m_pollClients, -1, m_maxfd);
}

PollSocket::~PollSocket()
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
}

/**
 * @brief TcpSocket::init
 * @param ser_ip : ip地址
 * @param ser_port : 端口号
 * @return 成功返回0，失败返回-1
 */
int PollSocket::initServer(const string ser_ip, const int ser_port, const int bufsize)
{
    printf("[ Server ] \n");
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

    /// 5. 建立 select - poll　工作模式
    m_pollClients[0].fd = m_serfd;
    m_pollClients[0].events = POLLIN/** 有数据可读*/;
    m_nfds = 1;

    while(1) {
        /// poll 模式
        printf("[ Server ] poll block.... ! \n");
        int _ret = poll(m_pollClients, m_nfds/*len of pollfd */, -1 /** block waiting*/);
        if(_ret < 0) {
            printf("[ Server ] poll error \n");
            continue;
        } else {
            /// 监听有无新的client
            if(m_pollClients[0].revents & POLLIN) {
                struct sockaddr_in _client; /// 这里可以获取到 client 的地址
                socklen_t _client_len = sizeof(_client);
                int _confd = accept(m_serfd, (sockaddr *)&_client, &_client_len);
                if( _confd < 0 ) {
                    printf("[ Server ] socket accept failed ! \n");
                    continue;
                } else {
                    printf("[ Server ] get a new client : %s \n", inet_ntoa(_client.sin_addr));
                    /// 计算填充位置
                    int _pos = 0;
                    for(int i = 1; i < m_maxfd; i++)
                    {
                        if(m_pollClients[i].fd < 0)
                        {
                            _pos = i;
                            break;
                        }
                    }
                    /// 判断client是否已满
                    if(_pos >= m_maxfd) {
                        printf("[ Server ] client is max, can add it to lists \n");
                        close(_confd);
                        continue;
                    }
                    ///　填充m_pollClients
                    m_pollClients[_pos].fd = _confd;
                    m_pollClients[_pos].events = POLLIN;
                    if(_pos + 1 > m_nfds)   /// m_nfds 为最右端有效值的位置
                        m_nfds = _pos + 1;
                }
            }
            /// 遍历，监听client端的信息
            int _pos2 = 1;
            for(int i=1; i<m_nfds; i++) {
                if(m_pollClients[i].revents & POLLIN) {
                    int _ret = recv( m_pollClients[i].fd, (void*)m_serbuf, m_sbuflen, 0);
                    if(_ret < 0)
                    {
                        printf( "[ Server ]  info is empty !!!\n" );
                        _pos2 = i;
                        continue;
                    }
                    else if(_ret == 0)
                    {
                        printf( "[ Server ]  disconnect  !!!\n" );
                        close( m_pollClients[i].fd );
                        m_pollClients[i].fd = -1;
                    }
                    else
                    {
                        printf( "[ Server ] data = %s  !!!\n", m_serbuf);
                        _pos2 = i;
                    }
                }
            }
            if(_pos2 < m_nfds)
                m_nfds = _pos2;
        }
    }
}

///**
// * @brief TcpSocket::clientSend
// *      tcp client  向  server 发送信息
// * @param buf : 信息
// * @param len : 信息长度
// * @return 成功返回0,失败返回-1
// */
int PollSocket::initClient(const string ser_ip, const int ser_port, const int bufsize)
{
    printf("[ Client ] \n");
    /// 分配 buffer
    m_cbuflen = bufsize;
    m_clibuf = new char[m_cbuflen];

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

/**
 * @brief PollSocket::setSbuflen
 *      设置　server 端 buf 长度
 * @param sbuflen : len
 */
void PollSocket::setSbuflen(int sbuflen)
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
 * @brief PollSocket::setSbuflen
 *      设置　client 端 buf 长度
 * @param cbuflen : len
 */
void PollSocket::setCbuflen(int cbuflen)
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
 * @brief PollSocket::setMaxfd
 *      设置　可监听的　client 的最大值
 * @param maxfd
 */
void PollSocket::setMaxfd(int maxfd)
{
    m_maxfd = maxfd;
    if(m_pollClients != nullptr) {
        delete []m_pollClients;
        m_pollClients = nullptr;
    }
    m_pollClients = new pollfd[m_maxfd];
    memset(m_pollClients, -1, m_maxfd);
}
