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

#include <boost/circular_buffer.hpp>
#include <sniper/cache/Cache.h>
#include <sniper/event/Loop.h>
#include <sniper/http/server2/Config.h>
#include <sniper/pico/Request.h>
#include <sniper/std/memory.h>
#include <sniper/std/string.h>
#include <sniper/std/tuple.h>
#include <sniper/std/vector.h>

namespace sniper::http {

struct Buffer;

} // namespace sniper::http

namespace sniper::http::server2 {

enum class WriteState
{
    Stop,
    Again,
    Error
};

struct Pool;
struct Request;
struct Response;
struct Connection;
using ConnectionCache = cache::STDCache<Connection>;

struct Connection final : public intrusive_cache_unsafe_ref_counter<Connection, ConnectionCache>
{
    Connection();

    void clear() noexcept;
    void set(event::loop_ptr loop, intrusive_ptr<Pool> pool, intrusive_ptr<Config> config, int fd) noexcept;
    void send(const intrusive_ptr<Response>& resp) noexcept;

    void detach() noexcept;
    void disconnect() noexcept;

private:
    void cb_read(ev::io& w, [[maybe_unused]] int revents) noexcept;
    void cb_write(ev::io& w, [[maybe_unused]] int revents) noexcept;
    void cb_close(ev::prepare& w, [[maybe_unused]] int revents) noexcept;
    void cb_user(ev::prepare& w, [[maybe_unused]] int revents) noexcept;
    WriteState cb_writev_int_resp(ev::io& w) noexcept;

    void close() noexcept;

    event::loop_ptr _loop;
    intrusive_ptr<Pool> _pool;
    intrusive_ptr<Config> _config;
    int _fd = -1;
    bool _closed = true;

    ev::io _w_read;
    ev::io _w_write;
    ev::prepare _w_close;
    ev::prepare _w_user;

    uint32_t _sent = 0;
    size_t _processed = 0;

    intrusive_ptr<Buffer> _buf;
    boost::circular_buffer<intrusive_ptr<Response>> _out;
    vector<tuple<intrusive_ptr<Request>, intrusive_ptr<Response>>> _user;
    pico::RequestCache::unique _pico = pico::RequestCache::get_unique_empty();
};

[[nodiscard]] intrusive_ptr<Connection> make_connection() noexcept;

[[nodiscard]] bool parse_buffer(const Config& config, const intrusive_ptr<Buffer>& buf, size_t& processed,
                                vector<tuple<intrusive_ptr<Request>, intrusive_ptr<Response>>>& user,
                                boost::circular_buffer<intrusive_ptr<Response>>& out,
                                pico::RequestCache::unique& pico) noexcept;

} // namespace sniper::http::server2
