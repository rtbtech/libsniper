// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

/*
 * Copyright (c) 2018 - 2019, MetaHash
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstring>
#include "varint.h"

namespace sniper::mh {

namespace {

constexpr uint8_t BYTED_2 = 0xfaU;
constexpr uint8_t BYTED_4 = 0xfbU;
constexpr uint8_t BYTED_8 = 0xfcU;

} // namespace

uint8_t append_varint(uint64_t value, string& out)
{
    auto* p_int = reinterpret_cast<char*>(&value);

    if (value < 0xfaULL) {
        out.append(p_int, 1);
        return 1;
    }
    else if (value <= 0xffffULL) {
        out.append(reinterpret_cast<const char*>(&BYTED_2), 1);
        out.append(p_int, 2);
        return 3;
    }
    else if (value <= 0xffffffffULL) {
        out.append(reinterpret_cast<const char*>(&BYTED_4), 1);
        out.append(p_int, 4);
        return 5;
    }
    else {
        out.append(reinterpret_cast<const char*>(&BYTED_8), 1);
        out.append(p_int, 8);
        return 9;
    }
}

uint8_t read_varint(string_view data, uint64_t& varint)
{
    if (data.empty())
        return 0;

    auto data_0 = static_cast<uint8_t>(data[0]);
    if (data_0 < BYTED_2) {
        varint = data_0;
        return 1;
    }

    switch (data_0) {
        case BYTED_2: {
            if (data.size() < 3)
                return 0;

            memcpy(&varint, data.data()+1, 2);
            return 3;
        }
        case BYTED_4: {
            if (data.size() < 5)
                return 0;

            memcpy(&varint, data.data()+1, 4);
            return 5;
        }
        default: {
            if (data.size() < 9)
                return 0;

            memcpy(&varint, data.data()+1, 8);
            return 9;
        }
    }
}

} // namespace sniper::mh
