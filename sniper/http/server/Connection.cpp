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
#include <sniper/log/log.h>
#include <sniper/std/check.h>
#include <sniper/std/string.h>
#include <sys/uio.h>
#include "Connection.h"
#include "Config.h"
#include "Pool.h"
#include "Request.h"
#include "Response.h"

namespace sniper::http::server {

Connection::Connection(event::loop_ptr loop, intrusive_ptr<Pool> pool, intrusive_ptr<Config> config) :
    _loop(std::move(loop)), _pool(std::move(pool)), _config(std::move(config))
{
    check(_loop, "Loop is nullptr");
    check(_pool, "Pool is nullptr");
    check(_pool->_cb, "Callback not set");
    check(_config, "Config is nullptr");

    if (_config->add_server_header && !_config->server_name.empty())
        _server_name_header = fmt::format("Server: {}\r\n", _config->server_name);


    _out.set_capacity(128);
    _user.reserve(128);

    _w_read.set(*_loop);
    _w_write.set(*_loop);
    _w_close.set(*_loop);
    _w_user.set(*_loop);
    _w_keep_alive_timeout.set(*_loop);

    _w_read.set<Connection, &Connection::cb_read>(this);
    _w_write.set<Connection, &Connection::cb_write>(this);
    _w_close.set<Connection, &Connection::cb_close>(this);
    _w_user.set<Connection, &Connection::cb_user>(this);
    _w_keep_alive_timeout.set<Connection, &Connection::cb_keep_alive_timeout>(this);
}

net::Peer Connection::peer() const noexcept
{
    return _peer;
}

void Connection::set(net::Peer peer, int fd) noexcept
{
    _peer = peer;
    _fd = fd;
    _closed = false;

    if (_config->keep_alive_timeout > 0ms) {
        double to_d = (double)duration_cast<milliseconds>(_config->keep_alive_timeout).count() / 1000.0;
        _w_keep_alive_timeout.start(to_d, to_d);
    }

    _buf = make_buffer(_config->buffer_size);
    _pico = cache::STDCache<pico::Request>::get_unique();

    if (!_buf || !_pico) {
        close();
        return;
    }

    _w_read.start(fd, ev::READ);
    _w_write.set(fd, ev::WRITE);
    _w_read.feed_event(0);
}

// internal call only from async callbacks
void Connection::close() noexcept
{
    _w_read.stop();
    _w_write.stop();
    _w_close.stop();
    _w_user.stop();
    _w_keep_alive_timeout.stop();

    ::close(_fd);
    _closed = true;
    _fd = -1;
    _closed = true;
    _processed = 0;

    _out.clear();
    _user.clear();
    _buf.reset();
    _pico.reset();

    if (_pool)
        _pool->disconnect(this);
}

// call from user
void Connection::disconnect() noexcept
{
    if (!_closed && !_w_close.is_active()) {
        _w_close.start();
        _w_close.feed_event(0);
    }
}

void Connection::cb_close(ev::prepare& w, int revents) noexcept
{
    if (!_closed)
        close();
}

void Connection::cb_read(ev::io& w, int revents) noexcept
{
    if (_closed)
        return;

    while (true) {
        if (auto state = _buf->read(_fd); state != BufferState::Error) { // BufferState::Again or BufferState::Full
            if (!parse_buffer(*_config, _buf, _processed, _user, _out, _pico)) {
                close();
                return;
            }

            if (_buf = renew_buffer(_buf, _config->buffer_renew_threshold, _processed); !_buf) {
                close();
                return;
            }

            // relaunch timeout timer
            if (_w_keep_alive_timeout.is_active())
                _w_keep_alive_timeout.again();

            if (state == BufferState::Again)
                break;

            continue;
        }
        else { // BufferState::Error
            close();
            return;
        }
    }

    if (!_closed && !_user.empty() && !_w_user.is_active()) {
        _w_user.start();
        _w_user.feed_event(0);
    }
}

WriteState Connection::cb_writev_int(ev::io& w) noexcept
{
    while (!_out.empty() && _out.front()->_ready) {
        std::array<iovec, 1024> iov{};
        uint32_t iov_count = 0;

        for (auto it = _out.begin(); iov_count < iov.size() && it != _out.end() && (*it)->_ready; ++it) {
            if (auto count = (*it)->add_iov(iov.data() + iov_count, iov.size() - iov_count); count)
                iov_count += count;
            else
                break;
        }

        if (!iov_count)
            return WriteState::Stop;

        if (ssize_t size = writev(_fd, iov.data(), (int)iov_count); size > 0) {
            for (auto it = _out.begin(); size && it != _out.end();) {
                if (!(*it)->process_iov(size))
                    break;

                if (!(*it)->keep_alive) {
                    close();
                    return WriteState::Error;
                }

                ++it;
                _out.pop_front();
            }
        }
        else if (size < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return WriteState::Again;
        }
        else if (size < 0 && errno == EINTR) {
            continue;
        }
        else {
            close();
            return WriteState::Error;
        }
    }

    return WriteState::Stop;
}

void Connection::cb_write(ev::io& w, int revents) noexcept
{
    if (_closed)
        return;

    if (cb_writev_int(w) == WriteState::Stop)
        w.stop();
}

void Connection::cb_user(ev::prepare& w, int revents) noexcept
{
    if (_closed || _user.empty())
        return;

    auto tmp = cache::Vector<tuple<intrusive_ptr<Request>, intrusive_ptr<Response>>>::get_unique(_user.capacity());
    if (!tmp)
        return;

    tmp->swap(_user);
    auto pool = _pool;

    for (auto& [req, resp] : *tmp) {
        try {
            pool->_cb(intrusive_ptr(this), req, resp);
        }
        catch (std::exception& e) {
            log_err("[Connection] Exception in user callback: {}", e.what());
        }
        catch (...) {
            log_err("[Connection] Exception in user callback");
        }

        if (_closed || _w_close.is_active())
            return;
    }

    if (!_w_write.is_active()) {
        cb_writev_int(_w_write);

        if (!_out.empty() && _out.front()->_ready) {
            _w_write.start();
            _w_write.feed_event(0);
        }
    }

    if (_user.empty())
        w.stop();
}

void Connection::cb_keep_alive_timeout(ev::timer& w, int revents) noexcept
{
    close();
}

void Connection::send(const intrusive_ptr<Response>& resp) noexcept
{
    if (resp && !_closed) {
        if (_config->add_server_header)
            resp->add_header_nocopy(_server_name_header);

        if (_config->add_date_header)
            resp->_date = _pool->date;

        if (!resp->set_ready()) {
            if (!_w_close.is_active()) {
                _w_close.start();
                _w_close.feed_event(0);
            }
            return;
        }

        if (!_w_write.is_active() && !_out.empty() && _out.front() == resp) {
            _w_write.start();
            _w_write.feed_event(0);
        }
    }
}

// call from pool close
void Connection::detach() noexcept
{
    _pool.reset();

    if (!_closed)
        close();
}

bool parse_buffer(const Config& config, const intrusive_ptr<Buffer>& buf, size_t& processed,
                  vector<tuple<intrusive_ptr<Request>, intrusive_ptr<Response>>>& user,
                  boost::circular_buffer<intrusive_ptr<Response>>& out, cache::STDCache<pico::Request>::unique& pico) noexcept
{
    auto data = buf->tail(processed);

    while (!data.empty()) {
        if (auto res = pico->parse(data, config.request_max_size, config.normalize, config.normalize_other);
            res == pico::ParseResult::Complete) { // request ready

            string_view body;
            if (pico->content_length) {
                body = data.substr(pico->header_size, pico->content_length);
                data.remove_prefix(pico->header_size + pico->content_length);
                processed += pico->header_size + pico->content_length;
            }
            else {
                data.remove_prefix(pico->header_size);
                processed += pico->header_size;
            }

            auto req = make_request(buf, std::move(pico), body);
            auto resp = make_response(req->minor_version(), req->keep_alive());

            if (!req || !resp)
                return false;

            if (out.full())
                out.set_capacity(2 * out.capacity());

            user.emplace_back(req, resp);
            out.push_back(std::move(resp));

            if (pico = cache::STDCache<pico::Request>::get_unique(); !pico)
                return false;
        }
        else {
            return res != pico::ParseResult::Err;
        }
    }

    return true;
}

} // namespace sniper::http::server
