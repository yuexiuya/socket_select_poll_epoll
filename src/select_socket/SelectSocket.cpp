#include "SelectSocket.h"
#include <thread>


///**
// * @brief TcpSocket::severSend
// *      tcp server  向 client 发送信息
// * @param buf : 信息
// * @param len : 信息长度
// * @return 成功返回0,失败返回-1
// */
//int TcpSocket::severSend(const char *buf, const int len)
//{
//    if(m_confd < 0)
//        return -1;
//    send(m_confd, buf, len, 0);
//    return 0;
//}

///**
// * @brief TcpSocket::clientSend
// *      tcp client  向  server 发送信息
// * @param buf : 信息
// * @param len : 信息长度
// * @return 成功返回0,失败返回-1
// */
//int TcpSocket::clientSend(const char *buf, const int len)
//{
//    if(m_clifd < 0)
//        return -1;
//    send(m_clifd, buf, len, 0);
//    return 0;
//}

SelectSocket::SelectSocket()
{
    m_socklen = 1024;
    m_sockArray = new int[m_socklen];
    memset(m_sockArray, -1, m_socklen);
    m_clientList.clear();
}

SelectSocket::~SelectSocket()
{
    if(m_serbuf != nullptr) {
        delete []m_serbuf;
        m_serbuf = nullptr;
        m_clilen = -1;
    }
    if(m_clibuf != nullptr) {
        delete []m_clibuf;
        m_clibuf = nullptr;
        m_clilen = -1;
    }
    if(m_sockArray != nullptr) {
        delete []m_sockArray;
        m_sockArray = nullptr;
        m_socklen = -1;
    }
}

/**
 * @brief TcpSocket::init
 * @param ser_ip : ip地址
 * @param ser_port : 端口号
 * @return 成功返回0，失败返回-1
 */
int SelectSocket::initServer(const string ser_ip, const int ser_port, const int bufsize)
{
    printf("[ Server ] \n");
    /// 分配 buffer
    m_serlen = bufsize;
    m_serbuf = new char[m_serlen];
    memset(m_serbuf, -1, m_serlen);

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

    /// 5. 建立 select - socket　工作模式
    fd_set _rfds;
    fd_set _wfds;
    FD_ZERO(&_rfds);
    FD_ZERO(&_wfds);
    FD_SET(m_serfd, &_rfds);
    FD_SET(m_serfd, &_wfds);
    m_maxfd = m_serfd;

    while(1) {
        ///　注册当前连接的 client 到 fd_set 中
        /// select 模式
        printf("[ Server ] select block.... ! \n");
        int _ret = select(m_maxfd + 1, &_rfds, &_wfds, NULL, NULL);
        if(_ret < 0) {
            printf("[ Server ] select error \n");
            continue;
        } else {
            /// 监听 server fd 上的变化
            if(FD_ISSET(m_serfd, &_rfds)) {
                /// 监听有无新的client
                struct sockaddr_in _client; /// 这里可以获取到 client 的地址
                socklen_t _client_len = sizeof(_client);
                int _confd = accept(m_serfd, (sockaddr *)&_client, &_client_len);
                if( _confd < 0 ) {
                    printf("[ Server ] socket accept failed ! \n");
                    continue;
                } else {
                    printf("[ Server ] get a new client : %s \n", inet_ntoa(_client.sin_addr));
                    m_clientList.push_back(_confd);
                    FD_SET(_confd, &_rfds);
                    m_maxfd = max(_confd, m_maxfd);
                }
            }
            /// 监听现有client上的信息
            for(int i = 0; i<m_clientList.size(); i++) {
                if(m_clientList[i] < 0)
                    continue;
                if(FD_ISSET(m_serfd, &_rfds)) {
                    int _ret = recv( m_clientList[i], (void*)m_serbuf, m_serlen, 0);
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
                }
            }
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
int SelectSocket::initClient(const string ser_ip, const int ser_port, const int bufsize)
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
 * @brief SelectSocket::setSockArraySize
 * @param size : 数组大小
 */
void SelectSocket::setSockArraySize(const int size)
{
    if(m_sockArray != nullptr) {
        delete []m_sockArray;
        m_sockArray = nullptr;
    }
    m_socklen = size;
    m_sockArray = new int[m_socklen];

}
