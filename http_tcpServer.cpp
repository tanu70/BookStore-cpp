//
// Created by user on 6/7/23.
//
#include "http_tcpServer.h"
#include <bits/stdc++.h>
#include <sys/socket.h>
#include "json.hpp"
#include "Base64.h"


using json = nlohmann::json;

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

    std::string TcpServer::buildResponse(std::string resBody, int resCode, std::string resMessage) {

        std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> Hello from your Server :) </p></body></html>";
        std::ostringstream ss;
        std::string responseHeader = "200 OK\n";

        responseHeader = std::to_string(resCode) + " " + resMessage + "\n";

        if(!resBody.empty()){
            htmlFile = resBody;
        }


        ss << "HTTP/1.1 "<< responseHeader <<"Content-Type: application/json\nContent-Length: " << htmlFile.size() << "\n\n"
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
            std::cout<<buffer<<std::endl;

            if(bytesReceived <0)
            {
                exitWithError("Failed to read bytes from client socket connection");
            }

            std::ostringstream ss;
            ss << "------ Received Request from client ------\n\n";
            log(ss.str());

            auto reqInfo = parseReqInfo(buffer);
            bool isAdmin = checkAdminAuth(reqInfo);

            switch (reqInfo.reqMethod) {
                case GET:
                    reqGetHandler(reqInfo);
                    break;
                case POST:
                    reqPostHandler(reqInfo);
                    break;
                case PUT:
                    reqPutHandler(reqInfo);
                    break;
                case DELETE:
                    reqDeleteHandler(reqInfo);
                default:
                    break;
            }
            //sendResponse();
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

namespace http { //Data Processing Functions

    TcpServer::ReqInfo TcpServer::parseReqInfo(char *req) {
        bool methodFlag = true, pathFlag = false, jsonFlag = false, lookingForAuth = false, authFlag = false;
        std::string method, path, body, tempStr, authStr;
        ReqInfo reqInfo;

        for (int i = 0; i < BUFFER_SIZE && req[i] != 0; i++) {

            if (methodFlag) {
                if (req[i] == ' ') {
                    methodFlag = false;
                    pathFlag = true;
                } else {
                    method += req[i];
                }
            } else if (pathFlag) {
                if (req[i] == '/') {
                    if (!path.empty()) {
                        reqInfo.pathVariables.push_back(path);

                    }
                    path = "";
                } else if (req[i] == ' ') {
                    if (!path.empty()) {
                        reqInfo.pathVariables.push_back(path);

                    }
                    path = "";
                    pathFlag = false;
                    lookingForAuth = true;
                    std::cout<<i<<std::endl;
                } else {
                    path += req[i];
                }
            }
            else if(lookingForAuth)
            {
                if((int)req[i]==10){
                    std::stringstream ss(tempStr);
                    std::string tmp,a,b;
                    ss >> tmp;
                    std::cout<<tmp<<std::endl;
                    if(tmp == "Authorization:"){
                        ss>>a>>b;
                        reqInfo.authToken = b;
                        lookingForAuth = false;
                    }
                    tempStr = "";
                }
                else{
                    tempStr += req[i];
                }
            }

            if (req[i] == 10 && req[i + 1] == '{') {
                jsonFlag = true;
                lookingForAuth = false;
            } else if (jsonFlag) {
                body += req[i];
            }
        }

        if (!body.empty()) {
            reqInfo.jsonBody = json::parse(body);
        }

        if (method == "GET") {
            reqInfo.reqMethod = GET;
        } else if (method == "POST") {
            reqInfo.reqMethod = POST;
        } else if (method == "PUT") {
            reqInfo.reqMethod = PUT;
        } else if (method == "DELETE") {
            reqInfo.reqMethod = DELETE;
        }

        return reqInfo;

    }

    bool TcpServer::checkAdminAuth(http::TcpServer::ReqInfo reqInfo) {
        std::string adminToken = macaron::Base64::Encode("admin01:adminPass");
        if(adminToken == reqInfo.authToken){
            return true;
        }
        return false;
    }
}

