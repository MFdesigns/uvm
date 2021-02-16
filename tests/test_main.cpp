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

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../src/uvm.hpp"
#include "./lib/doctest.h"
#include "test_data.hpp"
#include <cstdint>
#include <iostream>

TEST_CASE("testing file header validation") {
    // Dummy instance
    HeaderInfo hi;

    bool validHeaderRes =
        validateHeader(&hi, FILE_HEADER.data(), FILE_HEADER.size());
    bool invalidMgcRes = validateHeader(&hi, FILE_HEADER_INVALID_MAGIC.data(),
                                        FILE_HEADER_INVALID_MAGIC.size());
    bool invalidSizeRes = validateHeader(&hi, FILE_HEADER_INVALID_SIZE.data(),
                                         FILE_HEADER_INVALID_SIZE.size());
    bool invalidModeRes = validateHeader(&hi, FILE_HEADER_INVALID_MODE.data(),
                                         FILE_HEADER_INVALID_MODE.size());
    bool invalidVersRes =
        validateHeader(&hi, FILE_HEADER_INVALID_VERSION.data(),
                       FILE_HEADER_INVALID_VERSION.size());
    bool invalidStartAddrRes =
        validateHeader(&hi, FILE_HEADER_INVALID_START_ADDR.data(),
                       FILE_HEADER_INVALID_START_ADDR.size());

    CHECK(validHeaderRes == true);
    CHECK(invalidMgcRes == false);
    CHECK(invalidSizeRes == false);
    CHECK(invalidModeRes == false);
    CHECK(invalidVersRes == false);
}
