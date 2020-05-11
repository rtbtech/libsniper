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

#include <sniper/cache/Cache.h>
#include <sniper/pico/Request.h>
#include <sniper/std/memory.h>

namespace sniper::http {

struct Buffer;

} // namespace sniper::http

namespace sniper::http::server {

struct Request;
using RequestCache = cache::STDCache<Request>;

struct Request final : public intrusive_cache_unsafe_ref_counter<Request, RequestCache>
{
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
    friend intrusive_ptr<Request> make_request(intrusive_ptr<Buffer> buf, cache::STDCache<pico::Request>::unique&& pico,
                                               string_view body) noexcept;

    string_view _body;
    intrusive_ptr<Buffer> _buf;
    cache::STDCache<pico::Request>::unique _pico = cache::STDCache<pico::Request>::get_unique_empty();

    static_vector<pair_sv, pico::MAX_HEADERS> _empty_headers;
    small_vector<pair_sv, pico::MAX_PARAMS> _empty_params;
};

[[nodiscard]] intrusive_ptr<Request>
make_request(intrusive_ptr<Buffer> buf, cache::STDCache<pico::Request>::unique&& pico, string_view body = {}) noexcept;

} // namespace sniper::http::server
