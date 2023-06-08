//
// Created by user on 6/7/23.
//
#include <bits/stdc++.h>
#include <arpa/inet.h>
#include "json.hpp"

using json = nlohmann::json;

#ifndef UNTITLED_HTTP_TCPSERVER_H
#define UNTITLED_HTTP_TCPSERVER_H

namespace http {


    class TcpServer{
    public:
        TcpServer(std::string ip_address, int port);
        ~TcpServer();
        void startListen();

        enum RequestMethod{
            GET,
            POST,
            PUT,
            DELETE
        };

        struct ReqInfo{
            RequestMethod reqMethod;
            json jsonBody;
            std::vector<std::string>pathVariables;
            int nxtMatchPathInd;
        };

    private:
        int m_socket;
        int m_newSocket;
        std:: string m_ipAddress;
        int m_port;
        int m_incomingMessage;
        struct sockaddr_in m_socketAddress;
        unsigned int m_socketAddressLen;
        std::string m_serverMessage;

        struct BookInfo{
            std::string bookTitle,author;
            int bookId, pageCount;

        };
        int nextBookId = 0;

        std::vector<BookInfo> bookStore;

        int startServer();
        void closeServer();
        std::string buildResponse(std::string resBody = "", int resCode = 200, std::string resMessage = "OK");
        void acceptConnection();
        void sendResponse();

        json getJsonBody(char* req);
        RequestMethod getReqMethod(char* req);
        ReqInfo parseReqInfo(char* req);

        void reqGetHandler(ReqInfo reqInfo);
        void reqPostHandler(ReqInfo reqInfo);
        void reqPutHandler(ReqInfo reqInfo);
        void reqDeleteHandler(ReqInfo reqInfo);
        void returnErrorResponse(int code = 404,std::string message = "Not Found");

        int reqCount=0;



    };
}

#endif //UNTITLED_HTTP_TCPSERVER_H
