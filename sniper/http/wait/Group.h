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

#include <ev++.h>
#include <sniper/cache/Cache.h>
#include <sniper/std/chrono.h>
#include <sniper/std/tuple.h>

namespace sniper::http::wait {

class Group;
using GroupCache = cache::STDCache<Group>;

class Group : public intrusive_cache_unsafe_ref_counter<Group, GroupCache>, public ev::timer
{
public:
    Group() = default;
    virtual ~Group() noexcept = default;

    void clear() noexcept;
    void set_timeout() noexcept;

    [[nodiscard]] bool is_timeout() const noexcept;

    milliseconds timeout = 0ms;
    unsigned count = 0;

private:
    bool _is_timeout = false;
};

using GroupPtr = intrusive_ptr<Group>;

inline intrusive_ptr<Group> make_group()
{
    return GroupCache::get_intrusive();
}

template<class T>
inline intrusive_ptr<T> make_group(unsigned count, milliseconds timeout = 0ms)
{
    auto ptr = cache::STDCache<T>::get_intrusive();
    ptr->count = count;
    ptr->timeout = timeout;

    return ptr;
}

} // namespace sniper::http::wait
