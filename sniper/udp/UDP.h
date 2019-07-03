/*
 * Copyright (c) 2019, MetaHash, Oleg Romanenko (oleg@romanenko.ro)
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

#include <sniper/cache/ArrayCache.h>
#include <sniper/event/Loop.h>
#include <sniper/net/Peer.h>
#include <sniper/std/deque.h>
#include <sniper/std/functional.h>
#include <sniper/std/list.h>
#include <sniper/std/string.h>
#include <sniper/std/variant.h>

namespace sniper::udp {

class UDP final
{
public:
    explicit UDP(event::loop_ptr loop);

    [[nodiscard]] bool bind(const string& ip, uint16_t port) noexcept;
    [[nodiscard]] bool send_copy(net::Peer peer, string_view data);
    [[nodiscard]] bool send_nocopy(net::Peer peer, string_view data);

    template<typename T>
    void set_cb(T&& cb);

private:
    void cb_read(ev::io& w, [[maybe_unused]] int revents) noexcept;
    void cb_write(ev::io& w, [[maybe_unused]] int revents) noexcept;

    event::loop_ptr _loop;
    function<void(net::Peer, string_view)> _cb;

    ev::io _w_read;
    ev::io _w_write;

    deque<tuple<net::Peer, variant<string_view, cache::StringCache::unique>>> _out;
};


template<typename T>
void UDP::set_cb(T&& cb)
{
    _cb = std::forward<T>(cb);
}

} // namespace sniper::udp
