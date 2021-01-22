// ======================================================================== //
// Copyright 2021 Michel Fäh
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ======================================================================== //

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include "../debug/http.hpp"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

bool HTTPServer::startup() {
    WSADATA wsaData{};
    // MAKEWORD = Version 2.2
    uint32_t wsaStatus = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaStatus != 0) {
        std::cout << "Error: Failed to create WSADATA\n";
        return false;
    }

    ADDRINFOA addrInfoHints{};
    ADDRINFOA* addrInfoResult = nullptr;

    ZeroMemory(&addrInfoHints, sizeof(addrInfoHints));
    addrInfoHints.ai_family = AF_INET;
    addrInfoHints.ai_socktype = SOCK_STREAM;
    addrInfoHints.ai_protocol = IPPROTO_TCP;
    addrInfoHints.ai_flags = AI_PASSIVE;

    uint32_t addrIntoStatus =
        getaddrinfo(NULL, PORT, &addrInfoHints, &addrInfoResult);
    if (addrIntoStatus != 0) {
        std::cout << "Error: getaddrinfo failed\n";
        WSACleanup();
        return false;
    }

    ListenSock = reinterpret_cast<uint64_t*>(INVALID_SOCKET);
    ListenSock = reinterpret_cast<uint64_t*>(
        socket(addrInfoResult->ai_family, addrInfoResult->ai_socktype,
               addrInfoResult->ai_protocol));
    if (ListenSock == reinterpret_cast<uint64_t*>(INVALID_SOCKET)) {
        std::cout << "Error [" << WSAGetLastError()
                  << "]: could not create socket\n";
        freeaddrinfo(addrInfoResult);
        WSACleanup();
        return false;
    }

    uint32_t socketBindResult =
        bind(reinterpret_cast<SOCKET>(ListenSock), addrInfoResult->ai_addr,
             addrInfoResult->ai_addrlen);
    if (socketBindResult == SOCKET_ERROR) {
        std::cout << "Error [" << WSAGetLastError()
                  << "]: could not bind socket\n";
        freeaddrinfo(addrInfoResult);
        closesocket(reinterpret_cast<SOCKET>(ListenSock));
        WSACleanup();
        return false;
    }

    // No longer needed after socket bind
    freeaddrinfo(addrInfoResult);

    return true;
}

void HTTPServer::listenLoop(RequestParser& rq) {
    if (listen(reinterpret_cast<SOCKET>(ListenSock), SOMAXCONN) ==
        SOCKET_ERROR) {
        std::cout << "Error [" << WSAGetLastError() << "]: listen failed\n";
        closesocket(reinterpret_cast<SOCKET>(ListenSock));
        WSACleanup();
        return;
    }

    ClientSock = reinterpret_cast<uint64_t*>(INVALID_SOCKET);
    ClientSock = reinterpret_cast<uint64_t*>(
        accept(reinterpret_cast<SOCKET>(ListenSock), NULL, NULL));
    if (ClientSock == reinterpret_cast<uint64_t*>(INVALID_SOCKET)) {
        std::cout << "Error [" << WSAGetLastError()
                  << "]: could not accept incoming request\n";
        closesocket(reinterpret_cast<SOCKET>(ListenSock));
        WSACleanup();
        return;
    }

    uint32_t recResult = 0;

    do {
        recResult =
            recv(reinterpret_cast<SOCKET>(ClientSock),
                 reinterpret_cast<char*>(RecBuffer), REC_BUFFER_SIZE, 0);
        if (recResult > 0) {
            rq.addReqBuffer(RecBuffer, recResult);

            if (rq.parse()) {
                break;
            }

        } else if (recResult == 0) {
            std::cout << "Closing connection...\n";
        } else {
            std::cout << "Error [" << WSAGetLastError()
                      << "]: receiving failed\n";
            closesocket(reinterpret_cast<SOCKET>(ClientSock));
            WSACleanup();
            return;
        }
    } while (recResult > 0);
}

void HTTPServer::sendReq(std::ostream& stream) {
    std::stringstream ss;
    ss << stream.rdbuf();
    std::string string = ss.str();

    uint32_t sendResult = 0;
    sendResult = send(reinterpret_cast<SOCKET>(ClientSock), string.c_str(),
                      string.length(), 0);
    if (sendResult == SOCKET_ERROR) {
        std::cout << "Error [" << WSAGetLastError() << "]: sending failed\n";
        closesocket(reinterpret_cast<SOCKET>(ClientSock));
        WSACleanup();
        return;
    }
}

void HTTPServer::shutdownSock() {
    uint32_t shutdownResult =
        shutdown(reinterpret_cast<SOCKET>(ClientSock), SD_SEND);
    if (shutdownResult == SOCKET_ERROR) {
        std::cout << "Error [" << WSAGetLastError() << "]: shutdown failed\n";
        closesocket(reinterpret_cast<SOCKET>(ClientSock));
        WSACleanup();
        return;
    }

    closesocket(reinterpret_cast<SOCKET>(ClientSock));
}

void HTTPServer::closeServer() {
    WSACleanup();
}
