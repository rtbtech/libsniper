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

#include <sniper/event/Loop.h>
#include <sniper/std/functional.h>
#include <utility>

namespace sniper::event {

class Touch final
{
public:
    explicit Touch(loop_ptr loop, function<void()>&& cb = {});
    ~Touch() noexcept;

    void touch() noexcept;

    template<typename T>
    void set_cb(T&& cb);

private:
    void cb_async(ev::async& w, [[maybe_unused]] int revents) noexcept;

    loop_ptr _loop;
    ev::async _w;
    function<void()> _cb;
};

template<typename T>
void Touch::set_cb(T&& cb)
{
    _cb = std::forward<T>(cb);
}

} // namespace sniper::event
