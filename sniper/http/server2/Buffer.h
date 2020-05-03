/*
 * Copyright (c) 2020, RTBtech, MediaSniper, Oleg Romanenko (oleg@romanenko.ro)
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

#include <sniper/cache/ArrayCache.h>
#include <sniper/cache/Cache.h>
#include <sniper/std/memory.h>
#include <sniper/std/string.h>

namespace sniper::http::server2 {

enum class BufferState
{
    Full,
    Again,
    Error
};

struct Buffer;
using BufferCache = cache::STDCache<Buffer>;

struct Buffer final : public intrusive_cache_unsafe_ref_counter<Buffer, BufferCache>
{
    ~Buffer();
    void clear() noexcept;
    [[nodiscard]] bool init(size_t size) noexcept;
    [[nodiscard]] string_view all_data() const noexcept;
    [[nodiscard]] string_view curr_data() const noexcept;
    [[nodiscard]] bool full() const noexcept;
    [[nodiscard]] BufferState read(int fd) noexcept;

    uint32_t size = 0;
    uint32_t processed = 0;
    uint32_t used = 0;
    char* _d = nullptr;
};

using BufferPtr = intrusive_ptr<Buffer>;

inline BufferPtr make_buffer() noexcept
{
    return BufferCache::get_intrusive();
}

inline BufferPtr make_buffer(uint32_t size) noexcept
{
    if (auto dst = BufferCache::get_intrusive(); dst->init(size))
        return dst;

    return nullptr;
}

inline BufferPtr make_buffer(uint32_t size, string_view src) noexcept
{
    if (src.size() > size)
        size = src.size();

    if (auto dst = BufferCache::get_intrusive(); dst->init(size)) {
        if (!src.empty()) {
            memcpy(dst->_d, src.data(), src.size());
            dst->used = src.size();
        }
        return dst;
    }

    return nullptr;
}

} // namespace sniper::http::server2
