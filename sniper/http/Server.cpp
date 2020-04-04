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

#include <sniper/http/server/Connection.h>
#include <sniper/log/log.h>
#include <sniper/net/socket.h>
#include <sniper/std/check.h>
#include "Server.h"

namespace sniper::http {

Server::Server(event::loop_ptr loop, server::Config config) : _loop(std::move(loop)), _config(config)
{
    check(_loop, "[Server] loop is nullptr");

    _w_clean.set(*_loop);
    _w_clean.set<Server, &Server::cb_clean>(this);

    auto sec = duration_cast<seconds>(_config.conns_clean_interval);
    _w_clean.start(sec.count(), sec.count());
}

Server::~Server() noexcept
{
    for (auto& w : _w_accept)
        if (w->is_active()) {
            w->stop();
            ::close(w->fd);
        }

    for (auto& conn : _conns)
        conn->disconnect();
}

bool Server::bind(uint16_t port, bool ssl) noexcept
{
    return bind("", port, ssl);
}

bool Server::bind(const string& ip, uint16_t port, bool ssl) noexcept
{
    if (!port)
        return false;

    int fd = net::socket::tcp::create();
    if (fd < 0)
        return false;

    if (!net::socket::tcp::set_no_delay(fd) || !net::socket::set_reuse_addr_and_port(fd)
        || !net::socket::set_non_blocking(fd) || !net::socket::set_keep_alive(fd)) {
        ::close(fd);
        return false;
    }

    if (_config.recv_buf && !net::socket::tcp::set_recv_buf(fd, _config.recv_buf)) {
        ::close(fd);
        return false;
    }

    if (_config.send_buf && !net::socket::tcp::set_send_buf(fd, _config.send_buf)) {
        ::close(fd);
        return false;
    }

    if (!net::socket::bind(fd, ip, port)) {
        ::close(fd);
        return false;
    }

    if (!net::socket::tcp::listen(fd, _config.backlog)) {
        ::close(fd);
        return false;
    }


    try {
        auto w = make_unique<ServerIO>();
        w->ssl = ssl;
        w->set(*_loop);
        w->set<Server, &Server::cb_accept>(this);
        w->start(fd, ev::READ);
        _w_accept.emplace_back(std::move(w));
    }
    catch (...) {
        // OOM guard
        perror("[OOM][Server] cannot bind");
        ::close(fd);
        return false;
    }

    return true;
}

intrusive_ptr<server::Connection> Server::get_conn()
{
    // looking for closed conn
    {
        size_t n = 0;
        for (auto it = _conns.begin(); it != _conns.end() && n < 50; ++it, ++n) {
            if ((*it)->status() == server::ConnectionStatus::Closed && (*it)->use_count() == 1) {
                _conns.splice(_conns.end(), _conns, it);
                return *it;
            }
        }
    }

    if (!_free_conns.empty()) {
        _conns.splice(_conns.end(), _free_conns, _free_conns.begin());
        return _conns.back();
    }

    if (_free_conns.size() + _conns.size() < _config.max_conns) {
        try {
            auto conn = make_intrusive<server::Connection>(_loop, _config.connection, _cb_request);
            return _conns.emplace_back(std::move(conn));
        }
        catch (...) {
            perror("[OOM][Server] Cannot allocate memory for conn");
            return nullptr;
        }
    }

    return nullptr;
}

void Server::cb_accept(ev::io& w, int revents) noexcept
{
    auto* server_w = static_cast<ServerIO*>(&w);

    while (true) {
        if (auto [fd, peer] = net::socket::tcp::accept(w.fd); fd >= 0) {
            if (!net::socket::tcp::set_no_delay(fd) || !net::socket::set_non_blocking(fd)
                || !net::socket::set_keep_alive(fd)) {
                ::close(fd);
                continue;
            }

            if (auto conn = get_conn(); conn) {
                if (conn->accept(fd, peer, server_w->ssl))
                    continue;

                conn->disconnect();
            }
            else {
                ::close(fd);
            }
        }
        else if (fd < 0 && errno == EINTR) {
            continue;
        }
        else if (fd < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return;
        }
        else {
            log_err("[Server:accept] cannot accept, error={}", strerror(errno));
            return;
        }
    }
}

void Server::cb_clean(ev::timer& w, int revents) noexcept
{
    for (auto it = _conns.begin(); it != _conns.end();) {
        if ((*it)->status() == server::ConnectionStatus::Closed && (*it)->use_count() == 1) {
            if (_free_conns.size() + _conns.size() > _config.max_conns) {
                it = _conns.erase(it);
            }
            else {
                auto it2 = it;
                ++it;
                _free_conns.splice(_free_conns.end(), _conns, it2);
            }
        }
        else {
            ++it;
        }
    }
}

} // namespace sniper::http
