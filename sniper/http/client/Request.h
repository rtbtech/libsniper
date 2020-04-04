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
#include <sniper/http/wait/Group.h>
#include <sniper/net/Url.h>
#include <sniper/std/any.h>
#include <sniper/std/chrono.h>
#include <sniper/std/functional.h>
#include <sniper/std/memory.h>
#include <sniper/std/optional.h>
#include <sniper/std/string.h>
#include <sniper/std/tuple.h>
#include <sniper/std/vector.h>
#include <sys/uio.h>

namespace sniper::http::client {

enum class Method
{
    Head,
    Get,
    Post,
    Put
};

enum class SendStatus
{
    Err,
    Async,
    Complete
};

class Connection;
class Request;
using RequestCache = cache::STDCache<Request>;

class Request final : public intrusive_cache_unsafe_ref_counter<Request, RequestCache>
{
public:
    Request();
    void clear();

    // "Content-Type: text/html; charset=utf-8\r\n";
    void add_header_copy(string_view header);
    void add_header_nocopy(string_view header);
    void add_header(cache::StringCache::unique&& header_ptr);

    void set_data_copy(string_view data);
    void set_data_nocopy(string_view data);
    void set_data(cache::StringCache::unique&& data_ptr);

    [[nodiscard]] string_view data() const noexcept;
    [[nodiscard]] size_t generation() const noexcept;
    [[nodiscard]] milliseconds latency() const noexcept;

    Method method = Method::Get;
    bool keep_alive = true;
    net::Url url;

    any user_data;
    optional<int64_t> user_int;
    string user_string;

    string close_reason;

    intrusive_ptr<wait::Group> wg;

private:
    friend class Connection;
    [[nodiscard]] SendStatus send(int fd) noexcept;
    void set_ready(bool full_url) noexcept;

    vector<iovec> _iov;
    tuple<string_view, string_view, string> _first_headers;
    tuple<string_view, string_view> _last_headers;
    vector<tuple<string_view, string_view, cache::StringCache::unique>> _headers;
    tuple<string_view, string_view, cache::StringCache::unique> _data{"", "", cache::StringCache::get_unique_empty()};

    size_t _sent = 0;
    size_t _generation = 0;
    steady_clock::time_point _ts_start;
    steady_clock::time_point _ts_end;
};

using RequestPtr = intrusive_ptr<Request>;

inline intrusive_ptr<Request> make_request()
{
    return RequestCache::get_intrusive();
}

} // namespace sniper::http::client
