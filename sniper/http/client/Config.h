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

#include <sniper/net/Domain.h>
#include <sniper/std/chrono.h>

namespace sniper::http::client {

struct MessageConfig final
{
    bool keep_alive = true;

    size_t header_max_size = 4 * 1024;
    size_t body_max_size = 128 * 1024;
    size_t header_chunk_size = 1024;
};

struct ConnectionConfig final
{
    uint32_t recv_buf = 20 * 1024 * 1024;
    uint32_t send_buf = 20 * 1024 * 1024;

    milliseconds response_timeout = 5s;

    MessageConfig message;
};

struct PoolConfig final
{
    size_t max_conns = 10;
    size_t conns_per_ip = 1;

    ConnectionConfig connection;
};

struct Config final
{
    net::Domain proxy;
    size_t max_pools = 1000;

    PoolConfig pool;
};

} // namespace sniper::http::client
