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

#include <fmt/format.h>
#include "Response.h"
#include "Connection.h"

namespace sniper::http::server2 {

namespace {

constexpr string_view connection_close = "Connection: close\r\n";
constexpr string_view connection_keep_alive = "Connection: keep-alive\r\n";
constexpr string_view content_length = "Content-Length: ";
constexpr string_view content_length_0 = "Content-Length: 0\r\n";

inline void fill(string_view str, iovec& i) noexcept
{
    i.iov_len = str.size();
    i.iov_base = const_cast<char*>(str.data());
}

} // namespace

void Response::clear() noexcept
{
    code = ResponseStatus::NOT_IMPLEMENTED;

    _ready = false;
    _keep_alive = false;
    _minor_version = 0;

    _first_header = {};
    _headers.clear();
    _data = {""sv, cache::StringCache::get_unique_empty()};
    _iov.clear();
    _processed = 0;
}

void Response::add_header_copy(string_view header)
{
    if (header.empty())
        return;

    if (auto str = cache::StringCache::get_unique(header.size()); str) {
        str->assign(header);
        _headers.emplace_back(*str, std::move(str));
    }
}

void Response::add_header_nocopy(string_view header)
{
    if (header.empty())
        return;

    _headers.emplace_back(header, cache::StringCache::get_unique_empty());
}

void Response::add_header(cache::String::unique&& header_ptr)
{
    if (!header_ptr)
        return;

    _headers.emplace_back(*header_ptr, std::move(header_ptr));
}

void Response::set_data_copy(string_view data)
{
    if (data.empty())
        return;

    if (auto str = cache::StringCache::get_unique(data.size()); str) {
        str->assign(data);
        _data = make_tuple(*str, std::move(str));
    }
}

void Response::set_data_nocopy(string_view data) noexcept
{
    std::get<string_view>(_data) = data;
}

void Response::set_data(cache::String::unique&& data_ptr) noexcept
{
    if (!data_ptr)
        return;

    _data = make_tuple(*data_ptr, std::move(data_ptr));
}

bool Response::set_ready() noexcept
{
    if (_ready)
        return true;

    // first header
    _first_header = http_status(_minor_version, code);

    // connection header
    if (_minor_version == 0) {
        if (_keep_alive)
            _headers.emplace_back(connection_keep_alive, cache::StringCache::get_unique_empty());
        else
            _headers.emplace_back(connection_close, cache::StringCache::get_unique_empty());
    }
    else if (!_keep_alive) {
        _headers.emplace_back(connection_close, cache::StringCache::get_unique_empty());
    }

    // last header - content length
    if (!std::get<string_view>(_data).empty()) {
        fmt::format_int len(std::get<string_view>(_data).size());

        auto str = cache::StringCache::get_unique(content_length.size() + len.size() + 4); // + \r\n\r\n
        if (!str)
            return false;

        str->append(content_length);
        str->append(len.data(), len.size());
        str->append("\r\n\r\n");

        _headers.emplace_back(*str, std::move(str));
    }
    else {
        _headers.emplace_back(content_length_0, cache::StringCache::get_unique_empty());
    }

    fill_iov();

    _ready = true;
    return true;
}

void Response::fill_iov() noexcept
{
    fill(_first_header, _iov.emplace_back());

    for (auto& h : _headers)
        if (!std::get<string_view>(h).empty())
            fill(std::get<string_view>(h), _iov.emplace_back());

    if (!std::get<string_view>(_data).empty())
        fill(std::get<string_view>(_data), _iov.emplace_back());
}

} // namespace sniper::http::server2
