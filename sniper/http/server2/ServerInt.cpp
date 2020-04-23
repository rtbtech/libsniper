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
#include <unistd.h>
#include "ServerInt.h"

namespace sniper::http::server2::internal {

bool set_no_delay(int fd) noexcept
{
    return net::socket::tcp::set_no_delay(fd) && net::socket::set_non_blocking(fd) && net::socket::set_keep_alive(fd);
}

int create_socket(const string& ip, uint16_t port, uint32_t send_buf, uint32_t recv_buf, int backlog) noexcept
{
    int fd = net::socket::tcp::create();
    if (fd < 0)
        return -1;

    if (!set_no_delay(fd)) {
        log_err("[create_socket] set_no_delay error");
        ::close(fd);
        return -1;
    }

    if (!net::socket::set_reuse_addr_and_port(fd)) {
        log_err("[create_socket] set_reuse_addr_and_port error");
        ::close(fd);
        return -1;
    }

    if (recv_buf && !net::socket::tcp::set_recv_buf(fd, recv_buf)) {
        log_err("[create_socket] set_recv_buf error");
        ::close(fd);
        return -1;
    }

    if (send_buf && !net::socket::tcp::set_send_buf(fd, send_buf)) {
        log_err("[create_socket] set_send_buf error");
        ::close(fd);
        return -1;
    }

    if (!net::socket::bind(fd, ip, port)) {
        log_err("[create_socket] bind error");
        ::close(fd);
        return -1;
    }

    if (!net::socket::tcp::listen(fd, backlog)) {
        log_err("[create_socket] listen error");
        ::close(fd);
        return -1;
    }

    return fd;
}

} // namespace sniper::http::server2::internal
