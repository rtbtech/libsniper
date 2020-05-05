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
    void clear() noexcept;
    [[nodiscard]] bool reserve(size_t size) noexcept;
    [[nodiscard]] BufferState read(int fd, bool processed) noexcept;

    // size of buffer, set by reserve
    [[nodiscard]] size_t capacity() const noexcept;
    // size of data, set by read or fill
    [[nodiscard]] size_t size() const noexcept;

private:
    friend intrusive_ptr<Buffer> make_buffer(uint32_t size, string_view src) noexcept;
    friend inline string_view tail(const Buffer& buf, size_t processed) noexcept;
    friend bool fill(string_view data, Buffer& buf) noexcept;

    uint32_t _capacity = 0;
    uint32_t _size = 0;
    cache::String::unique _data = cache::String::get_unique_empty();
};

inline intrusive_ptr<Buffer> make_buffer(uint32_t size) noexcept
{
    if (auto dst = BufferCache::get_intrusive(); dst->reserve(size))
        return dst;

    return nullptr;
}

inline intrusive_ptr<Buffer> make_buffer(uint32_t size, string_view src) noexcept
{
    if (src.size() > size)
        size = src.size();

    if (auto dst = BufferCache::get_intrusive(); dst->reserve(size)) {
        if (!src.empty()) {
            memcpy(dst->_data->data(), src.data(), src.size());
            dst->_size = src.size();
        }
        return dst;
    }

    return nullptr;
}

inline string_view tail(const Buffer& buf, size_t processed) noexcept
{
    if (buf._size > processed)
        return string_view(buf._data->data() + processed, buf._size - processed);

    return {};
}

inline bool fill(string_view data, Buffer& buf) noexcept
{
    if (!data.empty() && data.size() <= (buf.capacity() - buf.size())) {
        memcpy(buf._data->data() + buf.size(), data.data(), data.size());
        buf._size += data.size();
        return true;
    }

    return false;
}

} // namespace sniper::http::server2
