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

//#define SNIPER_TRACE
#include <sniper/log/log.h>
#include <sniper/std/check.h>
#include "Client.h"

namespace sniper::http {

Client::Client(event::loop_ptr loop, client::Config config) : _loop(std::move(loop)), _config(std::move(config))
{
    check(_loop, "[Client] loop is nullptr");
}

bool Client::send(client::Method method, string_view url, string_view data)
{
    log_trace(__PRETTY_FUNCTION__);

    auto req = client::make_request();
    req->method = method;
    req->keep_alive = _config.pool.connection.message.keep_alive;

    if (!req->url.parse(url))
        return false;

    if (!data.empty())
        req->set_data_copy(data);

    return send(std::move(req));
}

bool Client::get(string_view url)
{
    return send(client::Method::Get, url);
}

bool Client::head(string_view url)
{
    return send(client::Method::Head, url);
}

bool Client::post(string_view url, string_view data)
{
    return send(client::Method::Post, url, data);
}

bool Client::put(string_view url, string_view data)
{
    return send(client::Method::Put, url, data);
}

bool Client::send(intrusive_ptr<client::Request>&& req)
{
    log_trace(__PRETTY_FUNCTION__);

    if (!req)
        return false;

    if (!req->url)
        return false;

    if (_config.proxy)
        return send(_config.proxy, std::move(req));

    return send(req->url.domain(), std::move(req));
}

bool Client::send(const net::Domain& domain, intrusive_ptr<client::Request>&& req)
{
    log_trace(__PRETTY_FUNCTION__);

    if (auto it = _pools.find(domain); it != _pools.end()) {
        it->second.send(std::move(req));
        return true;
    }

    if (_pools.size() >= _config.max_pools)
        return false;

    try {
        if (auto [it, rc] = _pools.try_emplace(domain, _loop, _config.pool, domain, domain == _config.proxy, _cb); rc) {
            it->second.send(std::move(req));
            return true;
        }
    }
    catch (std::exception& e) {
        log_err("[Client] Can't init pool: {}", e.what());
    }

    return false;
}

string Client::debug_info() const
{
    string out = fmt::format("Pools: {}\n", _pools.size());

    for (auto& [domain, pool] : _pools)
        out += pool.debug_info();

    return out;
}

} // namespace sniper::http
