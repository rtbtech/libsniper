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
#include <sniper/http/client/Config.h>
#include <sniper/net/Peer.h>
#include <sniper/std/deque.h>
#include <sniper/std/functional.h>
#include <sniper/std/memory.h>
#include <sniper/std/vector.h>

namespace sniper::http::client {

enum class ConnectionStatus
{
    Connecting,
    Ready,
    Closed
};

enum class RecvStatus;

class Request;
class Response;

class Connection
{
public:
    Connection(event::loop_ptr loop, ConnectionConfig config, net::Peer peer, bool is_proxy,
               const function<void(intrusive_ptr<Request>&&, intrusive_ptr<Response>&&)>& cb);
    ~Connection() noexcept;

    [[nodiscard]] ConnectionStatus status() const noexcept;

    // Does not invoke CB
    [[nodiscard]] bool send(intrusive_ptr<Request>&& req);
    void connect();

    [[nodiscard]] string debug_info() const;

private:
    void cb_prepare(ev::prepare& w, [[maybe_unused]] int revents);
    void cb_read(ev::io& w, [[maybe_unused]] int revents) noexcept;
    void cb_write(ev::io& w, [[maybe_unused]] int revents) noexcept;
    void cb_response_timeout(ev::timer& w, [[maybe_unused]] int revents) noexcept;
    void cb_connect_timeout(ev::timer& w, [[maybe_unused]] int revents) noexcept;
    void close(bool run_cb_disconnect, string_view reason) noexcept;
    [[nodiscard]] bool write_int() noexcept;
    [[nodiscard]] RecvStatus read_int(int fd) noexcept;
    [[nodiscard]] RecvStatus read_int_empty(int fd) const noexcept;

    event::loop_ptr _loop;
    ConnectionConfig _config;
    net::Peer _peer;
    bool _is_proxy;

    ev::io _w_read;
    ev::io _w_write;
    ev::prepare _w_prepare;
    ev::timer _w_response_timeout;
    ev::timer _w_connect_timeout;

    const function<void(intrusive_ptr<Request>&&, intrusive_ptr<Response>&&)>& _cb;
    ConnectionStatus _status = ConnectionStatus::Closed;

    deque<intrusive_ptr<Request>> _out;
    deque<tuple<intrusive_ptr<Request>, intrusive_ptr<Response>>> _in;
    vector<tuple<intrusive_ptr<Request>, intrusive_ptr<Response>>> _user_cb;
};

} // namespace sniper::http::client
