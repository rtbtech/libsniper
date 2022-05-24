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

#include <sniper/log/log.h>
#include "Sig.h"

namespace sniper::async::sig {

Sig& make(const loop_ptr& loop, int signum)
{
    auto* s = SigCache::get_raw();
    s->set(loop, signum);

    return *s;
}

Sig::Sig()
{
    w_sig.set<Sig, &Sig::cb_sig>(this);
}

Sig::~Sig() noexcept
{
    clear();
}

void Sig::then(function<void(const loop_ptr&)>&& f) noexcept
{
    cb = std::move(f);
}

void Sig::set(const loop_ptr& l, int signum)
{
    loop = l;
    w_sig.set(*loop);

    w_sig.start(signum);
}

void Sig::clear() noexcept
{
    if (loop)
        w_sig.stop();

    loop.reset();
    cb = {};
}

void Sig::cb_sig(ev::sig& w, int revents) noexcept
{
    if (cb) {
        try {
            cb(loop);
        }
        catch (std::exception& e) {
            log_err("[Prepare] Exception in user callback: {}", e.what());
        }
        catch (...) {
            log_err("[Prepare] Exception in user callback");
        }
    }
}

} // namespace sniper::async::sig
