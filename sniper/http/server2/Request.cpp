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

#include <sniper/http/Buffer.h>
#include "Request.h"

namespace sniper::http::server2 {

void Request::clear() noexcept
{
    _head.reset();
    _pico.reset();
}

string_view Request::data() const noexcept
{
    return std::string_view();
}

size_t Request::content_length() const noexcept
{
    return 0;
}

bool Request::keep_alive() const noexcept
{
    return false;
}

int Request::minor_version() const noexcept
{
    return 0;
}

string_view Request::method() const noexcept
{
    return std::string_view();
}

string_view Request::path() const noexcept
{
    return std::string_view();
}

string_view Request::qs() const noexcept
{
    return std::string_view();
}

string_view Request::fragment() const noexcept
{
    return std::string_view();
}

const static_vector<pair_sv, pico::MAX_HEADERS>& Request::headers() const noexcept
{
    return _empty_headers;
}

const small_vector<pair_sv, pico::MAX_PARAMS>& Request::params() const noexcept
{
    return _empty_params;
}

intrusive_ptr<Request> make_request(intrusive_ptr<Buffer> head, pico::RequestCache::unique&& pico) noexcept
{
    if (auto req = RequestCache::get_intrusive(); req) {
        req->_head = std::move(head);
        req->_pico = std::move(pico);
        return req;
    }

    return nullptr;
}


} // namespace sniper::http::server2
