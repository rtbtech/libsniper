// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

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

#include <fmt/format.h>
#include "Request.h"
#include "Connection.h"

namespace sniper::http::client {

namespace {

inline void fill_iov(string_view str, iovec& i)
{
    i.iov_len = str.size();
    i.iov_base = const_cast<char*>(str.data());
}

inline size_t update_view(size_t count, string_view& str)
{
    auto num = std::min(str.size(), static_cast<size_t>(count));
    str.remove_prefix(num);
    return count - num;
}

template<typename T>
void clear_tuple(T& t)
{
    get<string>(t).clear();
    get<0>(t) = {};
    get<1>(t) = {};
}

template<typename T>
void reinit_view(T& t)
{
    get<0>(t) = get<1>(t);
}

[[nodiscard]] string_view method_to_str(Method method)
{
    switch (method) {
        case Method::Get:
            return "GET";
        case Method::Post:
            return "POST";
        case Method::Put:
            return "PUT";
        case Method::Head:
            return "HEAD";
    }

    return "GET"; // GCC warning
}

void fill_first_headers(bool full_url, Method method, const net::Url& url, bool keep_alive, size_t data_size,
                        string& out)
{
    out.clear();

    // POST
    out.append(method_to_str(method));

    // /path
    out.push_back(' ');
    out.append(url.path());

    // ?a=b
    if (!url.query().empty()) {
        out.push_back('?');
        out.append(url.query());
    }

    // #fragment
    if (!url.fragment().empty()) {
        out.push_back('#');
        out.append(url.fragment());
    }

    // HTTP/1.1
    out.append(" HTTP/1.1\r\n");

    // Host: blabla.ru
    out.append("Host: ");
    out.append(url.host());
    out.append("\r\n");

    if (!keep_alive)
        out.append("Connection: close\r\n");

    if (method == Method::Post || method == Method::Put) {
        out.append("Content-Length: ");
        fmt::format_int len(data_size);
        out.append(len.data(), len.size());
        out.append("\r\n");
    }
}

} // namespace

Request::Request()
{
    _iov.reserve(30);
    _headers.reserve(30);
    get<string>(_first_headers).reserve(256);

    get<1>(_last_headers) = "\r\n";
}

void Request::clear()
{
    method = Method::Get;
    keep_alive = true;
    url.clear();

    user_data.reset();
    user_int = std::nullopt;
    user_string.clear();

    close_reason.clear();

    wg.reset();

    _iov.clear();
    clear_tuple(_first_headers);
    _headers.clear();
    _data = {"", "", cache::StringCache::get_unique_empty()};

    _sent = 0;
    _generation = 0;
    _ts_start = {};
    _ts_end = _ts_start;
}

string_view Request::data() const noexcept
{
    return get<1>(_data);
}

size_t Request::generation() const noexcept
{
    return _generation;
}

void Request::add_header_copy(string_view header)
{
    if (header.empty())
        return;

    auto str = cache::StringCache::get_unique(header.size());
    str->assign(header);
    _headers.emplace_back(*str, *str, std::move(str));
}

void Request::add_header_nocopy(string_view header)
{
    if (header.empty())
        return;

    _headers.emplace_back(header, header, cache::StringCache::get_unique_empty());
}

void Request::add_header(cache::StringCache::unique&& header_ptr)
{
    if (!header_ptr || header_ptr->empty())
        return;

    _headers.emplace_back(*header_ptr, *header_ptr, std::move(header_ptr));
}

void Request::set_data_nocopy(string_view data)
{
    std::get<1>(_data) = data;
}

void Request::set_data(cache::StringCache::unique&& data_ptr)
{
    if (!data_ptr || data_ptr->empty())
        return;

    std::get<cache::StringCache::unique>(_data) = std::move(data_ptr);
    std::get<1>(_data) = *std::get<cache::StringCache::unique>(_data);
}

void Request::set_data_copy(string_view data)
{
    if (data.empty())
        return;

    std::get<cache::StringCache::unique>(_data) = cache::StringCache::get_unique(data.size());
    std::get<cache::StringCache::unique>(_data)->assign(data);
    std::get<1>(_data) = *std::get<cache::StringCache::unique>(_data);
}

SendStatus Request::send(int fd) noexcept
{
    if (fd < 0)
        return SendStatus::Err;

    while (true) {
        _iov.clear();

        // prepare iovec
        if (!std::get<0>(_first_headers).empty())
            fill_iov(std::get<0>(_first_headers), _iov.emplace_back());

        for (auto& h : _headers)
            if (!std::get<0>(h).empty())
                fill_iov(std::get<0>(h), _iov.emplace_back());

        if (!std::get<0>(_last_headers).empty())
            fill_iov(std::get<0>(_last_headers), _iov.emplace_back());

        if (!std::get<0>(_data).empty())
            fill_iov(std::get<0>(_data), _iov.emplace_back());

        if (_iov.empty())
            return SendStatus::Complete;

        // read
        if (ssize_t count = writev(fd, _iov.data(), _iov.size()); count > 0) {
            if (!_sent)
                _ts_start = steady_clock::now();

            _sent += count;

            if (!std::get<0>(_first_headers).empty())
                count = update_view(count, std::get<0>(_first_headers));

            for (auto& h : _headers)
                if (count && !std::get<0>(h).empty())
                    count = update_view(count, std::get<0>(h));
                else if (!count)
                    break;

            if (!std::get<0>(_last_headers).empty())
                count = update_view(count, std::get<0>(_last_headers));

            if (count && !std::get<0>(_data).empty())
                update_view(count, std::get<0>(_data));

            continue;
        }
        else if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return SendStatus::Async;
        }
        else if (count < 0 && errno == EINTR) {
            continue;
        }
        else {
            return SendStatus::Err;
        }
    }
}

void Request::set_ready(bool full_url) noexcept
{
    _sent = 0;
    close_reason.clear();

    fill_first_headers(full_url, method, url, keep_alive, get<1>(_data).size(), get<string>(_first_headers));
    get<1>(_first_headers) = get<string>(_first_headers);
    reinit_view(_first_headers);

    for (auto& h : _headers)
        reinit_view(h);

    reinit_view(_last_headers);

    reinit_view(_data);

    _generation++;
}

milliseconds Request::latency() const noexcept
{
    return duration_cast<milliseconds>(_ts_end - _ts_start);
}

} // namespace sniper::http::client
