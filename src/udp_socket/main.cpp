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
    UdpSocket server_udp;
//    server_udp.init(UDP_SERVER_IP, UDP_SERVER_PROT,
//              UDP_CLIENT_IP, UDP_CLIENT_PROT);
//    std::thread ser_thread(std::bind(&UdpSocket::runServer,&server_udp));
//    std::this_thread::sleep_for(std::chrono::seconds(1));
//    UdpSocket client_udp;
//    client_udp.init(UDP_SERVER_IP, UDP_SERVER_PROT,
//              UDP_CLIENT_IP, UDP_CLIENT_PROT);
//    client_udp.runClient();
    while(1);
    return 0;
}
