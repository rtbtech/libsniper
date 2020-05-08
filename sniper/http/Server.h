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

#pragma once

#include <sniper/cache/Cache.h>
#include <sniper/event/Loop.h>
#include <sniper/http/server/Config.h>
#include <sniper/http/server/Connection.h>
#include <sniper/http/server/Pool.h>
#include <sniper/http/server/Request.h>
#include <sniper/http/server/Response.h>
#include <sniper/std/list.h>

namespace sniper::http {

class Server final
{
public:
    explicit Server(event::loop_ptr loop, intrusive_ptr<server::Config> config = nullptr);
    ~Server() noexcept;

    template<typename T>
    void set_cb(T&& cb);

    [[nodiscard]] bool bind(uint16_t port) noexcept;
    [[nodiscard]] bool bind(const string& ip, uint16_t port) noexcept;

private:
    void cb_accept(ev::io& w, [[maybe_unused]] int revents) noexcept;
    void cb_date(ev::timer& w, [[maybe_unused]] int revents) noexcept;

    event::loop_ptr _loop;
    ev::timer _w_date;
    intrusive_ptr<server::Config> _config;
    list<unique_ptr<ev::io>> _w_accept;
    intrusive_ptr<server::Pool> _pool;
};

template<typename T>
void Server::set_cb(T&& cb)
{
    _pool->_cb = std::forward<T>(cb);
}

} // namespace sniper::http
