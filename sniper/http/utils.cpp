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

#include <sniper/event/Loop.h>
#include <sniper/http/Client.h>
#include "utils.h"

namespace sniper::http {

namespace {

intrusive_ptr<client::Response> sync_send(client::Method method, string_view url, string_view data,
                                          milliseconds timeout) noexcept
{
    intrusive_ptr<client::Response> out;

    try {
        auto loop = event::make_loop();
        if (!loop)
            return out;

        client::Config config;
        config.pool.connection.response_timeout = timeout;


        Client client(loop, config);
        {
            client.set_cb([&](const auto& req, const auto& resp) {
                out = std::move(resp);
                loop->break_loop(ev::ALL);
            });

            auto req = client::make_request();
            req->method = method;
            req->keep_alive = false;

            if (!req->url.parse(url))
                return out;

            if (!data.empty())
                req->set_data_copy(data);

            if (!client.send(std::move(req)))
                return out;
        }


        loop->run();
    }
    catch (...) {
        // Do nothing, return empty ptr
    }

    return out;
}

} // namespace

intrusive_ptr<client::Response> sync_get(string_view url, milliseconds timeout) noexcept
{
    return sync_send(client::Method::Get, url, "", timeout);
}

intrusive_ptr<client::Response> sync_head(string_view url, milliseconds timeout) noexcept
{
    return sync_send(client::Method::Head, url, "", timeout);
}

intrusive_ptr<client::Response> sync_post(string_view url, string_view data, milliseconds timeout) noexcept
{
    return sync_send(client::Method::Post, url, data, timeout);
}

intrusive_ptr<client::Response> sync_put(string_view url, string_view data, milliseconds timeout) noexcept
{
    return sync_send(client::Method::Put, url, data, timeout);
}

} // namespace sniper::http
