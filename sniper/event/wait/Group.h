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
#include <sniper/event/wait/Pool.h>
#include <sniper/std/chrono.h>
#include <sniper/std/memory.h>
#include <sniper/std/tuple.h>

namespace sniper::event::wait {

class Group;
using GroupCache = cache::STDCache<Group>;

class Group : public intrusive_cache_unsafe_ref_counter<Group, GroupCache>, public ev::timer
{
public:
    Group() = default;
    virtual ~Group() noexcept = default;

    void clear() noexcept;
    void done();
    [[nodiscard]] bool is_timeout() const noexcept;

    intrusive_ptr<wait::Pool> _pool;

protected:
    virtual void release() noexcept; // clear method for derived class

private:
    friend struct Pool;

    template<class T>
    friend inline intrusive_ptr<T> make_group(unsigned count);

    template<class T>
    friend inline intrusive_ptr<T> make_group(unsigned count, milliseconds timeout);

    void set_timeout() noexcept;
    void detach() noexcept;

    bool _is_timeout = false;
    milliseconds _timeout = 0ms;
    unsigned _count = 0;
};

using GroupPtr = intrusive_ptr<Group>;

inline intrusive_ptr<Group> make_group()
{
    return GroupCache::get_intrusive();
}

template<class T>
inline intrusive_ptr<T> make_group(unsigned count)
{
    if (auto ptr = cache::STDCache<T>::get_intrusive(); ptr) {
        ptr->_count = count;
        return ptr;
    }

    return nullptr;
}

template<class T>
inline intrusive_ptr<T> make_group(unsigned count, milliseconds timeout)
{
    if (auto ptr = cache::STDCache<T>::get_intrusive(); ptr) {
        ptr->_count = count;
        ptr->_timeout = timeout;
        return ptr;
    }

    return nullptr;
}

} // namespace sniper::event::wait
