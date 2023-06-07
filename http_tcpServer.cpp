//
// Created by user on 6/7/23.
//
#include "http_tcpServer.h"
#include <bits/stdc++.h>
#include <sys/socket.h>

namespace {
    const int BUFFER_SIZE = 30720;

    void log(const std::string &message) {
        std::cout<< message<<std::endl;
    }

    void exitWithError(const std::string &errorMessage) {
        log("Error: " + errorMessage);
        exit(1);
    }

}

namespace http{
    TcpServer::TcpServer(std::string ip_address, int port): m_ipAddress(ip_address), m_port(port),
                                                            m_socketAddressLen(sizeof (m_socketAddress)),
                                                            m_serverMessage(buildResponse()) {
        //std::cout<<"constd"<<std::endl;
        m_socketAddress.sin_family = AF_INET;
        m_socketAddress.sin_port = htons(m_port);
        m_socketAddress.sin_addr.s_addr = inet_addr(m_ipAddress.c_str());

        if(startServer()){
            std::ostringstream ss;
            ss << "Failed to start server with PORT: " << ntohs(m_socketAddress.sin_port);
            log(ss.str());
        }
    }

    TcpServer::~TcpServer() {
        closeServer();
    }

    int TcpServer::startServer() {
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if(m_socket<0){
            exitWithError("Cannot create socket");
            return 1;
        }

        if(bind(m_socket, (sockaddr *)&m_socketAddress, m_socketAddressLen)<0){
            exitWithError("Cannot connect socket to address");
            return 1;
        }
        std:: cout<< "Server Started" << std::endl;
        return 0;
    }

    void TcpServer::closeServer() {
        close(m_socket);
        close(m_newSocket);
        exit(0);
    }

    std::string TcpServer::buildResponse() {
        //std::cout<< "came Here";
        //return "<html>Tanu</html>";

        std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> Hello from your Server :) </p></body></html>";
        std::ostringstream ss;
        ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << htmlFile.size() << "\n\n"
           << htmlFile;

        return ss.str();
    }

    void TcpServer::startListen() {
        if(listen(m_socket, 20) <0)
        {
            exitWithError("Socket listen Failed!!!");
        }

        std:: ostringstream ss;
        ss << "\n*** Listening on ADDRESS: "
           << inet_ntoa(m_socketAddress.sin_addr)
           << " PORT: " << ntohs(m_socketAddress.sin_port)
           << " ***\n\n";

        log(ss.str());
        int bytesReceived;
        while(true)
        {
            char buffer[BUFFER_SIZE] = {0};
            acceptConnection();
            bytesReceived = read(m_newSocket, buffer, BUFFER_SIZE);

            if(bytesReceived <0)
            {
                exitWithError("Failed to read bytes from client socket connection");
            }

            std::ostringstream ss;
            ss << "------ Received Request from client ------\n\n";
            log(ss.str());

            sendResponse();
            close(m_newSocket);

        }
    }

    void TcpServer::acceptConnection() {
        m_newSocket = accept(m_socket,(sockaddr *)&m_socketAddress, &m_socketAddressLen);

        if(m_newSocket<0)
        {
            std::ostringstream ss;
            ss << "Server failed to accept incoming connection from ADDRESS: " <<
            inet_ntoa(m_socketAddress.sin_addr) << "; PORT: " << ntohs(m_socketAddress.sin_port);
            exitWithError(ss.str());
        }
    }

    void TcpServer::sendResponse() {
        int bytesSent;
        std::cout<< ++reqCount<<std::endl;
        bytesSent = write(m_newSocket, m_serverMessage.c_str(), m_serverMessage.size());

        if(bytesSent == m_serverMessage.size())
        {
            log("------ Server Response sent to client ------\n\n");
        }
        else{
            log("Error sending response to client");
        }
    }

}


