//
// Created by user on 6/7/23.
//
#include <bits/stdc++.h>
#include <arpa/inet.h>

#ifndef UNTITLED_HTTP_TCPSERVER_H
#define UNTITLED_HTTP_TCPSERVER_H

namespace http {
    class TcpServer{
    public:
        TcpServer(std::string ip_address, int port);
        ~TcpServer();
        void startListen();
    private:
        int m_socket;
        int m_newSocket;
        std:: string m_ipAddress;
        int m_port;
        int m_incomingMessage;
        struct sockaddr_in m_socketAddress;
        unsigned int m_socketAddressLen;
        std::string m_serverMessage;

        int startServer();
        void closeServer();
        std::string buildResponse();
        void acceptConnection();
        void sendResponse();

        int reqCount=0;

    };
}

#endif //UNTITLED_HTTP_TCPSERVER_H
