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

namespace sniper::http::server {

void Request::clear() noexcept
{
    _body = {};
    _buf.reset();
    _pico.reset();
}

string_view Request::data() const noexcept
{
    if (_buf)
        return _body;

    return {};
}

size_t Request::content_length() const noexcept
{
    if (_pico)
        return _pico->content_length;

    return {};
}

bool Request::keep_alive() const noexcept
{
    if (_pico)
        return _pico->keep_alive;

    return {};
}

int Request::minor_version() const noexcept
{
    if (_pico)
        return _pico->minor_version;

    return {};
}

string_view Request::method() const noexcept
{
    if (_pico)
        return _pico->method;

    return {};
}

string_view Request::path() const noexcept
{
    if (_pico)
        return _pico->path;

    return {};
}

string_view Request::qs() const noexcept
{
    if (_pico)
        return _pico->qs;

    return {};
}

string_view Request::fragment() const noexcept
{
    if (_pico)
        return _pico->fragment;

    return {};
}

const static_vector<pair_sv, pico::MAX_HEADERS>& Request::headers() const noexcept
{
    if (_pico)
        return _pico->headers;

    return _empty_headers;
}

const small_vector<pair_sv, pico::MAX_PARAMS>& Request::params() const noexcept
{
    if (_pico)
        return _pico->params;

    return _empty_params;
}

intrusive_ptr<Request> make_request(intrusive_ptr<Buffer> buf, pico::RequestCache::unique&& pico,
                                    string_view body) noexcept
{
    if (auto req = RequestCache::get_intrusive(); req) {
        req->_buf = std::move(buf);
        req->_pico = std::move(pico);
        req->_body = body;
        return req;
    }

    return nullptr;
}

} // namespace sniper::http::server
