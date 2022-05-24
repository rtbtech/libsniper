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

#include <sniper/async/Loop.h>
#include <sniper/cache/Cache.h>
#include <sniper/std/chrono.h>
#include <sniper/std/functional.h>

namespace sniper::async::timer {

/*
 * Example:
 *
 * CMAKE: set(RTBTECH_DEPS "${RTBTECH_DEPS}" "async" "timer")
 *
 * #include <sniper/async/Loop.h>
 * #include <sniper/async/timer/Timer.h>
 * #include <sniper/log/log.h>
 *
 * void test()
 * {
 *     auto loop = async::make_loop();
 *
 *     async::timer::make_repeat(loop, 2s).then([](auto& t) {
 *         log_info("From 2s repeat timer");
 *         // t.stop() // optional
 *         // t.reset(5s) // optional
 *     });
 *
 *     async::timer::make_once(loop, 1s).then([](auto& t) {
 *         log_info("From 1s once timer");
 *         // t.reset(5s) // optional
 *     });
 *
 *     loop->run();
 * }
 */

class Timer;
using TimerCache = cache::STDCache<Timer>;

enum class Type
{
    Once,
    Repeat
};

class Timer final : public intrusive_cache_unsafe_ref_counter<Timer, TimerCache>
{
public:
    Timer();
    ~Timer() noexcept;

    void then(function<void(Timer& t)>&& f) noexcept;
    void stop();
    void reset(milliseconds t = 0ms);

    void set(const loop_ptr& l, milliseconds t, Type timer_type);
    void clear() noexcept;

private:
    void start();
    void cb_timer(ev::timer& w, [[maybe_unused]] int revents);

    Type ttype;
    milliseconds time = 0ms;
    function<void(Timer& t)> cb;

    loop_ptr loop;
    ev::timer w_timer;
};

Timer& make_repeat(const loop_ptr& loop, milliseconds time);
Timer& make_once(const loop_ptr& loop, milliseconds time);

} // namespace sniper::async::timer
