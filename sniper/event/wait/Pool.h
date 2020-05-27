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
#include <sniper/std/functional.h>
#include <sniper/std/map.h>
#include <sniper/std/memory.h>
#include <sniper/std/vector.h>

namespace sniper::event::wait {

class Group;

struct Pool final : public intrusive_unsafe_ref_counter<Pool>
{
    Pool(event::loop_ptr loop);
    ~Pool();

    void add(intrusive_ptr<Group>&& wg);
    void done(Group* wg);
    void close() noexcept;

    function<void(intrusive_ptr<Group>&&)> _cb;

private:
    void cb_timeout(ev::timer& w, [[maybe_unused]] int revents) noexcept;
    void cb_done(ev::prepare& w, [[maybe_unused]] int revents) noexcept;

    event::loop_ptr _loop;
    ev::prepare _w_done;

    unordered_map<Group*, intrusive_ptr<Group>> _groups;
    vector<intrusive_ptr<Group>> _done;
};

} // namespace sniper::event::wait
