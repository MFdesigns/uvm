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

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <map>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

constexpr char* PORT = "2001";
constexpr size_t REC_BUFFER_SIZE = 1024;
constexpr char* HTTP_VERSION = "HTTP/1.1";

enum class HTTPMethod {
    GET,
    HEAD,
    POST,
    PUT,
    _DELETE,
    CONNECT,
    OPTIONS,
    TRACE,
};

enum class ReqParseState {
    METHOD,
    PATH,
    VERSION,
    HEADER_KEY,
    HEADER_VAL,
    BODY
};

enum class ResponseCode {
    OK_200,
    BAD_REQUEST_400,
};

struct Response {
    ResponseCode Code = ResponseCode::OK_200;
    std::map<std::string, std::string> Headers;
    std::stringstream Body;
    std::stringstream Stream;
    void fillBuffer();
};

struct RequestParser {
    HTTPMethod Type;
    std::string Path;
    std::string Version;
    std::map<std::string, std::string> Headers;
    ReqParseState State = ReqParseState::METHOD;
    size_t Size = 0;
    uint8_t* Buffer = nullptr;
    uint8_t* Content = nullptr;
    uint32_t ContentLength = 0;
    size_t Cursor = 0;
    size_t BaseCursor = 0;
    size_t TmpKeyBase = 0;
    size_t TmpKeySize = 0;
    size_t TmpValBase = 0;
    size_t BodyStart = 0;
    uint8_t eatChar();
    uint8_t peekChar();
    void addReqBuffer(uint8_t* buff, size_t size);
    uint32_t hasContentLengthHeader();
    bool validateMethod(std::string& method);
    bool parse();
    void reset();
    ~RequestParser() {
        if (Content != nullptr) {
            delete[] Content;
        }
    }
};

struct HTTPServer {
    SOCKET ListenSock = INVALID_SOCKET;
    SOCKET ClientSock = INVALID_SOCKET;
    uint8_t RecBuffer[REC_BUFFER_SIZE];
    bool startup();
    void listenLoop(RequestParser& rq);
    void sendReq(std::ostream& stream);
    void shutdownSock();
    void close();
};