namespace http{ // Request Handling Functions

    void TcpServer::reqGetHandler(TcpServer::ReqInfo reqInfo) {

        std::string serializedBookData;
        int sz = bookStore.size();
        if(sz>1) serializedBookData += '[';
        for(int i = 0;i<sz;i++)
        {
            json tmpBook;
            tmpBook["bookId"] = bookStore[i].bookId;
            tmpBook["bookTitle"] = bookStore[i].bookTitle;
            tmpBook["author"] = bookStore[i].author;
            tmpBook["pageCount"] = bookStore[i].pageCount;

            serializedBookData += tmpBook.dump();

            if(i<sz-1){
                serializedBookData += ',';
            }

        }
        if(sz>1)serializedBookData += ']';

        if(serializedBookData.empty()){
            serializedBookData = "[]";
        }

        m_serverMessage = buildResponse(serializedBookData);
        std::cout<< m_serverMessage<< std :: endl;
        sendResponse();

        return;
    }

    void TcpServer::reqPostHandler(TcpServer::ReqInfo reqInfo) {

        if(!checkAdminAuth(reqInfo)){
            returnErrorResponse(403, "Not Authorized");
            return;
        }
        BookInfo newBook;
        json bookInfoJson = reqInfo.jsonBody;
        newBook.bookId = nextBookId++;
        newBook.bookTitle = bookInfoJson["bookTitle"];
        newBook.author = bookInfoJson["author"];
        newBook.pageCount = bookInfoJson["pageCount"];

        bookStore.push_back(newBook);

        m_serverMessage = buildResponse("Book Created Successfully");

        sendResponse();

        return;
    }

    void TcpServer::reqPutHandler(TcpServer::ReqInfo reqInfo) {
        if(!checkAdminAuth(reqInfo)){
            returnErrorResponse(403, "Not Authorized");
            return;
        }
        int pathSz = reqInfo.pathVariables.size();
        if(pathSz ==2){
            int updateBookId = stoi(reqInfo.pathVariables[1]);
            bool updatedFlag = false;

            for(int i = 0;i<bookStore.size();i++)
            {
                if(bookStore[i].bookId == updateBookId){
                    json updateBody = reqInfo.jsonBody;
                    if(updateBody.contains("bookTitle")){
                        bookStore[i].bookTitle = updateBody["bookTitle"];
                    }
                    if(updateBody.contains("author")){
                        bookStore[i].author = updateBody["author"];
                    }
                    if(updateBody.contains("pageCount")){
                        bookStore[i].pageCount = updateBody["pageCount"];
                    }

                    updatedFlag = true;
                    break;
                }
            }

            if(!updatedFlag){
                returnErrorResponse(400, "Bad Request");
                return;
            }

            m_serverMessage = buildResponse("Updated Successfully");
            sendResponse();
        }
        else{
            returnErrorResponse(400, "Bad Request");
        }
        return;
    }

    void TcpServer::reqDeleteHandler(TcpServer::ReqInfo reqInfo) {

        if(!checkAdminAuth(reqInfo)){
            returnErrorResponse(403, "Not Authorized");
            return;
        }

        if(reqInfo.pathVariables.size()>1){
            int deleteBookId = stoi(reqInfo.pathVariables[1]);
            bool deletedFlag = false;

            for(int i = 0;i<bookStore.size();i++)
            {
                if(bookStore[i].bookId == deleteBookId){
                    bookStore.erase(bookStore.begin()+i);
                    deletedFlag = true;
                    break;
                }
            }

            if(!deletedFlag){
                returnErrorResponse(400, "Bad Request");
                return;
            }

            m_serverMessage = buildResponse("deletedSuccessfully");
            sendResponse();
        }
        else{
            returnErrorResponse(400, "Bad Request");
        }
        return;
    }

    void TcpServer::returnErrorResponse(int code, std::string message) {

        m_serverMessage = buildResponse(message,code, message);
        sendResponse();
    }



}


