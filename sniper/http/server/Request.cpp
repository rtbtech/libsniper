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

#include <cstring>
#include <sniper/log/log.h>
#include "sniper/http/server/Config.h"
#include "Request.h"

namespace sniper::http::server {

void Request::clear() noexcept
{
    _pico_req.clear();
    _read = 0;
    _processed = 0;
    _total_size = 0;
    _tail = {};
}

bool Request::init(const MessageConfig& config, string_view tail)
{
    if (_buf.size() < config.usual_size)
        _buf.resize(config.usual_size);

    // copy tail to buf
    if (!tail.empty()) {
        if (_buf.size() < tail.size())
            return false;

        memcpy(_buf.data(), tail.data(), tail.size());
        _read = tail.size();
    }

    return true;
}

RecvStatus Request::parse(const MessageConfig& config) noexcept
{
    _processed = _read;

    if (!_total_size) {
        switch (_pico_req.parse(_buf.data(), _read)) {
            case pico::ParseResult::Err:
                log_err("[Server:Request] request parse error");
                return RecvStatus::Err;
            case pico::ParseResult::Partial:
                return RecvStatus::Partial;
            case pico::ParseResult::Complete:
                _total_size = _pico_req.header_size + _pico_req.content_length;

                if (_total_size > _buf.size()) {
                    if (_total_size <= config.max_size) {
                        _buf.resize(_total_size + 1);
                    }
                    else {
                        log_err("[Server:Request] request size {} > max size {}", _total_size, config.max_size);
                        return RecvStatus::Err;
                    }
                }

                // tail
                if (_read > _total_size)
                    _tail = string_view(_buf).substr(_total_size, _read - _total_size);


                break;
        }
    }

    if (_read < _total_size)
        return RecvStatus::Partial;

    return RecvStatus::Complete;
}

RecvStatus Request::recv(const MessageConfig& config, int fd) noexcept
{
    if (_read > _processed)
        return parse(config);


    if (ssize_t count = recv_int(config, fd); count > 0) {
        _read += count;
        return parse(config);
    }
    else if (count < 0 && errno == EINTR) {
        return RecvStatus::Partial;
    }
    else if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return RecvStatus::Async;
    }
    else if (count < 0 && errno == ENOMEM) {
        return RecvStatus::Err;
    }
    else {
        return RecvStatus::Err;
    }
}

ssize_t Request::recv_int(const MessageConfig& config, int fd) noexcept
{
    if (!_total_size && _read < _buf.size())
        return read(fd, _buf.data() + _read, std::min(config.header_chunk_size, _buf.size() - _read));

    if (_read < _total_size && _total_size <= _buf.size())
        return read(fd, _buf.data() + _read, _total_size - _read);

    log_err("[Server:Request] try to read over allocated memory");
    log_err("[Server:Request] read={}, total_size={}, buf.size={}", _read, _total_size, _buf.size());
    errno = ENOMEM;
    return -1;
}

string_view Request::tail() const noexcept
{
    return _tail;
}

string_view Request::data() const noexcept
{
    if (_pico_req.content_length)
        return string_view(_buf).substr(_pico_req.header_size, _pico_req.content_length);

    return {};
}

size_t Request::content_length() const noexcept
{
    return _pico_req.content_length;
}

bool Request::keep_alive() const noexcept
{
    return _pico_req.keep_alive;
}

int Request::minor_version() const noexcept
{
    return _pico_req.minor_version;
}

string_view Request::method() const noexcept
{
    return _pico_req.method;
}

string_view Request::path() const noexcept
{
    return _pico_req.path;
}

string_view Request::qs() const noexcept
{
    return _pico_req.qs;
}

string_view Request::fragment() const noexcept
{
    return _pico_req.fragment;
}

const static_vector<pair_sv, pico::MAX_HEADERS>& Request::headers() const noexcept
{
    return _pico_req.headers;
}

const small_vector<pair_sv, pico::MAX_PARAMS>& Request::params() const noexcept
{
    return _pico_req.params;
}

} // namespace sniper::http::server
