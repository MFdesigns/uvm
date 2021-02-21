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

/**
 * Builds the response message and fills it into the buffer
 */
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

/**
 * Destructor
 */
RequestParser::~RequestParser() {
    if (Content != nullptr) {
        delete[] Content;
    }
}

/**
 * Returns char at cursor position and increases cursor
 * @return Char at cursor position
 */
uint8_t RequestParser::eatChar() {
    if (Cursor > Size) {
        return 0;
    }
    return Buffer[Cursor++];
}

/**
 * Returns char at cursor position without incrementing cursor
 * @return Char at cursor position
 */
uint8_t RequestParser::peekChar() {
    if (Cursor > Size) {
        return 0;
    }
    return Buffer[Cursor];
}

/**
 * Adds a buffer containing a segment of the incoming message to the complete
 * message buffer
 * @param buff Pointer to buffer
 * @param size Size of buffer
 */
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

/**
 * Checks if content length header field is already available
 * @return Content length value
 */
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

/**
 * Continues to parse at position where last left off
 * @return On valid input returns true otherwise false
 */
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

/**
 * Resets the request parser to be reused
 */
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
