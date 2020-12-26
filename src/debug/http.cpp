// ======================================================================== //
// Copyright 2020 Michel Fäh
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

#include "http.hpp"
#include <iostream>
#include <sstream>
#include <string>

void Response::fillBuffer() {
    // Clear previous buffer
    Stream.str(std::string());
    Stream.clear();

    char* responseCode = nullptr;
    switch (Code) {
    case ResponseCode::OK_200:
        responseCode = "200 OK";
        break;
    case ResponseCode::BAD_REQUEST_400:
        responseCode = "400 Bad Request";
        break;
    }

    Stream << HTTP_VERSION << ' ' << responseCode << "\r\n";
    for (auto elem : Headers) {
        Stream << elem.first << ": " << elem.second << "\r\n";
    }
    Stream << "\r\n" << Body.rdbuf();
}

uint8_t RequestParser::eatChar() {
    if (Cursor > Size) {
        return 0;
    }
    return Buffer[Cursor++];
}

uint8_t RequestParser::peekChar() {
    if (Cursor > Size) {
        return 0;
    }
    return Buffer[Cursor];
}

void RequestParser::addReqBuffer(uint8_t* buff, size_t size) {
    size_t reallocSize = Size + size;
    uint8_t* reallocBuff = new uint8_t[reallocSize];

    if (Buffer == nullptr) {
        memcpy(reallocBuff, buff, size);
        Buffer = reallocBuff;
    } else {
        memcpy(reallocBuff, Buffer, Size);
        memcpy(&reallocBuff[Size], buff, size);
        delete[] Buffer;
    }
    Buffer = reallocBuff;
    Size = reallocSize;
}

uint32_t RequestParser::hasContentLengthHeader() {
    auto result = Headers.find("Content-Length");
    if (result == Headers.end()) {
        return 0;
    }
    uint32_t contentLength = std::stoi(result->second);
    return contentLength;
}

/**
 * Validates a string containg the HTTP method
 * @param rq Reference to RequestParser where valid method will be set
 * @param method Reference to string containing method
 * @return On valid method return true otherwise false
 */
bool RequestParser::validateMethod(std::string& method) {
    if (method == "GET") {
        Type = HTTPMethod::GET;
    } else if (method == "HEAD") {
        Type = HTTPMethod::HEAD;
    } else if (method == "POST") {
        Type = HTTPMethod::POST;
    } else if (method == "PUT") {
        Type = HTTPMethod::PUT;
    } else if (method == "DELETE") {
        Type = HTTPMethod::_DELETE;
    } else if (method == "CONNECT") {
        Type = HTTPMethod::CONNECT;
    } else if (method == "OPTIONS") {
        Type = HTTPMethod::OPTIONS;
    } else if (method == "TRACE") {
        Type = HTTPMethod::TRACE;
    } else {
        return false;
    }
    return true;
}

bool RequestParser::parse() {
    uint8_t c = eatChar();
    uint8_t p = peekChar();

    while (Cursor < Size) {
        switch (State) {
        case ReqParseState::METHOD: {
            if (p == ' ') {
                std::string method{reinterpret_cast<char*>(Buffer), BaseCursor,
                                   Cursor - BaseCursor};
                if (!validateMethod(method)) {
                    std::cout << "Error: unknown HTTP method\n";
                    return false;
                }

                // Skip space
                eatChar();
                BaseCursor = Cursor;
                State = ReqParseState::PATH;
            } else if (p == '\n') {
                std::cout << "Error: invalid HTTP method\n";
                return false;
            }
            break;
        } break;
        case ReqParseState::PATH: {
            if (p == ' ') {
                std::string path{reinterpret_cast<char*>(Buffer), BaseCursor,
                                 Cursor - BaseCursor};
                Path = std::move(path);
            } else if (p == '\n') {
                std::cout << "Error: invalid HTTP request path\n";
                return false;
            }

            // Skip space
            eatChar();
            BaseCursor = Cursor;
            State = ReqParseState::VERSION;
        } break;
        case ReqParseState::VERSION: {
            if (p == '\n' || p == '\r') {
                std::string version{reinterpret_cast<char*>(Buffer), BaseCursor,
                                    Cursor - BaseCursor};
                Version = std::move(version);
                State = ReqParseState::HEADER_KEY;

                // Set start of header key
                TmpKeyBase = Cursor + 2;

                eatChar();
                eatChar();
            }
        } break;
        case ReqParseState::HEADER_KEY: {
            if (p == ':') {
                TmpKeySize++;
                State = ReqParseState::HEADER_VAL;

                // Skip colon and space
                eatChar();
                eatChar();
                TmpValBase = Cursor;
            } else {
                TmpKeySize++;
            }
        } break;
        case ReqParseState::HEADER_VAL: {
            if (p == '\n' || p == '\r') {
                std::string key{reinterpret_cast<char*>(Buffer), TmpKeyBase,
                                TmpKeySize};
                std::string value{reinterpret_cast<char*>(Buffer), TmpValBase,
                                  Cursor - TmpValBase};
                State = ReqParseState::HEADER_KEY;

                Headers[key] = value;
                ContentLength = hasContentLengthHeader();

                // Skip new line
                if (p == '\r') {
                    eatChar();
                }
                eatChar();
                TmpKeyBase = Cursor;
                TmpKeySize = 0;

                // Check if body follows
                char tmpPeek = peekChar();
                if (tmpPeek == '\n' || tmpPeek == '\r') {
                    if (p == '\r') {
                        eatChar();
                    }
                    eatChar();
                    State = ReqParseState::BODY;
                    BodyStart = Cursor;
                }
            }
        } break;
        case ReqParseState::BODY: {
            if (ContentLength == 0) {
                // If not content-length is given assume body is empty
                return true;
            } else if (Size >= BodyStart + ContentLength) {
                Content = &Buffer[BodyStart];
                return true;
            }
        } break;
        }
        c = eatChar();
        p = peekChar();
    }

    return false;
}

