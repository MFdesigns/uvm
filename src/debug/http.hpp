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

#pragma once

#include <cstring>
#include <map>
#include <sstream>

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
    /** HTTP response code */
    ResponseCode Code = ResponseCode::OK_200;
    /** Header fields key value pair */
    std::map<std::string, std::string> Headers;
    /** Response body */
    std::stringstream Body;
    /** Response body stream */
    std::stringstream Stream;

    void fillBuffer();
};

struct RequestParser {
    /** HTTP method */
    HTTPMethod Type;
    /** HTTP header path */
    std::string Path;
    /** HTTP version */
    std::string Version;
    /** Header fields key value pair */
    std::map<std::string, std::string> Headers;
    /** Current parser state */
    ReqParseState State = ReqParseState::METHOD;
    /** current buffer size */
    size_t Size = 0;
    /** current message body */
    uint8_t* Buffer = nullptr;
    /** Pointer to content inside message buffer */
    uint8_t* Content = nullptr;
    /** Content size */
    uint32_t ContentLength = 0;
    /** Current parser cursor */
    size_t Cursor = 0;
    /** Cursor to base */
    size_t BaseCursor = 0;
    /** Cursor to currently parsed header fields key */
    size_t TmpKeyBase = 0;
    /** Size of currently parsed header fields key */
    size_t TmpKeySize = 0;
    /** Cursor to currently parsed header fields value */
    size_t TmpValBase = 0;
    /** Cursor to start of message body */
    size_t BodyStart = 0;

    ~RequestParser();
    uint8_t eatChar();
    uint8_t peekChar();
    void addReqBuffer(uint8_t* buff, size_t size);
    uint32_t hasContentLengthHeader();
    bool validateMethod(std::string& method);
    bool parse();
    void reset();
};

struct HTTPServer {
    /** Unix socket listening for incoming requests */
    uint32_t UnixListenSock = -1;
    /** Unix socket handling requests */
    uint32_t UnixClientSock = -1;
    /** WinSocket listening for incoming requests */
    uint64_t* ListenSock = nullptr;
    /** WinSocket socket handling requests */
    uint64_t* ClientSock = nullptr;
    /** Buffer containing incoming messages */
    uint8_t RecBuffer[REC_BUFFER_SIZE];

    bool startup();
    void listenLoop(RequestParser& rq);
    void sendReq(std::ostream& stream);
    void shutdownSock();
    void closeServer();
};
