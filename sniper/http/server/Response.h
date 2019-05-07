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

#include <sniper/cache/ArrayCache.h>
#include <sniper/cache/Cache.h>
#include <sniper/http/server/Status.h>
#include <sniper/std/memory.h>
#include <sniper/std/string.h>
#include <sniper/std/tuple.h>
#include <sniper/std/vector.h>
#include <sys/uio.h>

namespace sniper::http::server {

enum class SendStatus
{
    Err,
    Async,
    Complete
};

class Connection;
class Response;
using ResponseCache = cache::STDCache<Response>;

class Response final : public intrusive_cache_unsafe_ref_counter<Response, ResponseCache>
{
public:
    Response();
    void clear() noexcept;

    ResponseStatus code = ResponseStatus::NOT_IMPLEMENTED;

    // "Content-Type: text/html; charset=utf-8\r\n";
    void add_header_copy(string_view header);
    void add_header_nocopy(string_view header);
    void add_header(cache::StringCache::unique&& header_ptr);

    void set_data_copy(string_view data);
    void set_data_nocopy(string_view data);
    void set_data(cache::StringCache::unique&& data_ptr);

    void set_connection_close() noexcept;

private:
    friend class Connection;

    [[nodiscard]] bool is_ready() const noexcept;
    [[nodiscard]] bool keep_alive() const noexcept;

    [[nodiscard]] SendStatus send(int fd) noexcept;
    void set_keep_alive(bool keep_alive) noexcept;
    void set_minor_version(int version) noexcept;
    void set_ready() noexcept;

    bool _ready = false;
    bool _keep_alive = false;
    int _minor_version = 0;

    vector<iovec> _iov;
    vector<tuple<string_view, cache::StringCache::unique>> _chunks;
    tuple<string_view, cache::StringCache::unique> _data{""sv, cache::StringCache::get_unique_empty()};
    size_t _sent = 0;
};

} // namespace sniper::http::server
