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
#include "Touch.h"

namespace sniper::event {

event::Touch::Touch(loop_ptr loop, function<void()>&& cb) : _loop(std::move(loop)), _cb(std::move(cb))
{
    check(_loop, "Touch: loop is null");

    _w.set(*_loop);
    _w.set<Touch, &Touch::cb_async>(this);
    _w.start();
}

event::Touch::~Touch() noexcept
{
    _w.stop();
}

void event::Touch::touch() noexcept
{
    if (_w.is_active())
        _w.send();
}

void Touch::cb_async(ev::async& w, int revents) noexcept
{
    if (_cb)
        _cb();
}


} // namespace sniper::event
