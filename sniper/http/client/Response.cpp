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
#include "sniper/http/client/Config.h"
#include "Response.h"

namespace sniper::http::client {

void Response::clear() noexcept
{
    _pico_resp.clear();
    _read = 0;
    _processed = 0;
    _total = 0;
    debug_close_reason.clear();
    _buf_body.reset();
}

RecvStatus Response::recv(const MessageConfig& config, int fd) noexcept
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

bool Response::keep_alive() const noexcept
{
    return _pico_resp.keep_alive;
}

bool Response::init(const MessageConfig& config, string_view tail)
{
    if (_buf_header.size() < config.header_max_size)
        _buf_header.resize(config.header_max_size);

    if (_buf_header.size() < tail.size()) {
        log_err("[Client:Response] request tail size {} > max header size {}", tail.size(), config.header_max_size);
        return false;
    }

    // copy tail to buf
    if (!tail.empty()) {
        memcpy(_buf_header.data(), tail.data(), tail.size());
        _read = tail.size();
    }

    return true;
}

string_view Response::tail() const noexcept
{
    if (_read > _total)
        return string_view(_buf_header).substr(_total, _read - _total);

    return {};
}

string_view Response::data() const noexcept
{
    if (_buf_body)
        return *_buf_body;
    else if (_pico_resp.content_length)
        return string_view(_buf_header).substr(_pico_resp.header_size, _pico_resp.content_length);

    return {};
}

size_t Response::content_length() const noexcept
{
    return _pico_resp.content_length;
}

const static_vector<pair_sv, pico::MAX_HEADERS>& Response::headers() const noexcept
{
    return _pico_resp.headers;
}

ssize_t Response::recv_int(const MessageConfig& config, int fd) noexcept
{
    if (!_total && _read < _buf_header.size())
        return read(fd, _buf_header.data() + _read, std::min(config.header_chunk_size, _buf_header.size() - _read));

    if (_total && _read < _total) {
        if (_total <= _buf_header.size())
            return read(fd, _buf_header.data() + _read, _total - _read);
        else
            return read(fd, _buf_body->data() + _read - _pico_resp.header_size, _total - _read);
    }

    log_err("[Client:Response] request > allocated memory");
    log_err("[Client:Response] read={}, header_size={}, body_size={}", _read, _pico_resp.header_size,
            _pico_resp.content_length);
    errno = ENOMEM;
    return -1;
}

RecvStatus Response::parse(const MessageConfig& config) noexcept
{
    _processed = _read;

    if (!_total) {
        switch (_pico_resp.parse(_buf_header.data(), _read)) {
            case pico::ParseResult::Err:
                log_err("[Client:Response] request parse error");
                return RecvStatus::Err;
            case pico::ParseResult::Partial:
                return RecvStatus::Partial;
            case pico::ParseResult::Complete:
                _total = _pico_resp.header_size + _pico_resp.content_length;

                if (_pico_resp.content_length && _total > _buf_header.size()) {
                    if (_pico_resp.content_length <= config.body_max_size) {
                        _buf_body = cache::StringCache::get_unique(_pico_resp.content_length);
                        _buf_body->resize(_pico_resp.content_length);

                        // copy tail from buf_header to buf_body
                        if (_read > _pico_resp.header_size)
                            memcpy(_buf_body->data(), _buf_header.data() + _pico_resp.header_size,
                                   _read - _pico_resp.header_size);
                    }
                    else {
                        log_err("[Client:Response] request body size {} > max body size {}", _pico_resp.content_length,
                                config.body_max_size);
                        return RecvStatus::Err;
                    }
                }

                break;
        }
    }

    if (_read < _total)
        return RecvStatus::Partial;

    return RecvStatus::Complete;
}

int Response::code() const noexcept
{
    return _pico_resp.status;
}

} // namespace sniper::http::client
