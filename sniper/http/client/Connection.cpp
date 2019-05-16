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

//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <sniper/log/log.h>
#include <sniper/net/socket.h>
#include <sniper/std/array.h>
#include <sniper/std/check.h>
#include "Connection.h"
#include "Request.h"
#include "Response.h"

namespace sniper::http::client {

Connection::Connection(event::loop_ptr loop, ConnectionConfig config, net::Peer peer, bool is_proxy,
                       const function<void(intrusive_ptr<Request>&&, intrusive_ptr<Response>&&)>& cb) :
    _loop(std::move(loop)),
    _config(config), _peer(peer), _is_proxy(is_proxy), _cb(cb)
{
    SPDLOG_TRACE("Conn={:p}, {}", reinterpret_cast<const void*>(this), __PRETTY_FUNCTION__);

    check(_loop, "[Client:Connection] loop is nullptr");

    _w_read.set(*_loop);
    _w_write.set(*_loop);
    _w_response_timeout.set(*_loop);
    _w_connect_timeout.set(*_loop);
    _w_prepare.set(*_loop);

    _w_read.set<Connection, &Connection::cb_read>(this);
    _w_write.set<Connection, &Connection::cb_write>(this);
    _w_response_timeout.set<Connection, &Connection::cb_response_timeout>(this);
    _w_connect_timeout.set<Connection, &Connection::cb_connect_timeout>(this);
    _w_prepare.set<Connection, &Connection::cb_prepare>(this);

    _user_cb.reserve(100);
}

Connection::~Connection() noexcept
{
    SPDLOG_TRACE("Conn={:p}, {}", reinterpret_cast<const void*>(this), __PRETTY_FUNCTION__);

    close(false, "destructor");
}

ConnectionStatus Connection::status() const noexcept
{
    SPDLOG_TRACE("Conn={:p}, {}", reinterpret_cast<const void*>(this), __PRETTY_FUNCTION__);

    return _status;
}

bool Connection::send(intrusive_ptr<Request>&& req)
{
    SPDLOG_TRACE("Conn={:p}, {}", reinterpret_cast<const void*>(this), __PRETTY_FUNCTION__);

    if (_status != ConnectionStatus::Closed && req) {
        auto resp = ResponseCache::get_intrusive();
        if (!resp->init(_config.message, {}))
            return false;

        req->set_ready(_is_proxy);
        _out.emplace_back(req);
        _in.emplace_back(std::move(req), std::move(resp));

        if (_config.response_timeout > 0ms && !_w_response_timeout.is_active())
            _w_response_timeout.again();

        //        if (!_w_write.is_active() && write_int())
        //            _w_write.start();

        if (!_w_write.is_active())
            _w_write.start();

        return true;
    }

    return false;
}

void Connection::connect()
{
    SPDLOG_TRACE("Conn={:p}, {}", reinterpret_cast<const void*>(this), __PRETTY_FUNCTION__);

    if (_status != ConnectionStatus::Closed)
        return;

    int fd = net::socket::tcp::create();
    if (!net::socket::tcp::set_no_delay(fd) || !net::socket::set_non_blocking(fd) || !net::socket::set_keep_alive(fd)) {
        ::close(fd);
        return;
    }

    if (_config.recv_buf && !net::socket::tcp::set_recv_buf(fd, _config.recv_buf)) {
        ::close(fd);
        return;
    }

    if (_config.send_buf && !net::socket::tcp::set_send_buf(fd, _config.send_buf)) {
        ::close(fd);
        return;
    }

    int rc = net::socket::tcp::connect(fd, _peer.ip(), _peer.port());
    if (rc < 0 && errno != EINPROGRESS) {
        ::close(fd);
        return;
    }

    if (_config.response_timeout > 0ms) {
        double to_d = (double)duration_cast<milliseconds>(_config.response_timeout).count() / 1000.0;
        _w_response_timeout.set(to_d, to_d);
        _w_connect_timeout.start(to_d, 0);
    }

    _w_read.start(fd, ev::READ);

    if (rc == 0) {
        _status = ConnectionStatus::Ready;
        _w_write.set(fd, ev::WRITE);
        _w_connect_timeout.stop();
    }
    else {
        _status = ConnectionStatus::Connecting;
        _w_write.start(fd, ev::WRITE);
    }
}

void Connection::close(bool run_cb_disconnect, string_view reason) noexcept
{
    SPDLOG_TRACE("Conn={:p}, reason={}, {}", reinterpret_cast<const void*>(this), reason, __PRETTY_FUNCTION__);

    if (_status == ConnectionStatus::Ready || _status == ConnectionStatus::Connecting) {
        _w_read.stop();
        _w_write.stop();
        _w_response_timeout.stop();
        _w_connect_timeout.stop();

        ::close(_w_read.fd);

        if (_cb) {
            for (auto&& item : _in) {
                get<0>(item)->close_reason = reason;
                get<0>(item)->_ts_end = get<0>(item)->_ts_start;
                get<1>(item) = ResponseCache::get_intrusive();
                _user_cb.emplace_back(std::move(item));
            }
        }

        if (!_w_prepare.is_active())
            _w_prepare.start();

        _in.clear();
        _out.clear();
        _status = ConnectionStatus::Closed;
    }
}

void Connection::cb_response_timeout(ev::timer& w, int revents) noexcept
{
    SPDLOG_TRACE("Conn={:p}, {}", reinterpret_cast<const void*>(this), __PRETTY_FUNCTION__);

    close(true, "response timeout");
}

void Connection::cb_connect_timeout(ev::timer& w, int revents) noexcept
{
    SPDLOG_TRACE("Conn={:p}, {}", reinterpret_cast<const void*>(this), __PRETTY_FUNCTION__);

    close(true, "connect timeout");
}

void Connection::cb_write(ev::io& w, int revents) noexcept
{
    SPDLOG_TRACE("Conn={:p}, {}", reinterpret_cast<const void*>(this), __PRETTY_FUNCTION__);

    if (_status == ConnectionStatus::Connecting) {
        if (!net::socket::is_connected(w.fd)) {
            close(true, "not connected");
            return;
        }

        _w_connect_timeout.stop();
        _status = ConnectionStatus::Ready;
    }

    if (!write_int() && w.is_active())
        w.stop();
}

RecvStatus Connection::read_int(int fd) noexcept
{
    SPDLOG_TRACE("Conn={:p}, {}", reinterpret_cast<const void*>(this), __PRETTY_FUNCTION__);

    return get<intrusive_ptr<Response>>(_in.front())->recv(_config.message, fd);
}

RecvStatus Connection::read_int_empty(int fd) const noexcept
{
    SPDLOG_TRACE("Conn={:p}, {}", reinterpret_cast<const void*>(this), __PRETTY_FUNCTION__);

    array<char, 10> buf{};

    if (ssize_t count = read(fd, buf.data(), buf.size()); count > 0) {
        log_err("[Client:Connection] close: read data without request");
        return RecvStatus::Err;
    }
    else if (count < 0 && errno == EINTR) {
        return RecvStatus::Partial;
    }
    else if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return RecvStatus::Async;
    }
    else {
        return RecvStatus::Err;
    }
}

