// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

/*
 * Copyright (c) 2020, MediaSniper, Oleg Romanenko (oleg@romanenko.ro)
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

#include "Group.h"

namespace sniper::event::wait {

void Group::clear() noexcept
{
    _timeout = 0ms;
    _count = 0;
    _is_timeout = false;
    _pool.reset();

    release(); // call clear from derived class
}

void Group::release() noexcept {}

bool Group::is_timeout() const noexcept
{
    return _is_timeout;
}

void Group::set_timeout() noexcept
{
    _is_timeout = true;
}

void Group::detach() noexcept
{
    stop();
    _pool.reset();
}

void Group::done()
{
    if (!_pool || is_timeout() || !_count)
        return;

    if (_count)
        _count--;

    if (!_count) {
        stop();
        _pool->done(this);
    }
}

void Group::inc(unsigned count) noexcept
{
    _count += count;
}

bool Group::is_empty() const noexcept
{
    return !_count;
}

} // namespace sniper::event::wait
