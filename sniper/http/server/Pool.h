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

#include <sniper/event/Loop.h>
#include <sniper/std/functional.h>
#include <sniper/std/map.h>
#include <sniper/std/memory.h>
#include <sniper/std/vector.h>

namespace sniper::http::server {

struct Config;
struct Connection;
struct Request;
struct Response;

struct Pool final : public intrusive_unsafe_ref_counter<Pool>
{
    explicit Pool(intrusive_ptr<Config> config);
    ~Pool();

    intrusive_ptr<Connection> get(const event::loop_ptr& loop, const intrusive_ptr<Pool>& pool) noexcept;
    void disconnect(Connection* conn) noexcept;
    void close() noexcept;

    intrusive_ptr<Config> _config;
    unordered_map<Connection*, intrusive_ptr<Connection>> _conns;
    vector<intrusive_ptr<Connection>> _free_conns;
    function<void(const intrusive_ptr<Connection>&, const intrusive_ptr<Request>&, const intrusive_ptr<Response>&)> _cb;

    local_ptr<string> date;
};

} // namespace sniper::http::server
