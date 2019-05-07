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

#include <sniper/log/log.h>
#include <sniper/std/check.h>
#include "Connection.h"
#include "Request.h"
#include "Response.h"

namespace sniper::http::server {

Connection::Connection(event::loop_ptr loop, ConnectionConfig config,
                       const function<void(const intrusive_ptr<Connection>&, const intrusive_ptr<Request>&,
                                           const intrusive_ptr<Response>&)>& cb) :
    _loop(std::move(loop)),
    _config(config), _cb(cb)
{
    check(_loop, "[Server:Connection] loop is nullptr");

    _w_read.set(*_loop);
    _w_write.set(*_loop);
    _w_prepare.set(*_loop);
    _w_keep_alive_timeout.set(*_loop);
    _w_request_read_timeout.set(*_loop);

    _w_read.set<Connection, &Connection::cb_read>(this);
    _w_write.set<Connection, &Connection::cb_write>(this);
    _w_prepare.set<Connection, &Connection::cb_prepare>(this);
    _w_keep_alive_timeout.set<Connection, &Connection::cb_keep_alive_timeout>(this);
    _w_request_read_timeout.set<Connection, &Connection::cb_request_read_timeout>(this);

    _user_cb.reserve(100);
}

Connection::~Connection() noexcept
{
    close();
}

ConnectionStatus Connection::status() const noexcept
{
    return _status;
}

bool Connection::accept(int fd, net::Peer peer) noexcept
{
    if (fd <= 0 || _status != ConnectionStatus::Closed)
        return false;

    try {
        _in.reset(RequestCache::get_raw());
    }
    catch (...) {
        perror("[Server:Connection] Cannot allocate memory for new request");
        // OOM
        return false;
    }

    if (!_in->init(_config.message, {}))
        return false;


    if (_config.keep_alive_timeout > 0ms) {
        double to_d = (double)duration_cast<milliseconds>(_config.keep_alive_timeout).count() / 1000.0;
        _w_keep_alive_timeout.start(to_d, to_d);
    }

    if (_config.request_read_timeout > 0ms) {
        double to_d = (double)duration_cast<milliseconds>(_config.request_read_timeout).count() / 1000.0;
        _w_request_read_timeout.start(to_d, to_d);
    }


    _w_read.start(fd, ev::READ);
    _w_write.set(fd, ev::WRITE);

    _peer = peer;
    _status = ConnectionStatus::Ready;

    return true;
}

void Connection::close() noexcept
{
    if (_status == ConnectionStatus::Ready) {
        _w_read.stop();
        _w_write.stop();
        _w_keep_alive_timeout.stop();
        _w_request_read_timeout.stop();

        ::close(_w_read.fd);

        _in.reset();
        _out.clear();

        if (!_user_cb.empty() || _w_prepare.is_active())
            _status = ConnectionStatus::Pending;
        else
            _status = ConnectionStatus::Closed;
    }
}

void Connection::disconnect() noexcept
{
    close();
}

void Connection::send(const intrusive_ptr<Response>& resp) noexcept
{
    if (resp && _status == ConnectionStatus::Ready) {
        resp->set_ready();

        //        if (!_out.empty() && _out.front() == resp && !_w_write.is_active() && write_int())
        //            _w_write.start();

        if (!_out.empty() && _out.front() == resp && !_w_write.is_active())
            _w_write.start();
    }
}

void Connection::cb_read(ev::io& w, int revents) noexcept
{
    while (true) {
        switch (_in->recv(_config.message, w.fd)) {
            case RecvStatus::Complete:
                try {
                    if (auto resp = _out.emplace_back(ResponseCache::get_raw()); resp) {
                        resp->set_keep_alive(_in->keep_alive());
                        resp->set_minor_version(_in->minor_version());
                        _user_cb.emplace_back(_in, resp);

                        if (!_w_prepare.is_active())
                            _w_prepare.start();
                    }
                }
                catch (...) {
                    // OOM guard
                    perror("[OOM][Server:Connection] close: cannot allocate memory for new response object\n");
                    close();
                    return;
                }

                try {
                    intrusive_ptr<Request> tmp(RequestCache::get_raw());
                    if (!tmp->init(_config.message, _in->tail())) {
                        log_err("[Server:Connection] close: tail ({}) > usual buf size ({})", _in->tail().size(),
                                _config.message.usual_size);
                        close();
                        return;
                    }

                    _in = std::move(tmp);
                }
                catch (...) {
                    // OOM guard
                    perror("[OOM][Server:Connection] close: cannot allocate memory for new request object\n");
                    close();
                    return;
                }

                // relaunch timeout timer
                if (_w_keep_alive_timeout.is_active())
                    _w_keep_alive_timeout.again();

                _req_in_progress = false;
                _req_count++;
                continue;
            case RecvStatus::Partial:
                _req_in_progress = true;
                continue;
            case RecvStatus::Async:
                return;
            case RecvStatus::Err:
                close();
                return;
        }
    }
}

bool Connection::write_int() noexcept
{
    while (true) {
        if (_status == ConnectionStatus::Closed || _out.empty() || !_out.front()->is_ready())
            return false;

        switch (_out.front()->send(_w_write.fd)) {
            case SendStatus::Complete:
                if (!_out.front()->keep_alive()) {
                    close();
                    return false;
                }

                _out.pop_front();
                continue;
            case SendStatus::Async:
                return true;
            case SendStatus::Err:
                close();
                return false;
        }
    }
}

void Connection::cb_write(ev::io& w, int revents) noexcept
{
    if (!write_int() && w.is_active())
        w.stop();
}

void Connection::cb_keep_alive_timeout(ev::timer& w, int revents) noexcept
{
    close();
}

void Connection::cb_request_read_timeout(ev::timer& w, int revents) noexcept
{
    if (_req_in_progress) {
        if (_prev_req_count && *_prev_req_count == _req_count)
            close();
        else
            _prev_req_count = _req_count;
    }
}

net::Peer Connection::peer() const noexcept
{
    return _peer;
}

void Connection::cb_prepare(ev::prepare& w, int revents)
{
    auto tmp = cache::ArrayCache<vector<tuple<intrusive_ptr<Request>, intrusive_ptr<Response>>>>::get_unique(
        _user_cb.capacity());
    tmp->swap(_user_cb);

    for (auto&& [req, resp] : *tmp) {
        if (!_cb) {
            send(resp);
            continue;
        }

        try {
            _cb(intrusive_ptr(this), req, resp);
        }
        catch (std::exception& e) {
            log_err("[Server:Connection] Exception in user callback: {}", e.what());
        }
        catch (...) {
            log_err("[Server:Connection] Exception in user callback");
        }
    }

    if (_user_cb.empty()) {
        w.stop();

        if (_status == ConnectionStatus::Pending)
            _status = ConnectionStatus::Closed;
    }
}

} // namespace sniper::http::server
