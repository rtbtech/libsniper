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

#pragma once

#include <sniper/event/Loop.h>
#include <sniper/event/wait/Group.h>
#include <sniper/event/wait/Pool.h>

namespace sniper::event {

class Wait final
{
public:
    explicit Wait(event::loop_ptr loop);
    virtual ~Wait() noexcept;

    void add(intrusive_ptr<wait::Group> wg);

    template<typename T>
    void set_cb(T&& cb);

private:
    intrusive_ptr<wait::Pool> _pool;
};

template<typename T>
void Wait::set_cb(T&& cb)
{
    _pool->_cb = std::forward<T>(cb);
}

} // namespace sniper::event
