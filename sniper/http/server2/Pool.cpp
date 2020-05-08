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

#include <sniper/http/Buffer.h>
#include "Pool.h"
#include "Config.h"
#include "Connection.h"
#include "Request.h"
#include "Response.h"

namespace sniper::http::server2 {

Pool::Pool(intrusive_ptr<Config> config) : _config(std::move(config))
{
    _conns.reserve(_config->max_conns);
    _free_conns.reserve(_config->max_free_conns);
}

Pool::~Pool()
{
    close();
}

// call from server accept
intrusive_ptr<Connection> Pool::get(const event::loop_ptr& loop, const intrusive_ptr<Pool>& pool) noexcept
{
    if (!_free_conns.empty()) {
        auto conn = _free_conns.back();
        _free_conns.pop_back();
        _conns.emplace(conn.get(), conn);
        return conn;
    }

    if (auto conn = make_intrusive_noexcept<Connection>(loop, pool, _config); conn) {
        _conns.emplace(conn.get(), conn);
        return conn;
    }

    return nullptr;
}

// call from Server destructor or from self destructor
void Pool::close() noexcept
{
    for (auto& e : _free_conns)
        e->detach();

    for (auto& e : _conns)
        e.first->detach();

    _free_conns.clear();
    _conns.clear();
}

// call from connection close
void Pool::disconnect(Connection* conn) noexcept
{
    if (auto it = _conns.find(conn); it != _conns.end()) {
        if (_free_conns.size() < _config->max_free_conns)
            _free_conns.emplace_back(std::move(it->second));

        _conns.erase(conn);
    }
}

} // namespace sniper::http::server2
