// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

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

#include <sniper/std/check.h>
#include "Prepare.h"

namespace sniper::async::prepare {

Prepare& make(const loop_ptr& loop)
{
    auto* p = PrepareCache::get_raw();
    p->set(loop);

    return *p;
}

Prepare::Prepare()
{
    w_prepare.set<Prepare, &Prepare::cb_prepare>(this);
}

Prepare::~Prepare() noexcept
{
    clear();
}

void Prepare::then(function<void()>&& f) noexcept
{
    cb = std::move(f);
}

void Prepare::set(const loop_ptr& l)
{
    loop = l;
    w_prepare.set(*loop);

    if (!w_prepare.is_active()) {
        w_prepare.start();
        w_prepare.feed_event(0);
    }
}

void Prepare::clear() noexcept
{
    if (loop)
        w_prepare.stop();

    loop.reset();
    cb = {};
}

void Prepare::cb_prepare(ev::prepare& w, int revents)
{
    w.stop();

    if (cb) {
        try {
            cb();
        }
        catch (std::exception& e) {
            log_err("[Prepare] Exception in user callback: {}", e.what());
        }
        catch (...) {
            log_err("[Prepare] Exception in user callback");
        }
    }

    if (!w.is_active())
        PrepareCache::release(this);
}

} // namespace sniper::async::prepare
