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

#include "hex.h"

namespace sniper::strings {

namespace {

const char HexLookup[513] = {
    "000102030405060708090a0b0c0d0e0f"
    "101112131415161718191a1b1c1d1e1f"
    "202122232425262728292a2b2c2d2e2f"
    "303132333435363738393a3b3c3d3e3f"
    "404142434445464748494a4b4c4d4e4f"
    "505152535455565758595a5b5c5d5e5f"
    "606162636465666768696a6b6c6d6e6f"
    "707172737475767778797a7b7c7d7e7f"
    "808182838485868788898a8b8c8d8e8f"
    "909192939495969798999a9b9c9d9e9f"
    "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf"
    "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
    "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
    "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
    "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
    "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"};

const unsigned char DecLookup[256] = {
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // gap before first hex digit
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, // 0123456789
    0,  0,  0,  0,  0,  0,  0, // :;<=>?@ (gap)
    10, 11, 12, 13, 14, 15, // ABCDEF
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, // GHIJKLMNOPQRS (gap)
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, // TUVWXYZ[/]^_` (gap)
    10, 11, 12, 13, 14, 15, // abcdef
    0,  0,  0,  0,  0,  0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 // fill zeroes
};

} // namespace

void bin2hex(const unsigned char* src, size_t len, char* dst) noexcept
{
    for (size_t i = 0; i < len; i++) {
        const char* hex = HexLookup + 2 * src[i];
        dst[i * 2] = *hex;
        dst[i * 2 + 1] = *(hex + 1);
    }
}

void bin2hex_append(const unsigned char* src, size_t len, string& dst) noexcept
{
    for (size_t i = 0; i < len; i++)
        dst.append(HexLookup + *(src + i) * 2, 2);
}

size_t hex2bin(const char* src, size_t len, unsigned char* dst) noexcept
{
    auto* data = reinterpret_cast<const unsigned char*>(src);

    size_t dst_len = 0;
    for (size_t i = 0; i < len; i += 2) {
        unsigned char d = DecLookup[data[i]] << 4u;
        d |= DecLookup[data[i + 1]];

        dst[dst_len] = d;
        dst_len++;
    }

    return dst_len;
}

tuple<size_t, size_t> hex2bin_append(string_view src, string& dst)
{
    auto* data = reinterpret_cast<const unsigned char*>(src.data());
    size_t src_len = src.length();
    size_t start_size = dst.size();

    for (size_t i = 0; i < src_len; i += 2) {
        unsigned char d = DecLookup[data[i]] << 4u;
        d |= DecLookup[data[i + 1]];
        dst.push_back(d);
    }

    return std::make_tuple(start_size, dst.size() - start_size);
}

} // namespace sniper::strings
