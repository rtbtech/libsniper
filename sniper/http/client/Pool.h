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
#include <sniper/http/client/Connection.h>
#include <sniper/std/functional.h>
#include <sniper/std/list.h>
#include <sniper/std/memory.h>
#include <sniper/std/vector.h>

namespace sniper::http::client {

class Request;

class Pool final
{
public:
    Pool(event::loop_ptr loop, PoolConfig config, const net::Domain& domain, bool is_proxy,
         const function<void(intrusive_ptr<Request>&&, intrusive_ptr<Response>&&)>& cb);

    void send(intrusive_ptr<Request>&& req);

    [[nodiscard]] string debug_info() const;

private:
    void cb_prepare(ev::prepare& w, [[maybe_unused]] int revents);
    bool _send(intrusive_ptr<Request>&& req);

    event::loop_ptr _loop;
    PoolConfig _config;
    net::Domain _domain;
    bool _is_proxy = false;
    ev::prepare _w;

    vector<intrusive_ptr<Request>> _out;
    list<Connection> _active;
    list<Connection> _connecting;

    const function<void(intrusive_ptr<Request>&&, intrusive_ptr<Response>&&)>& _cb;
};

} // namespace sniper::http::client
