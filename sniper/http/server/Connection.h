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

#pragma once

#include <sniper/event/Loop.h>
#include <sniper/http/server/Config.h>
#include <sniper/net/Peer.h>
#include <sniper/std/deque.h>
#include <sniper/std/functional.h>
#include <sniper/std/memory.h>
#include <sniper/std/optional.h>
#include <sniper/std/vector.h>

namespace sniper::http {

class Server;

} // namespace sniper::http

namespace sniper::http::server {

class Request;
class Response;

enum class ConnectionStatus
{
    Ready,
    Pending,
    Closed
};

class Connection final : public intrusive_unsafe_ref_counter<Connection>
{
public:
    Connection(event::loop_ptr loop, ConnectionConfig config,
               const function<void(const intrusive_ptr<Connection>&, const intrusive_ptr<Request>&,
                                   const intrusive_ptr<Response>&)>& cb);
    ~Connection() noexcept;

    void disconnect() noexcept;

    [[nodiscard]] ConnectionStatus status() const noexcept;
    [[nodiscard]] net::Peer peer() const noexcept;

    void send(const intrusive_ptr<Response>& resp) noexcept;

private:
    friend class sniper::http::Server;

    [[nodiscard]] bool accept(int fd, net::Peer peer, bool ssl) noexcept;

    void cb_prepare(ev::prepare& w, [[maybe_unused]] int revents);
    void cb_read(ev::io& w, [[maybe_unused]] int revents) noexcept;
    void cb_write(ev::io& w, [[maybe_unused]] int revents) noexcept;
    [[nodiscard]] bool write_int() noexcept;
    void cb_keep_alive_timeout(ev::timer& w, [[maybe_unused]] int revents) noexcept;
    void cb_request_read_timeout(ev::timer& w, [[maybe_unused]] int revents) noexcept;
    void close() noexcept;

    event::loop_ptr _loop;
    ConnectionConfig _config;

    const function<void(const intrusive_ptr<Connection>&, const intrusive_ptr<Request>&,
                        const intrusive_ptr<Response>&)>& _cb;

    ev::io _w_read;
    ev::io _w_write;
    ev::prepare _w_prepare;
    ev::timer _w_keep_alive_timeout;
    ev::timer _w_request_read_timeout;

    net::Peer _peer;
    ConnectionStatus _status = ConnectionStatus::Closed;

    intrusive_ptr<Request> _in;
    deque<intrusive_ptr<Response>> _out;
    vector<tuple<intrusive_ptr<Request>, intrusive_ptr<Response>>> _user_cb;

    uint64_t _req_count = 0;
    bool _req_in_progress = false;
    optional<uint64_t> _prev_req_count;
};

} // namespace sniper::http::server
