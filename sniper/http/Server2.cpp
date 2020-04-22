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

#include <sniper/log/log.h>
#include <sniper/net/socket.h>
#include <sniper/std/check.h>
#include "Server2.h"

namespace sniper::http {

namespace {

bool set_no_delay(int fd) noexcept
{
    return net::socket::tcp::set_no_delay(fd) && net::socket::set_non_blocking(fd) && net::socket::set_keep_alive(fd);
}

int create_socket(const string& ip, uint16_t port, const server::Config& config) noexcept
{
    int fd = net::socket::tcp::create();
    if (fd < 0)
        return -1;

    if (!set_no_delay(fd) || !net::socket::set_reuse_addr_and_port(fd)) {
        ::close(fd);
        return -1;
    }

    if (config.recv_buf && !net::socket::tcp::set_recv_buf(fd, config.recv_buf)) {
        ::close(fd);
        return -1;
    }

    if (config.send_buf && !net::socket::tcp::set_send_buf(fd, config.send_buf)) {
        ::close(fd);
        return -1;
    }

    if (!net::socket::bind(fd, ip, port)) {
        ::close(fd);
        return -1;
    }

    if (!net::socket::tcp::listen(fd, config.backlog)) {
        ::close(fd);
        return -1;
    }

    return fd;
}

} // namespace

Server2::Server2(event::loop_ptr loop, server::Config config) :
    _loop(std::move(loop)), _config(config), _pool(make_intrusive<server2::Pool>())
{
    check(_loop, "[Server] loop is nullptr");
}

Server2::~Server2() noexcept
{
    for (auto& w : _w_accept)
        if (w->is_active()) {
            w->stop();
            ::close(w->fd);
        }
}

bool Server2::bind(uint16_t port, bool ssl) noexcept
{
    return bind("", port, ssl);
}

bool Server2::bind(const string& ip, uint16_t port, bool ssl) noexcept
{
    int fd = create_socket(ip, port, _config);
    if (fd < 0)
        return false;

    try {
        auto w = make_unique<WServer>();
        w->ssl = ssl;
        w->set(*_loop);
        w->set<Server2, &Server2::cb_accept>(this);
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

void Server2::cb_accept(ev::io& w, int revents) noexcept
{
    auto* wserver = static_cast<WServer*>(&w);

    while (true) {
        if (auto [fd, peer] = net::socket::tcp::accept(w.fd); fd >= 0) {
            if (!set_no_delay(fd)) {
                ::close(fd);
                continue;
            }

            if (auto conn = _pool->get(); conn) {
                // TODO: add params
                // fd, peer, wserver->ssl
                // _loop
                conn->attach(_pool);
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

} // namespace sniper::http
