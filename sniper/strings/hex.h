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

#pragma once

#include <sniper/std/array.h>
#include <sniper/std/string.h>
#include <sniper/std/tuple.h>

namespace sniper::strings {

// dst size should be at least 2x src size
void bin2hex(const unsigned char* src, size_t len, char* dst) noexcept;

// dst size should be at least 1/2 src size
size_t hex2bin(const char* src, size_t len, unsigned char* dst) noexcept;

template<size_t SIZE>
void bin2hex(const std::array<unsigned char, SIZE>& src, std::array<char, SIZE * 2>& dst) noexcept
{
    bin2hex(src.data(), SIZE, dst.data());
}

void bin2hex_append(const unsigned char* src, size_t len, string& dst) noexcept;

inline void bin2hex_append(string_view src, string& dst) noexcept
{
    bin2hex_append(reinterpret_cast<const unsigned char*>(src.data()), src.size(), dst);
}

[[nodiscard]] inline uint64_t hex2bin_size(uint64_t size) noexcept
{
    return ((size + 1ull) & ~1ull) / 2;
}

// return start, len
tuple<size_t, size_t> hex2bin_append(string_view src, string& dst);

template<typename I>
inline void int2hex_append(I w, string& dst)
{
    static const char hex[] = "0123456789abcdef";

    size_t hex_len = sizeof(I) << 1ull;

    for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4) {
        uint8_t idx = (w >> j) & 0x0f;

        if (!idx && dst.empty())
            continue;

        dst.push_back(hex[idx]);
    }
}

} // namespace sniper::strings
