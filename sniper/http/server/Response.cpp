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
#include <sniper/net/socket.h>
#include "Response.h"

namespace sniper::http::server {

namespace {

constexpr string_view last_crlf = "\r\n";
constexpr string_view last_connection_close = "Connection: close\r\n\r\n";
constexpr string_view last_connection_keep_alive = "Connection: keep-alive\r\n\r\n";
constexpr string_view content_length = "Content-Length: ";
constexpr string_view content_length_0 = "Content-Length: 0\r\n";

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

} // namespace

Response::Response()
{
    _iov.reserve(30);
    _chunks.reserve(30);
    _chunks.emplace_back(""sv, cache::StringCache::get_unique_empty()); // for http header
}

void Response::clear() noexcept
{
    code = ResponseStatus::NOT_IMPLEMENTED;

    _ready = false;
    _keep_alive = false;
    _minor_version = 0;
    _iov.clear();

    _data = {""sv, cache::StringCache::get_unique_empty()};
    _sent = 0;

    _chunks.clear();
    _chunks.emplace_back(""sv, cache::StringCache::get_unique_empty()); // for http header
}

void Response::set_keep_alive(bool keep_alive) noexcept
{
    _keep_alive = keep_alive;
}

void Response::set_minor_version(int version) noexcept
{
    if (version == 0 || version == 1)
        _minor_version = version;
}

bool Response::is_ready() const noexcept
{
    return _ready;
}
bool Response::keep_alive() const noexcept
{
    return _keep_alive;
}

SendStatus Response::send(int fd) noexcept
{
    if (fd < 0)
        return SendStatus::Err;

    while (true) {
        _iov.clear();

        // prepare iovec
        for (auto& h : _chunks)
            if (!std::get<string_view>(h).empty())
                fill_iov(std::get<string_view>(h), _iov.emplace_back());

        if (_iov.empty())
            return SendStatus::Complete;

        // read
        if (ssize_t count = writev(fd, _iov.data(), _iov.size()); count > 0) {
            _sent += count;

            for (auto& h : _chunks)
                if (count && !std::get<string_view>(h).empty())
                    count = update_view(count, std::get<string_view>(h));
                else if (!count)
                    break;

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

void Response::set_ready() noexcept
{
    std::get<string_view>(_chunks[0]) = http_status(_minor_version, code);

    // Content-Length
    if (!std::get<string_view>(_data).empty()) {
        fmt::format_int len(std::get<string_view>(_data).size());

        // Возможный эксепшн. Подумать, как вернуть ошибку
        auto str = cache::StringCache::get_unique(content_length.size() + len.size() + 2); // + \r\n
        str->append(content_length);
        str->append(len.data(), len.size());
        str->append("\r\n");

        _chunks.emplace_back(*str, std::move(str));
    } else {
        _chunks.emplace_back(content_length_0, cache::StringCache::get_unique_empty());
    }


    // last header
    if (!_keep_alive)
        _chunks.emplace_back(last_connection_close, cache::StringCache::get_unique_empty());
    else if (_minor_version == 0)
        _chunks.emplace_back(last_connection_keep_alive, cache::StringCache::get_unique_empty());
    else
        _chunks.emplace_back(last_crlf, cache::StringCache::get_unique_empty());

    if (!std::get<string_view>(_data).empty())
        _chunks.emplace_back(std::move(_data));

    // data
    _iov.reserve(_chunks.size());

    _ready = true;
}

void Response::add_header_copy(string_view header)
{
    if (header.empty())
        return;

    auto str = cache::StringCache::get_unique(header.size());
    str->assign(header);
    _chunks.emplace_back(*str, std::move(str));
}

void Response::add_header_nocopy(string_view header)
{
    if (header.empty())
        return;

    _chunks.emplace_back(header, cache::StringCache::get_unique_empty());
}

void Response::add_header(cache::StringCache::unique&& header_ptr)
{
    if (!header_ptr || header_ptr->empty())
        return;

    _chunks.emplace_back(*header_ptr, std::move(header_ptr));
}

void Response::set_data_copy(string_view data)
{
    if (data.empty())
        return;

    std::get<cache::StringCache::unique>(_data) = cache::StringCache::get_unique(data.size());
    std::get<cache::StringCache::unique>(_data)->assign(data);
    std::get<string_view>(_data) = *std::get<cache::StringCache::unique>(_data);
}

void Response::set_data_nocopy(string_view data)
{
    std::get<string_view>(_data) = data;
}

void Response::set_data(cache::StringCache::unique&& data_ptr)
{
    if (!data_ptr || data_ptr->empty())
        return;

    std::get<cache::StringCache::unique>(_data) = std::move(data_ptr);
    std::get<string_view>(_data) = *std::get<cache::StringCache::unique>(_data);
}

void Response::set_connection_close() noexcept
{
    _keep_alive = false;
}

} // namespace sniper::http::server
