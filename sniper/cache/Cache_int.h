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

#include <sniper/cache/FreeList.h>

namespace sniper::cache {

template<class T, unsigned MaxSize = 100>
class _cache
{
public:
    using value_type = T;
    using pointer = T*;

    _cache() = default;

    T* get();
    void clear();
    size_t size() const;
    size_t count_inuse() const;

    _cache(const _cache&) = delete;
    _cache(_cache&&) = delete;
    _cache& operator=(const _cache&) = delete;
    _cache& operator=(_cache&&) = delete;

protected:
    FreeList<T, MaxSize> _free_list;
};

template<class T, unsigned MaxSize = 100>
class _simple_cache final : public _cache<T, MaxSize>
{
public:
    _simple_cache() = default;
    void release(T* ptr);

    _simple_cache(const _simple_cache&) = delete;
    _simple_cache(_simple_cache&&) = delete;
    _simple_cache& operator=(const _simple_cache&) = delete;
    _simple_cache& operator=(_simple_cache&&) = delete;
};

template<class T, unsigned MaxSize = 100>
class _std_cache final : public _cache<T, MaxSize>
{
public:
    _std_cache() = default;
    void release(T* ptr);

    _std_cache(const _std_cache&) = delete;
    _std_cache(_std_cache&&) = delete;
    _std_cache& operator=(const _std_cache&) = delete;
    _std_cache& operator=(_std_cache&&) = delete;
};

template<class T, unsigned MaxSize = 100>
class _proto_cache final : public _cache<T, MaxSize>
{
public:
    _proto_cache() = default;
    void release(T* ptr);

    _proto_cache(const _proto_cache&) = delete;
    _proto_cache(_proto_cache&&) = delete;
    _proto_cache& operator=(const _proto_cache&) = delete;
    _proto_cache& operator=(_proto_cache&&) = delete;
};

// ----------------------------------------------------------------------------

template<class T, unsigned int MaxSize>
inline T* _cache<T, MaxSize>::get()
{
    return _free_list.pop();
}

template<class T, unsigned int MaxSize>
inline void _cache<T, MaxSize>::clear()
{
    _free_list.clear();
}

template<class T, unsigned int MaxSize>
inline size_t _cache<T, MaxSize>::size() const
{
    return _free_list.size();
}

template<class T, unsigned int MaxSize>
inline size_t _cache<T, MaxSize>::count_inuse() const
{
    return _free_list.count_inuse();
}

template<class T, unsigned int MaxSize>
inline void _simple_cache<T, MaxSize>::release(T* ptr)
{
    if (ptr)
        _cache<T, MaxSize>::_free_list.push(ptr);
}

template<class T, unsigned int MaxSize>
inline void _std_cache<T, MaxSize>::release(T* ptr)
{
    if (ptr) {
        ptr->clear();
        _cache<T, MaxSize>::_free_list.push(ptr);
    }
}

template<class T, unsigned int MaxSize>
inline void _proto_cache<T, MaxSize>::release(T* ptr)
{
    if (ptr) {
        ptr->Clear();
        _cache<T, MaxSize>::_free_list.push(ptr);
    }
}

} // namespace sniper::cache
