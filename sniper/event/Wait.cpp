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

#include <sniper/log/log.h>
#include <sniper/std/check.h>
#include "Wait.h"

namespace sniper::event {

static constexpr seconds clean_interval = 1s;

Wait::Wait(event::loop_ptr loop) : _pool(make_intrusive_noexcept<wait::Pool>(std::move(loop)))
{
    log_trace(__PRETTY_FUNCTION__);

    check(_pool, "[Wait] pool is nullptr");
}

Wait::~Wait() noexcept
{
    log_trace(__PRETTY_FUNCTION__);

    _pool->close();
}

void Wait::add(intrusive_ptr<wait::Group> wg)
{
    log_trace(__PRETTY_FUNCTION__);

    wg->_pool = _pool;
    _pool->add(std::move(wg));
}

} // namespace sniper::event