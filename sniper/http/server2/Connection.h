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

#include <sniper/cache/Cache.h>
#include <sniper/event/Loop.h>
#include <sniper/http/server2/Response.h>
#include <sniper/net/Peer.h>

namespace sniper::http::server2 {

struct Pool;
struct Connection;
using ConnectionCache = cache::STDCache<Connection>;

struct Connection final : public intrusive_cache_unsafe_ref_counter<Connection, ConnectionCache>
{
    void clear() noexcept;
    void send(ResponsePtr&& resp) noexcept;

    //    bool accept(Pool* pool, int fd, net::Peer peer, bool ssl) noexcept;
    void attach(intrusive_ptr<Pool> pool) noexcept;
    void detach() noexcept;
    void close() noexcept;

    intrusive_ptr<Pool> _pool;

private:
    //    void cb_close(ev::prepare& w, [[maybe_unused]] int revents);
    //
    //    ev::prepare _w_close;
};

using ConnectionPtr = intrusive_ptr<Connection>;

inline ConnectionPtr make_connection() noexcept
{
    return ConnectionCache::get_intrusive();
}


} // namespace sniper::http::server2
