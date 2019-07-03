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
#include <errno.h>
#include <fcntl.h>
#include <linux/tcp.h>
#include <netinet/in.h>
#include <sniper/net/ip.h>
#include <stdexcept>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "socket.h"

namespace sniper::net::socket {

bool set_non_blocking(int fd)
{
    if (fd < 0)
        return false;

    int nb = 1;
    return ioctl(fd, FIONBIO, &nb) == 0;
}

bool set_keep_alive(int fd)
{
    if (fd < 0)
        return false;

    int enable = 1;
    return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(enable)) == 0;
}

bool is_connected(int fd)
{
    if (fd < 0)
        return false;

    int result = 0;
    socklen_t result_len = sizeof(result);
    return getsockopt(fd, SOL_SOCKET, SO_ERROR, &result, &result_len) == 0 && !result;
}

bool set_reuse_addr_and_port(int fd)
{
    if (fd < 0)
        return false;

    int enable = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(enable)) == 0;
}

bool bind(int fd, const string& ip, uint16_t port)
{
    uint32_t u = 0;
    if (!ip.empty() && !net::ip_from_str(ip, u))
        return false;

    return bind(fd, u, port);
}

bool bind(int fd, uint32_t ip, uint16_t port)
{
    if (fd < 0 || !port)
        return false;

    sockaddr_in servaddr{};
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = (ip == 0) ? htonl(INADDR_ANY) : ip;

    return ::bind(fd, (sockaddr*)&servaddr, sizeof(servaddr)) == 0;
}


namespace udp {

int create()
{
    return ::socket(AF_INET, SOCK_DGRAM, 0);
}

} // namespace udp

namespace tcp {

int create()
{
    return ::socket(AF_INET, SOCK_STREAM, 0);
}

bool set_no_delay(int fd)
{
    if (fd < 0)
        return false;

    int enable = 1;
    return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable)) == 0;
}

bool get_rtt(int fd, uint32_t& rtt)
{
    if (fd < 0)
        return false;

    struct tcp_info ti
    {};
    socklen_t ti_size = sizeof(ti);
    if (getsockopt(fd, IPPROTO_TCP, TCP_INFO, &ti, &ti_size) < 0)
        return false;

    rtt = ti.tcpi_rtt;
    return true;
}

bool set_mss(int fd, uint16_t mss)
{
    if (fd < 0)
        return false;

    return setsockopt(fd, IPPROTO_TCP, TCP_MAXSEG, &mss, sizeof(mss)) == 0;
}

bool get_mss(int fd, uint16_t& mss)
{
    if (fd < 0)
        return false;

    socklen_t mss_size = sizeof(mss);
    return getsockopt(fd, IPPROTO_TCP, TCP_MAXSEG, &mss, &mss_size) == 0;
}

bool set_enable_cork(int fd)
{
    if (fd < 0)
        return false;

    int enable = 1;
    return setsockopt(fd, IPPROTO_TCP, TCP_CORK, &enable, sizeof(enable)) == 0;
}

bool set_disable_cork(int fd)
{
    if (fd < 0)
        return false;

    int enable = 0;
    return setsockopt(fd, IPPROTO_TCP, TCP_CORK, &enable, sizeof(enable)) == 0;
}

bool set_recv_buf(int fd, uint32_t size)
{
    if (fd < 0)
        return false;

    return setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(size)) == 0;
}

bool set_send_buf(int fd, uint32_t size)
{
    if (fd < 0)
        return false;

    return setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(size)) == 0;
}

bool get_recv_buf(int fd, uint32_t& size)
{
    if (fd < 0)
        return false;

    socklen_t buf_size = sizeof(size);
    return getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, &buf_size) == 0;
}

bool get_send_buf(int fd, uint32_t& size)
{
    if (fd < 0)
        return false;

    socklen_t buf_size = sizeof(size);
    return getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &size, &buf_size) == 0;
}

int accept(int server_fd, uint32_t& ip, uint16_t& port)
{
    if (server_fd < 0) {
        errno = EBADF;
        return -1;
    }

    sockaddr_in servaddr{};
    memset(&servaddr, 0, sizeof(servaddr));

    socklen_t sa_len = sizeof(servaddr);
    int fd = ::accept(server_fd, (sockaddr*)&servaddr, &sa_len);
    if (fd >= 0) {
        ip = servaddr.sin_addr.s_addr;
        port = ntohs(servaddr.sin_port);
    }

    return fd;
}

tuple<int, Peer> accept(int server_fd)
{
    uint32_t ip = 0;
    uint16_t port = 0;

    int rc = accept(server_fd, ip, port);
    return make_tuple(rc, Peer(ip, port));
}

int connect(int fd, uint32_t ip, uint16_t port)
{
    if (fd < 0)
        return -1;

    sockaddr_in server{};
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = ip;

    return ::connect(fd, (struct sockaddr*)&server, sizeof(server));
}

bool listen(int fd, int backlog)
{
    if (fd < 0)
        return false;

    return ::listen(fd, backlog) == 0;
}

} // namespace tcp

} // namespace sniper::net::socket
