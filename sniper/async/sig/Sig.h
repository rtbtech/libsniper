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
#include <sniper/std/functional.h>

namespace sniper::async::sig {

/*
 * Example:
 *
 * CMAKE: set(RTBTECH_DEPS "${RTBTECH_DEPS}" "async" "sig")
 *
 * #include <sniper/async/Loop.h>
 * #include <sniper/async/sig/Sig.h>
 * #include <sniper/log/log.h>
 *
 * void stop_signal(const async::loop_ptr& loop)
 * {
 *     log_warn("Stop signal. Exiting");
 *     loop->break_loop(ev::ALL);
 * }
 *
 * void test()
 * {
 *     auto loop = async::make_loop();
 *
 *     signal(SIGPIPE, SIG_IGN);
 *     async::sig::make(loop, SIGTERM).then(stop_signal);
 *     async::sig::make(loop, SIGINT).then(stop_signal);
 *
 *     loop->run();
 * }
 */

class Sig;
using SigCache = cache::STDCache<Sig>;

class Sig final : public intrusive_cache_unsafe_ref_counter<Sig, SigCache>
{
public:
    Sig();
    ~Sig() noexcept;

    void then(function<void(const loop_ptr&)>&& f) noexcept;

    void set(const loop_ptr& l, int signum);
    void clear() noexcept;

private:
    void cb_sig(ev::sig& w, [[maybe_unused]] int revents) noexcept;

    loop_ptr loop;
    ev::sig w_sig;
    function<void(const loop_ptr&)> cb;
};

Sig& make(const loop_ptr& loop, int signum);

} // namespace sniper::async::sig
