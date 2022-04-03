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

#include <linux/tcp.h>
#include <sniper/log/log.h>
#include <sniper/net/socket.h>
#include <sniper/std/check.h>
#include "Server.h"
#include "server/ServerInt.h"

namespace sniper::http {

namespace {

local_ptr<string> gen_date() noexcept
{
    try {
        return make_local<string>(fmt::format("Date: {:%a, %d %b %Y %H:%M:%S} GMT\r\n", fmt::gmtime(time(nullptr))));
    }
    catch (...) {
        return nullptr;
    }
}

} // namespace

Server::Server(event::loop_ptr loop) : Server(std::move(loop), nullptr)
{
    //
}

Server::Server(event::loop_ptr loop, const server::Config& config) :
    Server(std::move(loop), make_intrusive_noexcept<server::Config>(config))
{
    //
}

Server::Server(event::loop_ptr loop, intrusive_ptr<server::Config> config) :
    _loop(std::move(loop)), _config(std::move(config))
{
    check(_loop, "[Server] loop is nullptr");

    if (!_config)
        _config = server::make_config();

    check(_config, "[Server] config is nullptr");

    _pool = make_intrusive_noexcept<server::Pool>(_config);
    check(_pool, "[Server] pool is nullptr");

    _w_date.set(*_loop);
    _w_date.set<Server, &Server::cb_date>(this);
    _w_date.start(1.0, 1.0);
    _pool->date = gen_date();
}

Server::~Server() noexcept
{
    _w_date.stop();

    for (auto& w : _w_accept)
        if (w->is_active()) {
            w->stop();
            ::close(w->fd);
        }

    _pool->close();
}

bool Server::bind(uint16_t port) noexcept
{
    return bind("", port);
}

bool Server::bind(const string& ip, uint16_t port) noexcept
{
    int fd = server::internal::create_socket(ip, port, _config->send_buf, _config->recv_buf, _config->backlog);
    if (fd < 0)
        return false;

    try {
        auto w = make_unique<ev::io>();
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

void Server::cb_accept(ev::io& w, int revents) noexcept
{
    while (true) {
        if (auto [fd, peer] = net::socket::tcp::accept4(w.fd); fd >= 0) {
            net::socket::tcp::set_defer_accept(fd);
            net::socket::tcp::set_fastopen(fd);

            if (auto conn = _pool->get(_loop, _pool); conn) {
                conn->set(peer, fd);
                continue;
            }

            ::close(fd);
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

void Server::cb_date(ev::timer& w, int revents) noexcept
{
    _pool->date = gen_date();
}

} // namespace sniper::http
