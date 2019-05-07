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
    explicit SyncClient(client::Config config);

    [[nodiscard]] intrusive_ptr<client::Response> get(string_view url) noexcept;
    [[nodiscard]] intrusive_ptr<client::Response> head(string_view url) noexcept;
    [[nodiscard]] intrusive_ptr<client::Response> post(string_view url, string_view data) noexcept;
    [[nodiscard]] intrusive_ptr<client::Response> put(string_view url, string_view data) noexcept;

private:
    intrusive_ptr<client::Response> send(client::Method method, string_view url, string_view data) noexcept;

    event::loop_ptr _loop;
    unique_ptr<Client> _client;
    intrusive_ptr<client::Response> _resp;
};

} // namespace sniper::http
