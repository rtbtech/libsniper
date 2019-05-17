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
#include <sniper/pico/Request.h>
#include <sniper/std/memory.h>
#include <sniper/std/tuple.h>

namespace sniper::http::server {

enum class RecvStatus
{
    Err,
    Partial,
    Async,
    Complete
};

struct MessageConfig;
class Connection;
class Request;
using RequestCache = cache::STDCache<Request>;

class Request final : public intrusive_cache_unsafe_ref_counter<Request, RequestCache>
{
public:
    void clear() noexcept;

    [[nodiscard]] string_view data() const noexcept;
    [[nodiscard]] size_t content_length() const noexcept;
    [[nodiscard]] bool keep_alive() const noexcept;
    [[nodiscard]] int minor_version() const noexcept;
    [[nodiscard]] string_view method() const noexcept;
    [[nodiscard]] string_view path() const noexcept;
    [[nodiscard]] string_view qs() const noexcept;
    [[nodiscard]] string_view fragment() const noexcept;

    [[nodiscard]] const static_vector<pair_sv, pico::MAX_HEADERS>& headers() const noexcept;
    [[nodiscard]] const small_vector<pair_sv, pico::MAX_PARAMS>& params() const noexcept;

private:
    friend class Connection;

    // tail <= MessageConfig::usual_size
    [[nodiscard]] bool init(const MessageConfig& config, string_view tail);
    [[nodiscard]] string_view tail() const noexcept;
    [[nodiscard]] RecvStatus recv(const MessageConfig& config, int fd) noexcept;
    [[nodiscard]] ssize_t recv_int(const MessageConfig& config, int fd) noexcept;
    [[nodiscard]] RecvStatus parse(const MessageConfig& config) noexcept;

    pico::Request _pico_req;

    string _buf_header;
    cache::StringCache::unique _buf_body = cache::StringCache::get_unique_empty();

    size_t _read = 0;
    size_t _processed = 0;
    size_t _total = 0;
};

} // namespace sniper::http::server
