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
#include <sniper/std/chrono.h>
#include <sniper/std/functional.h>
#include <utility>

namespace sniper::event {

enum class TimerType
{
    Once,
    Repeat,
    NowAndRepeat
};

class TimerDetail
{
public:
    TimerDetail(TimerType ttype, loop_ptr loop, milliseconds t, function<void()>&& cb);
    ~TimerDetail();

    void start(milliseconds t = 0ms);

    template<typename T>
    void start(T&& cb);

    template<typename T>
    void start(milliseconds t, T&& cb);

    void stop();
    void restart(milliseconds t = 0ms);

    template<typename T>
    void set_cb(T&& cb);


private:
    void _start();
    void cb_timer(ev::timer& w, [[maybe_unused]] int revents);

    TimerType _ttype;
    loop_ptr _loop;
    milliseconds _t = 0ms;
    function<void()> _cb;
    ev::timer _w;
};

template<typename T>
void TimerDetail::start(T&& cb)
{
    _cb = std::forward<T>(cb);
    _start();
}

template<typename T>
void TimerDetail::start(milliseconds t, T&& cb)
{
    if (t > 0ms)
        _t = t;
    _cb = std::forward<T>(cb);
    _start();
}

template<typename T>
void TimerDetail::set_cb(T&& cb)
{
    _cb = std::forward<T>(cb);
}

} // namespace sniper::event
