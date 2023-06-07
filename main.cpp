#include <iostream>
#include "http_tcpServer.h"

int main() {
    std::cout << "Hello, World!" << std::endl;

    using namespace http;

    TcpServer server = TcpServer("0.0.0.0",8080);
    server.startListen();


    return 0;
}
