#include "EpollSocket.h"
#include <iostream>
#include <thread>

#define TCP_SERVER_IP "127.0.0.1"
#define TCP_SERVER_PROT 8080

using namespace std::placeholders;

static int clifd_1 = -1;
static int clifd_2 = -1;

void clientfd1Callback(const int fd);

void clientfd2Callback(const int fd);

int main(void) {
    EpollSocket server;
    EpollSocket client_1;
    EpollSocket client_2;
    client_1.sig_clientfd.connect(clientfd1Callback);
    client_2.sig_clientfd.connect(clientfd2Callback);
    ///　启动　server 端
    std::thread ser_thread(std::bind(&EpollSocket::initServer, &server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
                           TCP_SERVER_IP,
                           TCP_SERVER_PROT,
                           2048);
    std::this_thread::sleep_for(std::chrono::seconds(1));


    std::thread cli_thread_1(std::bind(&EpollSocket::initClient, &client_1, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
                           TCP_SERVER_IP,
                           TCP_SERVER_PROT,
                           2048);
    std::this_thread::sleep_for(std::chrono::seconds(1));


    std::thread cli_thread_2(std::bind(&EpollSocket::initClient, &client_2, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
                           TCP_SERVER_IP,
                           TCP_SERVER_PROT,
                           2048);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    while(1) {
        if( (clifd_1 > 0) && (clifd_1 > 0) )
            break;
    }

    client_1.clientSend(clifd_1, "ABC", 3);
    client_2.clientSend(clifd_2, "123", 3);

    ser_thread.join();
    cli_thread_1.join();
    cli_thread_2.join();
    printf("Normal End \n");

    return 0;
}

/**
 * @brief clientfd1Callback
 *      监听 socket client 1 fd 的回调
 * @param fd
 */
void clientfd1Callback(const int fd)
{
    printf("callback client 1 fd = %d \n", fd);
    clifd_1 = fd;
}

/**
 * @brief clientfd2Callback
 *      监听 socket client 2 fd 的回调
 * @param fd
 */
void clientfd2Callback(const int fd)
{
    printf("callback client 2 fd = %d \n", fd);
    clifd_2 = fd;
}
