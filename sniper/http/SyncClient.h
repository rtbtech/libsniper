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
#include <sniper/http/Client.h>
#include <sniper/std/chrono.h>
#include <sniper/std/vector.h>

/*
 * Usage
 *
 * http::SyncClient client(2s); // Timeout = 2s
 *
 * if (auto resp = client.get("http://metahash.org"); resp)
 *       log_info("code={}, size={}", resp->code(), resp->data().size());
 *
 */

namespace sniper::http {

class SyncClient final
{
public:
    explicit SyncClient(milliseconds timeout = 1s);
    explicit SyncClient(const client::Config& config);

    [[nodiscard]] client::ResponsePtr get(string_view url) noexcept;
    [[nodiscard]] client::ResponsePtr head(string_view url) noexcept;
    [[nodiscard]] client::ResponsePtr post(string_view url, string_view data) noexcept;
    [[nodiscard]] client::ResponsePtr put(string_view url, string_view data) noexcept;
    [[nodiscard]] client::ResponsePtr send(client::RequestPtr&& req) noexcept;

    [[nodiscard]] bool batch_add_get(string_view url) noexcept;
    [[nodiscard]] bool batch_add_head(string_view url) noexcept;
    [[nodiscard]] bool batch_add_post(string_view url, string_view data) noexcept;
    [[nodiscard]] bool batch_add_put(string_view url, string_view data) noexcept;
    [[nodiscard]] bool batch_add_send(client::RequestPtr&& req) noexcept;

    /* The order of the responses is not guaranteed
     */
    [[nodiscard]] vector<client::ResponsePtr> batch_run() noexcept;

private:
    void init(const client::Config& config);
    client::ResponsePtr send(client::Method method, string_view url, string_view data) noexcept;
    bool add(client::Method method, string_view url, string_view data) noexcept;

    event::loop_ptr _loop;
    unique_ptr<Client> _client;
    vector<client::ResponsePtr> _resp;
    size_t _count = 0;
};

} // namespace sniper::http
