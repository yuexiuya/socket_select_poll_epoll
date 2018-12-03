#include <iostream>
#include <stdio.h>
#include <thread>

#include "UdpSocket.h"

#define UDP_SERVER_IP "127.0.0.1"
#define UDP_SERVER_PROT 8080

#define UDP_CLIENT_IP "127.0.0.1"
#define UDP_CLIENT_PROT 8080

int main(void) {
    printf("[LOG] main \n");
    UdpSocket udpSokcet;
    udpSokcet.init(UDP_SERVER_IP, UDP_SERVER_PROT,
              UDP_CLIENT_IP, UDP_CLIENT_PROT);
    std::thread ser_thread(std::bind(&UdpSocket::runServer,&udpSokcet));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::thread cli_thread(std::bind(&UdpSocket::runClient,&udpSokcet));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    const char* _serMsg = "Server Test";
    const char* _cliMsg = "Client Test";
    udpSokcet.serverSend(_serMsg, strlen(_serMsg));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    udpSokcet.clientSend(_cliMsg, strlen(_cliMsg));
    while(1);
    return 0;
}
