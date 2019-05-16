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
    _total_size = 0;
    debug_close_reason.clear();
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

string_view Response::tail() const noexcept
{
    if (_read > _total_size)
        return string_view(_buf).substr(_total_size, _read - _total_size);

    return {};
}

string_view Response::data() const noexcept
{
    if (_pico_resp.content_length)
        return string_view(_buf).substr(_pico_resp.header_size, _pico_resp.content_length);

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
    if (!_total_size && _read < _buf.size())
        return read(fd, _buf.data() + _read, std::min(config.header_chunk_size, _buf.size() - _read));

    if (_read < _total_size && _total_size <= _buf.size())
        return read(fd, _buf.data() + _read, _total_size - _read);

    log_err("[Client:Response] try to read over allocated memory");
    log_err("[Client:Response] read={}, total_size={}, buf.size={}", _read, _total_size, _buf.size());
    errno = ENOMEM;
    return -1;
}

RecvStatus Response::parse(const MessageConfig& config) noexcept
{
    _processed = _read;

    if (!_total_size) {
        switch (_pico_resp.parse(_buf.data(), _read)) {
            case pico::ParseResult::Err:
                log_err("[Client:Response] request parse error");
                return RecvStatus::Err;
            case pico::ParseResult::Partial:
                return RecvStatus::Partial;
            case pico::ParseResult::Complete:
                _total_size = _pico_resp.header_size + _pico_resp.content_length;

                if (_total_size > _buf.size()) {
                    if (_total_size <= config.max_size) {
                        _buf.resize(_total_size + 1);
                        // TODO: should reinit string_view headers after this
                    }
                    else {
                        log_err("[Client:Response] request size {} > max size {}", _total_size, config.max_size);
                        return RecvStatus::Err;
                    }
                }

                break;
        }
    }

    if (_read < _total_size)
        return RecvStatus::Partial;

    return RecvStatus::Complete;
}

int Response::code() const noexcept
{
    return _pico_resp.status;
}

} // namespace sniper::http::client
