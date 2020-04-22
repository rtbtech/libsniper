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
#include <sniper/http/server/Status.h>
#include <sniper/std/memory.h>
#include <sniper/std/string.h>
#include <sniper/std/tuple.h>
#include <sniper/std/vector.h>

namespace sniper::http::server2 {

struct Response;
using ResponseCache = cache::STDCache<Response>;
using Chunk = tuple<string_view, cache::String::unique>;

struct Response final : public intrusive_cache_unsafe_ref_counter<Response, ResponseCache>
{
    void clear() noexcept;

    bool close = false;
    ResponseStatus code = ResponseStatus::NOT_IMPLEMENTED;

    // "Content-Type: text/html; charset=utf-8\r\n";
    void add_header_copy(string_view header);
    void add_header_nocopy(string_view header);
    void add_header(cache::String::unique&& header_ptr);

    void set_data_copy(string_view data);
    void set_data_nocopy(string_view data) noexcept;
    void set_data(cache::String::unique&& data_ptr) noexcept;

    [[nodiscard]] cache::Vector<Chunk>::unique headers() noexcept;
    [[nodiscard]] Chunk data() noexcept;

private:
    cache::Vector<Chunk>::unique _headers = cache::Vector<Chunk>::get_unique_empty();
    Chunk _data{""sv, cache::StringCache::get_unique_empty()};
};

using ResponsePtr = intrusive_ptr<Response>;

inline ResponsePtr make_response() noexcept
{
    return ResponseCache::get_intrusive();
}

} // namespace sniper::http::server2
