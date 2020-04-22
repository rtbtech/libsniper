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

#include <sniper/http/server2/Pool.h>
#include "Connection.h"

namespace sniper::http::server2 {

void Connection::clear() noexcept {}

void Connection::send(ResponsePtr&& resp) noexcept {}

//bool Connection::accept(Pool* pool, int fd, net::Peer peer, bool ssl) noexcept
//{
//    _pool = pool;
//    return true;
//}

void Connection::attach(intrusive_ptr<Pool> pool) noexcept
{
    _pool = std::move(pool);
}

void Connection::detach() noexcept
{
    _pool.reset();
}

void Connection::close() noexcept
{
    //
    //
    //

    //    if (!_w_close.is_active()) {
    //        _w_close.start();
    //        _w_close.feed_event(0);
    //    }
    //    if (_pool) {
    //        auto* tmp = _pool;
    //        _pool = nullptr;
    //
    //        tmp->detach(this);
    //    }
}

//void Connection::cb_close(ev::prepare& w, int revents)
//{
//    ///
//    ///
//    ///
//    ///
//
//    if (_pool) {
//        auto* tmp = _pool;
//        _pool = nullptr;
//
//        tmp->deattach(this);
//    }
//}

} // namespace sniper::http::server2
