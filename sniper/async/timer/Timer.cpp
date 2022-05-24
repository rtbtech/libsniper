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

#include <sniper/async/timer/Timer.h>
#include <sniper/log/log.h>

namespace sniper::async::timer {

Timer& make_repeat(const loop_ptr& loop, milliseconds time)
{
    auto* t = TimerCache::get_raw();
    t->set(loop, time, Type::Repeat);

    return *t;
}

Timer& make_once(const loop_ptr& loop, milliseconds time)
{
    auto* t = TimerCache::get_raw();
    t->set(loop, time, Type::Once);

    return *t;
}

Timer::Timer()
{
    w_timer.set<Timer, &Timer::cb_timer>(this);
}

Timer::~Timer() noexcept
{
    clear();
}

void Timer::then(function<void(Timer&)>&& f) noexcept
{
    cb = std::move(f);
}

void Timer::start()
{
    if (w_timer.is_active() || time == 0ms)
        return;

    switch (ttype) {
        case Type::Once:
            w_timer.start((double)time.count() / 1000.0, 0);
            break;
        case Type::Repeat:
            w_timer.start((double)time.count() / 1000.0, (double)time.count() / 1000.0);
            break;
    }
}

void Timer::set(const loop_ptr& l, milliseconds t, Type timer_type)
{
    loop = l;
    w_timer.set(*loop);

    time = t;
    if (time == 0ms)
        time = 1ms;

    ttype = timer_type;

    start();
}

void Timer::clear() noexcept
{
    if (loop)
        w_timer.stop();

    loop.reset();
    cb = {};
    time = 0ms;
}

void Timer::stop()
{
    w_timer.stop();
}

void Timer::reset(milliseconds t)
{
    w_timer.stop();

    if (t > 0ms)
        time = t;

    start();
}

void Timer::cb_timer(ev::timer& w, int revents)
{
    if (ttype == Type::Once)
        w_timer.stop();

    if (cb) {
        try {
            cb(*this);
        }
        catch (std::exception& e) {
            log_err("[Prepare] Exception in user callback: {}", e.what());
        }
        catch (...) {
            log_err("[Prepare] Exception in user callback");
        }
    }

    if (!w_timer.is_active())
        TimerCache::release(this);
}

} // namespace sniper::async::timer
