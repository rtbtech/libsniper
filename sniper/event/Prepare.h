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
#include <sniper/std/functional.h>
#include <utility>

namespace sniper::event {

class Prepare final
{
public:
    explicit Prepare(loop_ptr loop, function<void()>&& cb = {});
    ~Prepare() noexcept;

    void start() noexcept;
    void start_once() noexcept;

    template<typename T>
    void set_cb(T&& cb);

private:
    void cb_prepare(ev::prepare& w, [[maybe_unused]] int revents);

    loop_ptr _loop;
    ev::prepare _w;
    bool _once = false;
    function<void()> _cb;
};

template<typename T>
void Prepare::set_cb(T&& cb)
{
    _cb = std::forward<T>(cb);
}

} // namespace sniper::event