void Connection::cb_read(ev::io& w, int revents) noexcept
{
    SPDLOG_TRACE("Conn={:p}, {}", reinterpret_cast<const void*>(this), __PRETTY_FUNCTION__);

    while (true) {
        if (_status == ConnectionStatus::Closed)
            return;

        auto rc = _in.empty() ? read_int_empty(w.fd) : read_int(w.fd);
        switch (rc) {
            case RecvStatus::Complete: {
                auto item = std::move(_in.front());
                _in.pop_front();

                get<intrusive_ptr<Request>>(item)->_ts_end = steady_clock::now();
                if (_cb)
                    _user_cb.emplace_back(item);


                if (!_w_prepare.is_active())
                    _w_prepare.start();

                if (!get<intrusive_ptr<Request>>(item)->keep_alive
                    || !get<intrusive_ptr<Response>>(item)->keep_alive()) {
                    close(true, "no keep alive");
                    return;
                }

                if (_in.empty() && !get<intrusive_ptr<Response>>(item)->tail().empty()) {
                    log_err("[Client:Connection] close: read data without request");
                    log_err("[Client:Connection] data={}, content-length={}, tail={}",
                            get<intrusive_ptr<Response>>(item)->data().size(),
                            get<intrusive_ptr<Response>>(item)->content_length(),
                            get<intrusive_ptr<Response>>(item)->tail().size());
                    close(true, "read data without request");
                    return;
                }

                if (!_in.empty()
                    && !get<intrusive_ptr<Response>>(_in.front())
                            ->init(_config.message, get<intrusive_ptr<Response>>(item)->tail())) {
                    log_err("[Client:Connection] close: tail ({}) > usual buf size ({})",
                            get<intrusive_ptr<Response>>(item)->tail().size(), _config.message.usual_size);
                    close(true, "tail > usual buf size");
                    return;
                }

                if (_config.response_timeout > 0ms) {
                    if (!_in.empty())
                        _w_response_timeout.again();
                    else
                        _w_response_timeout.stop();
                }

                continue;
            }
            case RecvStatus::Partial:
                continue;
            case RecvStatus::Async:
                return;
            case RecvStatus::Err:
                close(true, fmt::format("read: network error={}", strerror(errno)));
                return;
        }
    }
}

