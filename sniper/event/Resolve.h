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

#include <sniper/event/Loop.h>
#include <sniper/std/chrono.h>
#include <sniper/std/functional.h>
#include <sniper/std/set.h>
#include <thread>

namespace sniper::event {

class Resolve final
{
public:
    Resolve(event::loop_ptr loop, string_view domain, seconds period);
    ~Resolve() noexcept;

    template<typename T>
    void set_cb_add(T&& cb);

    template<typename T>
    void set_cb_remove(T&& cb);

private:
    void cb_timer(ev::timer& w, [[maybe_unused]] int revents) noexcept;
    void cb_ready(ev::async& w, [[maybe_unused]] int revents) noexcept;

    event::loop_ptr _loop;
    const string _domain;

    function<void(const string& ip)> _cb_add;
    function<void(const string& ip)> _cb_remove;

    ev::timer _w_timer;
    ev::async _w_ready;

    std::thread _t;
    bool _idle = true;
    set<uint32_t> _ip;
    set<uint32_t> _tmp;
};

template<typename T>
void Resolve::set_cb_add(T&& cb)
{
    _cb_add = std::forward<T>(cb);
}

template<typename T>
void Resolve::set_cb_remove(T&& cb)
{
    _cb_remove = std::forward<T>(cb);
}

} // namespace sniper::event
