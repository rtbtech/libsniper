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

#include "SyncClient.h"

namespace sniper::http {

SyncClient::SyncClient(milliseconds timeout) : _loop(event::make_loop())
{
    http::client::Config config;
    config.pool.connection.response_timeout = timeout;
    _client = make_unique<Client>(_loop, config);

    _client->set_cb([this](const auto& req, const auto& resp) {
        _resp = std::move(resp);
        _loop->break_loop(ev::ALL);
    });
}

SyncClient::SyncClient(client::Config config) : _loop(event::make_loop()), _client(make_unique<Client>(_loop, config))
{
    _client->set_cb([this](const auto& req, const auto& resp) {
        _resp = std::move(resp);
        _loop->break_loop(ev::ALL);
    });
}

intrusive_ptr<client::Response> SyncClient::send(client::Method method, string_view url, string_view data) noexcept
{
    _resp.reset();
    ev_now_update(_loop->raw_loop);

    try {
        auto req = client::make_request();
        req->method = method;

        if (!req->url.parse(url))
            return nullptr;

        if (!data.empty())
            req->set_data_copy(data);

        if (!_client->send(std::move(req)))
            return nullptr;

        _loop->run();
    }
    catch (...) {
        //         Do nothing, return empty ptr
    }

    return _resp;
}

intrusive_ptr<client::Response> SyncClient::get(string_view url) noexcept
{
    return send(client::Method::Get, url, "");
}

intrusive_ptr<client::Response> SyncClient::head(string_view url) noexcept
{
    return send(client::Method::Head, url, "");
}

intrusive_ptr<client::Response> SyncClient::post(string_view url, string_view data) noexcept
{
    return send(client::Method::Post, url, data);
}

intrusive_ptr<client::Response> SyncClient::put(string_view url, string_view data) noexcept
{
    return send(client::Method::Put, url, data);
}

} // namespace sniper::http
