/*
 * Copyright (c) 2018 - 2020, MetaHash, RTBtech, Oleg Romanenko (oleg@romanenko.ro)
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

#include <sniper/cache/Cache_int.h>
#include <sniper/std/memory.h>

namespace sniper::cache {

template<class CacheInt>
class BaseCache
{
public:
    [[nodiscard]] static typename CacheInt::pointer get_raw() noexcept;
    static void release(typename CacheInt::pointer ptr) noexcept;

    using shared = shared_ptr<typename CacheInt::value_type>;
    using local = local_ptr<typename CacheInt::value_type>;
    using unique = unique_ptr<typename CacheInt::value_type, decltype(&release)>;
    using intrusive = intrusive_ptr<typename CacheInt::value_type>;

    [[nodiscard]] static unique get_unique() noexcept;
    [[nodiscard]] static unique get_unique_empty() noexcept;
    [[nodiscard]] static shared get_shared() noexcept;
    [[nodiscard]] static local get_local() noexcept;
    [[nodiscard]] static intrusive get_intrusive() noexcept;

    [[nodiscard]] static unique make_unique(typename CacheInt::pointer ptr) noexcept;
    [[nodiscard]] static shared make_shared(typename CacheInt::pointer ptr) noexcept;
    [[nodiscard]] static local make_local(typename CacheInt::pointer ptr) noexcept;

    static void clear() noexcept;
    [[nodiscard]] static size_t size() noexcept;

    BaseCache(const BaseCache&) = delete;
    BaseCache(BaseCache&&) = delete;
    BaseCache& operator=(const BaseCache&) = delete;
    BaseCache& operator=(BaseCache&&) = delete;

private:
    static CacheInt& instance()
    {
        static thread_local CacheInt s;
        return s;
    }
};

// ----------------------------------------------------------------------------

template<class T, unsigned MaxSize = 100>
using Cache = BaseCache<_simple_cache<T, MaxSize>>;

template<class T, unsigned MaxSize = 100>
using STDCache = BaseCache<_std_cache<T, MaxSize>>;

template<class T, unsigned MaxSize = 100>
using ProtoCache = BaseCache<_proto_cache<T, MaxSize>>;

// ----------------------------------------------------------------------------

template<class CacheInt>
inline void BaseCache<CacheInt>::clear() noexcept
{
    instance().clear();
}

template<class CacheInt>
inline typename CacheInt::pointer BaseCache<CacheInt>::get_raw() noexcept
{
    return instance().get();
}

template<class CacheInt>
inline void BaseCache<CacheInt>::release(typename CacheInt::pointer ptr) noexcept
{
    instance().release(ptr);
}

template<class CacheInt>
inline typename BaseCache<CacheInt>::unique BaseCache<CacheInt>::get_unique() noexcept
{
    return unique(get_raw(), &release);
}

template<class CacheInt>
inline typename BaseCache<CacheInt>::unique BaseCache<CacheInt>::get_unique_empty() noexcept
{
    return unique(nullptr, &release);
}

template<class CacheInt>
inline typename BaseCache<CacheInt>::unique BaseCache<CacheInt>::make_unique(typename CacheInt::pointer ptr) noexcept
{
    return unique(ptr, &release);
}

template<class CacheInt>
inline typename BaseCache<CacheInt>::shared BaseCache<CacheInt>::get_shared() noexcept
{
    return shared(get_raw(), &release);
}

template<class CacheInt>
inline typename BaseCache<CacheInt>::local BaseCache<CacheInt>::get_local() noexcept
{
    return local(get_raw(), &release);
}

template<class CacheInt>
inline typename BaseCache<CacheInt>::intrusive BaseCache<CacheInt>::get_intrusive() noexcept
{
    return intrusive_ptr(get_raw());
}

template<class CacheInt>
inline typename BaseCache<CacheInt>::shared BaseCache<CacheInt>::make_shared(typename CacheInt::pointer ptr) noexcept
{
    return shared(ptr, &release);
}

template<class CacheInt>
inline typename BaseCache<CacheInt>::local BaseCache<CacheInt>::make_local(typename CacheInt::pointer ptr) noexcept
{
    return local(ptr, &release);
}

template<class CacheInt>
inline size_t BaseCache<CacheInt>::size() noexcept
{
    return instance().size();
}

} // namespace sniper::cache
