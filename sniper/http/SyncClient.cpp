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

#include <limits>
#include "SyncClient.h"

namespace sniper::http {

SyncClient::SyncClient(milliseconds timeout)
{
    http::client::Config config;
    config.max_pools = std::numeric_limits<size_t>::max();
    config.pool.conns_per_ip = 1;
    config.pool.max_conns = std::numeric_limits<size_t>::max();
    config.pool.connection.response_timeout = timeout;

    init(config);
}

SyncClient::SyncClient(const client::Config& config)
{
    init(config);
}

void SyncClient::init(const client::Config& config)
{
    _resp.reserve(10);

    _loop = event::make_loop();
    _client = make_unique<Client>(_loop, config);

    _client->set_cb([this](const auto& req, const auto& resp) {
        _resp.emplace_back(std::move(resp));

        if (_resp.size() >= _count)
            _loop->break_loop(ev::ALL);
    });
}

bool SyncClient::add(client::Method method, string_view url, string_view data) noexcept
{
    try {
        auto req = client::make_request();
        req->method = method;

        if (!req->url.parse(url))
            return false;

        if (!data.empty())
            req->set_data_copy(data);

        if (_client->send(std::move(req))) {
            _count++;
            return true;
        }
    }
    catch (...) {
        //         Do nothing, return false
    }

    return false;
}

client::ResponsePtr SyncClient::send(client::Method method, string_view url, string_view data) noexcept
{
    if (_count)
        return nullptr;

    _resp.clear();
    ev_now_update(_loop->raw_loop);

    if (add(method, url, data))
        _loop->run();

    _count = 0;
    if (!_resp.empty())
        return _resp.front();

    return nullptr;
}

client::ResponsePtr SyncClient::get(string_view url) noexcept
{
    return send(client::Method::Get, url, "");
}

client::ResponsePtr SyncClient::head(string_view url) noexcept
{
    return send(client::Method::Head, url, "");
}

client::ResponsePtr SyncClient::post(string_view url, string_view data) noexcept
{
    return send(client::Method::Post, url, data);
}

client::ResponsePtr SyncClient::put(string_view url, string_view data) noexcept
{
    return send(client::Method::Put, url, data);
}

client::ResponsePtr SyncClient::send(client::RequestPtr&& req) noexcept
{
    if (_count)
        return nullptr;

    _resp.clear();
    ev_now_update(_loop->raw_loop);

    try {
        if (_client->send(std::move(req))) {
            _count = 1;
            _loop->run();
        }
    }
    catch (...) {
        //         Do nothing, return empty ptr
    }

    _count = 0;
    if (!_resp.empty())
        return _resp.front();

    return nullptr;
}

bool SyncClient::batch_add_get(string_view url) noexcept
{
    return add(client::Method::Get, url, "");
}

bool SyncClient::batch_add_head(string_view url) noexcept
{
    return add(client::Method::Head, url, "");
}

bool SyncClient::batch_add_post(string_view url, string_view data) noexcept
{
    return add(client::Method::Post, url, data);
}

bool SyncClient::batch_add_put(string_view url, string_view data) noexcept
{
    return add(client::Method::Put, url, data);
}

bool SyncClient::batch_add_send(client::RequestPtr&& req) noexcept
{
    if (_client->send(std::move(req))) {
        _count++;
        return true;
    }

    return false;
}

vector<client::ResponsePtr> SyncClient::batch_run() noexcept
{
    _resp.clear();
    ev_now_update(_loop->raw_loop);

    if (_count)
        _loop->run();

    _count = 0;
    return _resp;
}

} // namespace sniper::http