bool Connection::write_int() noexcept
{
    SPDLOG_TRACE("Conn={:p}, {}", reinterpret_cast<const void*>(this), __PRETTY_FUNCTION__);

    while (true) {
        if (_status == ConnectionStatus::Closed || _out.empty())
            return false;

        switch (_out.front()->send(_w_write.fd)) {
            case SendStatus::Complete:
                _out.pop_front();

                continue;
            case SendStatus::Async:
                return true;
            case SendStatus::Err:
                close(true, fmt::format("write: network error={}", strerror(errno)));
                return false;
        }
    }
}

string Connection::debug_info() const
{
    SPDLOG_TRACE("Conn={:p}, {}", reinterpret_cast<const void*>(this), __PRETTY_FUNCTION__);

    string out;

    out += fmt::format("\t\tConn {}\n", _peer.to_string());

    if (_status == ConnectionStatus::Closed)
        out += "\t\t\tStatus: closed\n";
    else if (_status == ConnectionStatus::Ready)
        out += "\t\t\tStatus: ready\n";
    else if (_status == ConnectionStatus::Connecting)
        out += "\t\t\tStatus: connecting\n";

    out += fmt::format("\t\t\tIn queue: {}\n", _in.size());
    out += fmt::format("\t\t\tOut queue: {}\n", _out.size());

    return out;
}

void Connection::cb_prepare(ev::prepare& w, int revents)
{
    SPDLOG_TRACE("Conn={:p}, {}", reinterpret_cast<const void*>(this), __PRETTY_FUNCTION__);

    w.stop();

    if (!_cb) {
        _user_cb.clear();
        return;
    }

    auto tmp = cache::ArrayCache<vector<tuple<intrusive_ptr<Request>, intrusive_ptr<Response>>>>::get_unique(
        _user_cb.capacity());
    tmp->swap(_user_cb);

    for (auto&& [req, resp] : *tmp) {
        try {
            _cb(std::move(req), std::move(resp));
        }
        catch (std::exception& e) {
            log_err("[Client:Connection] Exception in user callback: {}", e.what());
        }
        catch (...) {
            log_err("[Client:Connection] Exception in user callback");
        }
    }
}

} // namespace sniper::http::client
