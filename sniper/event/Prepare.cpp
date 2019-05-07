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

namespace sniper::event {

Prepare::Prepare(loop_ptr loop, function<void()>&& cb) : _loop(std::move(loop)), _cb(std::move(cb))
{
    check(_loop, "Prepare: loop is null");

    _w.set(*_loop);
    _w.set<Prepare, &Prepare::cb_prepare>(this);
}

Prepare::~Prepare() noexcept
{
    _w.stop();
}

void Prepare::start() noexcept
{
    if (!_w.is_active()) {
        _w.start();
        _once = false;
    }
}

void Prepare::start_once() noexcept
{
    if (!_w.is_active()) {
        _w.start();
        _once = true;
    }
}

void Prepare::cb_prepare(ev::prepare& w, int revents)
{
    if (_cb)
        _cb();

    if (_once)
        _w.stop();
}

} // namespace sniper::event
