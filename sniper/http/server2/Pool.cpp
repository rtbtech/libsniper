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

#include <sniper/http/server2/Connection.h>
#include <sniper/http/server2/Request.h>
#include <sniper/http/server2/Response.h>
#include "Pool.h"

namespace sniper::http::server2 {

Pool::Pool(size_t max_conns)
{
    _conns.reserve(max_conns);
}

Pool::~Pool()
{
    close();
}

// call from server accept
intrusive_ptr<Connection> Pool::get() noexcept
{
    if (auto conn = make_connection(); conn) {
        _conns.emplace(conn.get(), conn);
        return conn;
    }

    return nullptr;
}

// call from Server destructor or from self destructor
void Pool::close() noexcept
{
    for (auto& e : _conns)
        e.first->detach();

    _conns.clear();
}

// call from connection close
void Pool::detach(Connection* conn) noexcept
{
    if (auto it = _conns.find(conn); it != _conns.end())
        _conns.erase(conn);
}

} // namespace sniper::http::server2
