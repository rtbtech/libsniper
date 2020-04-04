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
#include <sniper/http/server/Connection.h>
#include <sniper/http/server/Request.h>
#include <sniper/http/server/Response.h>
#include <sniper/std/functional.h>
#include <sniper/std/list.h>
#include <sniper/std/string.h>

namespace sniper::http {

class Server final
{
public:
    explicit Server(event::loop_ptr loop, server::Config config = {});
    ~Server() noexcept;

    [[nodiscard]] bool bind(uint16_t port, bool ssl = false) noexcept;
    [[nodiscard]] bool bind(const string& ip, uint16_t port, bool ssl = false) noexcept;

    template<typename T>
    void set_cb_request(T&& cb);

private:
    struct ServerIO : public ev::io
    {
        bool ssl = false;
    };

    [[nodiscard]] intrusive_ptr<server::Connection> get_conn();
    void cb_accept(ev::io& w, [[maybe_unused]] int revents) noexcept;
    void cb_clean(ev::timer& w, [[maybe_unused]] int revents) noexcept;

    event::loop_ptr _loop;
    server::Config _config;

    function<void(const intrusive_ptr<server::Connection>&, const intrusive_ptr<server::Request>&,
                  const intrusive_ptr<server::Response>&)>
        _cb_request;

    ev::timer _w_clean;
    list<unique_ptr<ServerIO>> _w_accept;
    list<intrusive_ptr<server::Connection>> _conns;

    // use only refcount=1 pointers from this list
    // or create new conn
    list<intrusive_ptr<server::Connection>> _free_conns;
};

template<typename T>
void Server::set_cb_request(T&& cb)
{
    _cb_request = std::forward<T>(cb);
}

} // namespace sniper::http
