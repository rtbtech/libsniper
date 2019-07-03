// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

/*
 * Copyright (c) 2019, MetaHash, Oleg Romanenko (oleg@romanenko.ro)
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

#include <sniper/cache/Cache.h>
#include <sniper/log/log.h>
#include <sniper/net/ip.h>
#include <sniper/net/socket.h>
#include <sniper/std/array.h>
#include <sniper/std/check.h>
#include <sys/socket.h>
#include "UDP.h"

namespace sniper::udp {

namespace {

enum class SendResult
{
    Ok,
    Again,
    Error
};

SendResult send(int fd, net::Peer peer, string_view data)
{
    if (fd < 0 || data.empty())
        return SendResult::Error;

    sockaddr_in to{};
    net::fill_addr(peer.ip(), peer.port(), to);

    while (true) {
        int count = sendto(fd, data.data(), data.size(), 0, (const struct sockaddr*)&to, sizeof(to));
        if (count > 0) {
            if ((unsigned)count != data.size())
                log_err("[UDP::send] sent count {} != data.size {}", count, data.size());

            return SendResult::Ok;
        }
        else if (count < 0 && errno == EINTR) {
            continue;
        }
        else if (count < 0 && errno == EMSGSIZE) {
            log_err("[UDP::send] message size {} too large", data.size());
            return SendResult::Error; // drop
        }
        else if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return SendResult::Again; // repeat
        }
        else {
            log_err("[UDP::send] network error={}", strerror(errno));
            return SendResult::Error;
        }
    }
}

const unsigned message_max_size = 65507;

struct Buf final
{
    std::array<char, message_max_size> arr;
};

} // namespace

UDP::UDP(event::loop_ptr loop) : _loop(std::move(loop))
{
    check(_loop, "[UDP] loop is nullptr");

    _w_read.set(*_loop);
    _w_write.set(*_loop);

    _w_read.set<UDP, &UDP::cb_read>(this);
    _w_write.set<UDP, &UDP::cb_write>(this);
}

bool UDP::bind(const string& ip, uint16_t port) noexcept
{
    if (_w_read.is_active())
        return false;

    if (!port)
        return false;

    int fd = net::socket::udp::create();
    if (fd < 0)
        return false;

    if (!net::socket::set_reuse_addr_and_port(fd) || !net::socket::set_non_blocking(fd)) {
        ::close(fd);
        return false;
    }

    if (!net::socket::bind(fd, ip, port)) {
        ::close(fd);
        return false;
    }

    _w_read.start(fd, ev::READ);
    _w_write.set(fd, ev::WRITE);

    return true;
}

bool UDP::send_copy(net::Peer peer, string_view data)
{
    if (!_w_read.is_active() || data.empty())
        return false;

    if (!_w_write.is_active()) {
        if (auto rc = send(_w_write.fd, peer, data); rc == SendResult::Error)
            return false;
        else if (rc == SendResult::Ok)
            return true;
    }

    // add
    auto str = cache::StringCache::get_unique(data.size());
    str->assign(data);
    _out.emplace_back(make_tuple(peer, std::move(str)));

    if (!_w_write.is_active())
        _w_write.start();

    return true;
}

bool UDP::send_nocopy(net::Peer peer, string_view data)
{
    if (!_w_read.is_active() || data.empty())
        return false;

    if (!_w_write.is_active()) {
        if (auto rc = send(_w_write.fd, peer, data); rc == SendResult::Error)
            return false;
        else if (rc == SendResult::Ok)
            return true;
    }

    // add
    _out.emplace_back(make_tuple(peer, data));

    if (!_w_write.is_active())
        _w_write.start();

    return true;
}

void UDP::cb_read(ev::io& w, int revents) noexcept
{
    auto buf = cache::Cache<Buf>::get_unique();

    while (true) {
        sockaddr_in from{};
        socklen_t fromlen = sizeof(from);
        int count = recvfrom(w.fd, buf->arr.data(), buf->arr.size(), 0, (struct sockaddr*)&from, &fromlen);

        if (count > 0) {
            if (_cb) {
                try {
                    net::Peer peer(from);
                    _cb(peer, string_view(buf->arr.data(), count));
                }
                catch (std::exception& e) {
                    log_err("[UDP:cb_read] Exception in user callback: {}", e.what());
                }
                catch (...) {
                    log_err("[UDP:cb_read] Exception in user callback");
                }

                if (!w.is_active())
                    return;
            }
        }
        else if (count < 0 && errno == EINTR) {
            continue;
        }
        else if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return;
        }
        else {
            log_err("[UDP::cb_read] network error={}", strerror(errno));
        }
    }
}

void UDP::cb_write(ev::io& w, int revents) noexcept
{
    while (!_out.empty()) {
        string_view data;
        if (holds_alternative<string_view>(std::get<1>(_out.front())))
            data = std::get<0>(std::get<1>(_out.front()));
        else
            data = *std::get<1>(std::get<1>(_out.front()));

        auto rc = send(_w_write.fd, std::get<0>(_out.front()), data);
        if (rc == SendResult::Ok) {
            _out.pop_front();
            continue;
        }
        else if (rc == SendResult::Again) {
            return;
        }
        else if (rc == SendResult::Error) {
            _out.pop_front();
            continue;
        }
    }

    w.stop();
}

} // namespace sniper::udp
