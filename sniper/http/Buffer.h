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

namespace sniper::http {

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

    // size of buffer, set by reserve
    [[nodiscard]] size_t capacity() const noexcept;
    // size of data, set by read or fill
    [[nodiscard]] size_t size() const noexcept;
    //    [[nodiscard]] size_t exceed() const noexcept;
    [[nodiscard]] string_view tail(size_t processed) const noexcept;

    [[nodiscard]] BufferState read(int fd, uint32_t max_size = 0) noexcept;
    [[nodiscard]] bool fill(string_view data) noexcept;

private:
    friend intrusive_ptr<Buffer> make_buffer(size_t size, string_view src) noexcept;
    friend intrusive_ptr<Buffer> make_buffer(const intrusive_ptr<Buffer>& buf, size_t processed) noexcept;

    uint32_t _capacity = 0;
    uint32_t _size = 0;
    cache::String::unique _data = cache::String::get_unique_empty();
};

[[nodiscard]] intrusive_ptr<Buffer> make_buffer(size_t size, string_view src = {}) noexcept;
[[nodiscard]] intrusive_ptr<Buffer> renew_buffer(const intrusive_ptr<Buffer>& buf, size_t threshold, uint32_t max_size,
                                                 size_t& processed) noexcept;

} // namespace sniper::http
