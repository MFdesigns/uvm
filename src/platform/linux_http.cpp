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

#include "../debug/http.hpp"
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

bool HTTPServer::startup() {
    uint16_t portNr = atoi(PORT);

    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons(portNr);

    UnixListenSock = socket(AF_INET, SOCK_STREAM, 0);
    if (UnixListenSock == -1) {
        std::cout << "Error could not create listen socket\n";
        return false;
    }

    uint32_t socketBindResult =
        bind(UnixListenSock, reinterpret_cast<sockaddr*>(&sin), sizeof(sin));
    if (socketBindResult == -1) {
        std::cout << "Error could not bind socket\n";
        return false;
    }

    return true;
}

void HTTPServer::listenLoop(RequestParser& rq) {
    uint32_t listenResult = listen(UnixListenSock, 5);

    if (listenResult < 0) {
        std::cout << "Error listen failed\n";
        return;
    }

    UnixClientSock = accept(UnixListenSock, NULL, NULL);
    if (UnixClientSock < 0) {
        std::cout << "Error could not accept incoming request\n";
        return;
    }

    uint32_t recResult = 0;

    do {
        recResult = read(UnixClientSock, RecBuffer, REC_BUFFER_SIZE);
        if (recResult > 0) {
            rq.addReqBuffer(RecBuffer, recResult);

            if (rq.parse()) {
                break;
            }

        } else if (recResult == 0) {
            std::cout << "Closing connection...\n";
        } else {
            std::cout << "Error receiving failed\n";
            return;
        }
    } while (recResult > 0);
}

void HTTPServer::sendReq(std::ostream& stream) {
    std::stringstream ss;
    ss << stream.rdbuf();
    std::string string = ss.str();

    uint32_t sendResult = 0;
    sendResult = write(UnixClientSock, string.c_str(), string.length());
    if (sendResult < 0) {
        std::cout << "Error sending failed\n";
        return;
    }
}

void HTTPServer::shutdownSock() {
    close(UnixClientSock);
}

void HTTPServer::closeServer() {
    close(UnixListenSock);
}
