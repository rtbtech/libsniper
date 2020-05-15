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
#include <sniper/std/boost_vector.h>
#include <sniper/std/memory.h>
#include <sniper/std/string.h>
#include <sniper/std/tuple.h>
#include <sniper/std/vector.h>
#include <sys/uio.h>

namespace sniper::http::server {

struct Connection;
struct Response;
using ResponseCache = cache::STDCache<Response>;
using Chunk = tuple<string_view, cache::String::unique>;

struct Response final : public intrusive_cache_unsafe_ref_counter<Response, ResponseCache>
{
    void clear() noexcept;

    bool keep_alive = false;
    ResponseStatus code = ResponseStatus::NOT_IMPLEMENTED;

    // "Content-Type: text/html; charset=utf-8\r\n";
    void add_header_copy(string_view header);
    void add_header_nocopy(string_view header);
    void add_header(cache::String::unique&& header_ptr);

    void set_data_copy(string_view data) noexcept;
    void set_data_nocopy(string_view data) noexcept;
    void set_data(cache::String::unique&& data_ptr) noexcept;

private:
    friend struct Connection;
    friend intrusive_ptr<Response> make_response(int minor_version, bool keep_alive) noexcept;

    void fill_iov() noexcept;
    [[nodiscard]] uint32_t add_iov(iovec* data, size_t max_size) noexcept;
    [[nodiscard]] bool process_iov(ssize_t& size) noexcept;
    [[nodiscard]] bool set_ready() noexcept;

    bool _ready = false;
    int _minor_version = 0;

    string_view _first_header;
    small_vector<Chunk, 32> _headers;
    Chunk _data{""sv, cache::StringCache::get_unique_empty()};

    small_vector<iovec, 35> _iov;
    uint32_t _processed = 0;
    uint32_t _total_size = 0;
    local_ptr<string> _date;
};

[[nodiscard]] inline intrusive_ptr<Response> make_response(int minor_version, bool keep_alive) noexcept
{
    if (auto resp = ResponseCache::get_intrusive(); resp) {
        resp->_minor_version = minor_version;
        resp->keep_alive = keep_alive;
        return resp;
    }

    return nullptr;
}

} // namespace sniper::http::server
