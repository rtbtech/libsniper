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
#include <sniper/log/log.h>
#include "Response.h"
#include "Connection.h"

namespace sniper::http::server {

namespace {

constexpr string_view connection_close = "Connection: close\r\n";
constexpr string_view connection_keep_alive = "Connection: keep-alive\r\n";
constexpr string_view content_length = "Content-Length: ";
constexpr string_view content_length_0 = "Content-Length: 0\r\n\r\n";

inline uint32_t fill(string_view str, iovec& i) noexcept
{
    log_trace(__PRETTY_FUNCTION__);

    i.iov_len = str.size();
    i.iov_base = const_cast<char*>(str.data());

    return i.iov_len;
}

} // namespace

void Response::clear() noexcept
{
    log_trace(__PRETTY_FUNCTION__);

    code = ResponseStatus::NOT_IMPLEMENTED;

    _ready = false;
    keep_alive = false;
    _minor_version = 0;

    _first_header = {};
    _headers.clear();
    _data = {""sv, cache::StringCache::get_unique_empty()};
    _iov.clear();
    _processed = 0;
    _total_size = 0;
    _date.reset();
}

void Response::add_header_copy(string_view header)
{
    log_trace(__PRETTY_FUNCTION__);

    if (header.empty())
        return;

    if (auto str = cache::StringCache::get_unique(header.size()); str) {
        str->assign(header);

        auto& h = _headers.emplace_back("", std::move(str));
        std::get<0>(h) = *std::get<1>(h);
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
    log_trace(__PRETTY_FUNCTION__);

    if (!header_ptr)
        return;

    auto& h = _headers.emplace_back("", std::move(header_ptr));
    std::get<0>(h) = *std::get<1>(h);
}

void Response::set_data_copy(string_view data) noexcept
{
    log_trace(__PRETTY_FUNCTION__);

    if (data.empty())
        return;

    if (auto str = cache::StringCache::get_unique(data.size()); str) {
        str->assign(data);
        std::get<1>(_data) = std::move(str);
        std::get<0>(_data) = *std::get<1>(_data);
    }
}

void Response::set_data_nocopy(string_view data) noexcept
{
    log_trace(__PRETTY_FUNCTION__);

    std::get<string_view>(_data) = data;
}

void Response::set_data(cache::String::unique&& data_ptr) noexcept
{
    log_trace(__PRETTY_FUNCTION__);

    if (!data_ptr)
        return;

    std::get<1>(_data) = std::move(data_ptr);
    std::get<0>(_data) = *std::get<1>(_data);
}

bool Response::set_ready() noexcept
{
    log_trace(__PRETTY_FUNCTION__);

    if (_ready)
        return true;

    // first header
    _first_header = http_status(_minor_version, code);

    // connection header
    if (_minor_version == 0) {
        if (keep_alive)
            _headers.emplace_back(connection_keep_alive, cache::StringCache::get_unique_empty());
        else
            _headers.emplace_back(connection_close, cache::StringCache::get_unique_empty());
    }
    else if (!keep_alive) {
        _headers.emplace_back(connection_close, cache::StringCache::get_unique_empty());
    }

    // Date
    if (_date)
        _headers.emplace_back(*_date, cache::StringCache::get_unique_empty());

    // last header - content length
    if (!std::get<string_view>(_data).empty()) {
        fmt::format_int len(std::get<string_view>(_data).size());

        auto str = cache::StringCache::get_unique(content_length.size() + len.size() + 4); // + \r\n\r\n
        if (!str)
            return false;

        str->append(content_length);
        str->append(len.data(), len.size());
        str->append("\r\n\r\n");

        auto& h = _headers.emplace_back("", std::move(str));
        std::get<0>(h) = *std::get<1>(h);
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
    log_trace(__PRETTY_FUNCTION__);

    _total_size += fill(_first_header, _iov.emplace_back());

    for (auto& h : _headers)
        if (!std::get<string_view>(h).empty())
            _total_size += fill(std::get<string_view>(h), _iov.emplace_back());

    if (!std::get<string_view>(_data).empty())
        _total_size += fill(std::get<string_view>(_data), _iov.emplace_back());
}

uint32_t Response::add_iov(iovec* data, size_t max_size) noexcept
{
    log_trace("{}, ptr={}, max_count={}, iov count={}, processed={}", __PRETTY_FUNCTION__, (void*)data, max_size,
              _iov.size(), _processed);

    if (auto count = (_iov.size() - _processed); count && count <= max_size) {
        memcpy(data, _iov.data() + _processed, count * sizeof(iovec));
        return count;
    }

    return 0;
}

bool Response::process_iov(ssize_t& size) noexcept
{
    log_trace("{}, size={}, total_size={}, processed={}", __PRETTY_FUNCTION__, size, _total_size, _processed);

    if (_total_size <= size) {
        size -= _total_size;
        return true;
    }

    for (unsigned i = _processed; size && i < _iov.size(); i++) {
        if ((size_t)size >= _iov[i].iov_len) {
            size -= _iov[i].iov_len;
            _total_size -= _iov[i].iov_len;
            _processed++;
        }
        else {
            _iov[i].iov_len -= size;
            _total_size -= size;
            size = 0;
            break;
        }
    }

    return _processed == _iov.size();
}

} // namespace sniper::http::server
