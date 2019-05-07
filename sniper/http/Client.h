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
#include <sniper/http/client/Pool.h>
#include <sniper/http/client/Request.h>
#include <sniper/http/client/Response.h>
#include <sniper/std/functional.h>
#include <sniper/std/map.h>

namespace sniper::http {

class Client final
{
public:
    explicit Client(event::loop_ptr loop, client::Config config = {});

    [[nodiscard]] bool get(string_view url);
    [[nodiscard]] bool head(string_view url);
    [[nodiscard]] bool post(string_view url, string_view data);
    [[nodiscard]] bool put(string_view url, string_view data);

    [[nodiscard]] bool send(intrusive_ptr<client::Request>&& req);

    template<typename T>
    void set_cb(T&& cb);

    [[nodiscard]] string debug_info() const;

private:
    [[nodiscard]] bool send(client::Method method, string_view url, string_view data = {});
    [[nodiscard]] bool send(const net::Domain& domain, intrusive_ptr<client::Request>&& req);

    event::loop_ptr _loop;
    client::Config _config;

    function<void(intrusive_ptr<client::Request>&&, intrusive_ptr<client::Response>&&)> _cb;

    unordered_map<net::Domain, client::Pool> _pools;
};

template<typename T>
void Client::set_cb(T&& cb)
{
    _cb = std::forward<T>(cb);
}

} // namespace sniper::http
