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
#include <sniper/pico/Response.h>
#include <sniper/std/memory.h>

namespace sniper::http::client {

enum class RecvStatus
{
    Err,
    Partial,
    Async,
    Complete
};

class Connection;
class Response;
struct MessageConfig;
using ResponseCache = cache::STDCache<Response>;

class Response : public intrusive_cache_unsafe_ref_counter<Response, ResponseCache>
{
public:
    void clear() noexcept;

    [[nodiscard]] string_view data() const noexcept;
    [[nodiscard]] size_t content_length() const noexcept;
    [[nodiscard]] bool keep_alive() const noexcept;
    [[nodiscard]] const static_vector<pair_sv, pico::MAX_HEADERS>& headers() const noexcept;

    /* Returns: http code,
     *          -1 if internal error (response timeout, connection timeout, etc)
     */
    [[nodiscard]] int code() const noexcept;

    string debug_close_reason;

private:
    friend class Connection;

    [[nodiscard]] RecvStatus recv(const MessageConfig& config, int fd) noexcept;
    [[nodiscard]] bool init(const MessageConfig& config, string_view tail);
    [[nodiscard]] string_view tail() const noexcept;
    [[nodiscard]] ssize_t recv_int(const MessageConfig& config, int fd) noexcept;
    [[nodiscard]] RecvStatus parse(const MessageConfig& config) noexcept;

    pico::Response _pico_resp;

    string _buf_header;
    cache::StringCache::unique _buf_body = cache::StringCache::get_unique_empty();

    size_t _read = 0;
    size_t _processed = 0;
    size_t _total = 0;
};

using ResponsePtr = intrusive_ptr<Response>;

} // namespace sniper::http::client
