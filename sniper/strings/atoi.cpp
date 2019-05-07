// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

/*
 * Copyright (c) 2018 - 2019, MetaHash, Oleg Romanenko (oleg@romanenko.ro)
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

#include "atoi.h"

namespace sniper::strings {

optional<int32_t> fast_atoi32(string_view str) noexcept
{
    if (str.empty())
        return std::nullopt;

    int64_t x = 0;
    int sign = 0;
    size_t i = 0;
    const char* p = str.data();

    char c = *(p + i);
    if (c == '-') {
        sign = 1;
        c = *(p + ++i);
    }
    else if (c < '0' || c > '9') {
        return std::nullopt;
    }

    size_t len = str.size();
    for (; c > '/' && c < ':' && i < len; c = *(p + ++i))
        x = (x << 1ll) + (x << 3ll) + c - '0';

    if (x > INT32_MAX || x < INT32_MIN)
        return std::nullopt;

    return sign ? -x : x;
}

optional<int64_t> fast_atoi64(string_view str) noexcept
{
    if (str.empty())
        return std::nullopt;

    int64_t x = 0;
    int sign = 0;
    size_t i = 0;

    const char* p = str.data();

    char c = *(p + i);
    if (c == '-') {
        sign = 1;
        c = *(p + ++i);
    }
    else if (c < '0' || c > '9') {
        return std::nullopt;
    }

    size_t len = str.size();
    if (len > 18u + sign)
        return std::nullopt;

    for (; c > '/' && c < ':' && i < len; c = *(p + ++i))
        x = (x << 1ll) + (x << 3ll) + c - '0';

    return sign ? -x : x;
}

} // namespace sniper::strings