void RequestParser::reset() {
    Type = HTTPMethod::GET;
    Path = "";
    Version = "";
    Headers.clear();
    State = ReqParseState::METHOD;
    Size = 0;
    ContentLength = 0;
    Cursor = 0;
    BaseCursor = 0;
    TmpKeyBase = 0;
    TmpKeySize = 0;
    TmpValBase = 0;
    BodyStart = 0;

    if (Buffer != nullptr) {
        delete[] Buffer;
    }

    Buffer = nullptr;
    Content = nullptr;
}

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

    ListenSock = INVALID_SOCKET;
    ListenSock = socket(addrInfoResult->ai_family, addrInfoResult->ai_socktype,
                        addrInfoResult->ai_protocol);
    if (ListenSock == INVALID_SOCKET) {
        std::cout << "Error [" << WSAGetLastError()
                  << "]: could not create socket\n";
        freeaddrinfo(addrInfoResult);
        WSACleanup();
        return false;
    }

    uint32_t socketBindResult =
        bind(ListenSock, addrInfoResult->ai_addr, addrInfoResult->ai_addrlen);
    if (socketBindResult == SOCKET_ERROR) {
        std::cout << "Error [" << WSAGetLastError()
                  << "]: could not bind socket\n";
        freeaddrinfo(addrInfoResult);
        closesocket(ListenSock);
        WSACleanup();
        return false;
    }

    // No longer needed after socket bind
    freeaddrinfo(addrInfoResult);

    return true;
}

void HTTPServer::listenLoop(RequestParser& rq) {
    if (listen(ListenSock, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "Error [" << WSAGetLastError() << "]: listen failed\n";
        closesocket(ListenSock);
        WSACleanup();
        return;
    }

    ClientSock = INVALID_SOCKET;
    ClientSock = accept(ListenSock, NULL, NULL);
    if (ClientSock == INVALID_SOCKET) {
        std::cout << "Error [" << WSAGetLastError()
                  << "]: could not accept incoming request\n";
        closesocket(ListenSock);
        WSACleanup();
        return;
    }

    uint32_t recResult = 0;

    do {
        recResult = recv(ClientSock, reinterpret_cast<char*>(RecBuffer),
                         REC_BUFFER_SIZE, 0);
        if (recResult > 0) {
            rq.addReqBuffer(RecBuffer, recResult);
            std::cout << "Bytes received: " << recResult << '\n';

            if (rq.parse()) {
                break;
            }

        } else if (recResult == 0) {
            std::cout << "Closing connection...\n";
        } else {
            std::cout << "Error [" << WSAGetLastError()
                      << "]: receiving failed\n";
            closesocket(ClientSock);
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
    sendResult = send(ClientSock, string.c_str(), string.length(), 0);
    if (sendResult == SOCKET_ERROR) {
        std::cout << "Error [" << WSAGetLastError() << "]: sending failed\n";
        closesocket(ClientSock);
        WSACleanup();
        return;
    }
    std::cout << "Bytes sent: " << string.length() << '\n';
}

void HTTPServer::shutdownSock() {
    uint32_t shutdownResult = shutdown(ClientSock, SD_SEND);
    if (shutdownResult == SOCKET_ERROR) {
        std::cout << "Error [" << WSAGetLastError() << "]: shutdown failed\n";
        closesocket(ClientSock);
        WSACleanup();
        return;
    }

    closesocket(ClientSock);
}

void HTTPServer::close() {
    WSACleanup();
}
