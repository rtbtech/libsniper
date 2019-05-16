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

#include <cstring>
#include <endian.h>
#include "utils.h"
#include "lz4.h"

namespace sniper::lz4 {

bool compress_raw_block(string_view src, string& dst)
{
    if (src.empty())
        return false;

    int bound_size = LZ4_compressBound(src.size());
    if (!bound_size)
        return false;

    if (dst.size() < (uint32_t)bound_size)
        dst.resize(bound_size);


    if (auto lz4_size = LZ4_compress_default(src.data(), dst.data(), src.size(), dst.size()); lz4_size) {
        if (lz4_size != bound_size)
            dst.resize(lz4_size);
        return true;
    }

    return false;
}

bool compress_uint32_block(string_view src, string& dst)
{
    if (src.empty())
        return false;

    int bound_size = LZ4_compressBound(src.size());
    if (!bound_size)
        return false;

    bound_size += sizeof(uint32_t);


    if (dst.size() < (uint32_t)bound_size)
        dst.resize(bound_size);

    int lz4_size =
        LZ4_compress_default(src.data(), dst.data() + sizeof(uint32_t), src.size(), dst.size() - sizeof(uint32_t));
    if (lz4_size) {
        if ((lz4_size + (int)sizeof(uint32_t)) != bound_size)
            dst.resize(lz4_size + sizeof(uint32_t));

        uint32_t header = htole32(src.size());
        memcpy(dst.data(), &header, sizeof(header));

        return true;
    }

    return false;
}

bool decompress_raw_block(string_view src, string& dst)
{
    if (src.empty())
        return false;

    int size = LZ4_decompress_safe(src.data(), dst.data(), src.size(), dst.size());
    if (size < 0)
        return false;

    if (dst.size() != (uint32_t)size)
        dst.resize(size);

    return true;
}

bool decompress_uint32_block(string_view src, string& dst, uint32_t max_size)
{
    if (src.size() <= 4)
        return false;

    uint32_t orig_size = 0;
    memcpy(&orig_size, src.data(), sizeof(uint32_t));
    orig_size = le32toh(orig_size);

    if (orig_size > max_size)
        return false;

    if (dst.size() < orig_size)
        dst.resize(orig_size);

    int size =
        LZ4_decompress_safe(src.data() + sizeof(uint32_t), dst.data(), src.size() - sizeof(uint32_t), dst.size());
    if (size < 0)
        return false;

    if (dst.size() != (uint32_t)size)
        dst.resize(size);

    return true;
}

} // namespace sniper::lz4
