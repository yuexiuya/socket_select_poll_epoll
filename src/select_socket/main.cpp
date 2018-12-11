#include "SelectSocket.h"
#include <iostream>
#include <thread>

#define TCP_SERVER_IP "127.0.0.1"
#define TCP_SERVER_PROT 8080

using namespace std::placeholders;

int main(void) {
    SelectSocket server;
    ///　启动　server 端
    std::thread ser_thread(std::bind(&SelectSocket::initServer, &server, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
                           TCP_SERVER_IP,
                           TCP_SERVER_PROT,
                           2048);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    SelectSocket client;
    std::thread cli_thread(std::bind(&SelectSocket::initClient, &client, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
                           TCP_SERVER_IP,
                           TCP_SERVER_PROT,
                           2048);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    ser_thread.join();
    cli_thread.join();

    return 0;
}
