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

#include "Sig.h"

namespace sniper::event {

Sig::Sig(loop_ptr loop, int signum, function<void(const loop_ptr& loop)>&& cb) :
    _loop(std::move(loop)), _cb(std::move(cb))
{
    _w.set(*_loop);
    _w.set<Sig, &Sig::cb_sig>(this);
    _w.start(signum);
}

Sig::~Sig() noexcept
{
    _w.stop();
}

void Sig::cb_sig(ev::sig& w, int revents) noexcept
{
    if (_cb)
        _cb(_loop);
}

} // namespace sniper::event
