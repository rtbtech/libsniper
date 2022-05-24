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

namespace sniper::async::prepare {


/*
 * Example:
 *
 * CMAKE: set(RTBTECH_DEPS "${RTBTECH_DEPS}" "async" "prepare")
 *
 * #include <sniper/async/Loop.h>
 * #include <sniper/async/prepare/Prepare.h>
 * #include <sniper/log/log.h>
 *
 * void test()
 * {
 *     auto loop = async::make_loop();
 *
 *     async::prepare::make(loop).then([] {
 *         log_info("From prepare");
 *     });
 *
 *     loop->run();
 * }
 */

class Prepare;
using PrepareCache = cache::STDCache<Prepare>;

class Prepare final : public intrusive_cache_unsafe_ref_counter<Prepare, PrepareCache>
{
public:
    Prepare();
    ~Prepare() noexcept;

    void then(function<void()>&& f) noexcept;

    void set(const loop_ptr& l);
    void clear() noexcept;

private:
    void cb_prepare(ev::prepare& w, [[maybe_unused]] int revents);

    loop_ptr loop;
    ev::prepare w_prepare;
    function<void()> cb;
};

Prepare& make(const loop_ptr& loop);

} // namespace sniper::async::prepare
