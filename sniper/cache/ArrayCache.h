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

#include <cstdint>
#include <memory>
#include <sniper/cache/FreeList.h>
#include <string>

namespace sniper::cache {

template<class T, unsigned MaxSize = 100>
class ArrayCache
{
public:
    static void clear()
    {
        instance().sized_free_list.clear();
        instance().empty_free_list.clear();
        instance().big_free_list.clear();
    }

    static T* get_raw(size_t size = 0)
    {
        if (size == 0)
            return instance().empty_free_list.pop();

        if (size > (1ull << 31ull)) {
            T* ptr = instance().big_free_list.pop();
            if (ptr->capacity() < size)
                ptr->reserve(size);

            return ptr;
        }

        size_t upper_bound = get_upper_bound(size);
        T* ptr = instance().sized_free_list.pop(get_index(size));
        if (ptr->capacity() < upper_bound)
            ptr->reserve(upper_bound);

        return ptr;
    }

    static void release(T* ptr)
    {
        if (ptr) {
            ptr->clear();

            if (ptr->capacity() == 0) {
                instance().empty_free_list.push(ptr);
            }
            else if (ptr->capacity() > (1ull << 31ull)) {
                instance().big_free_list.push(ptr);
            }
            else {
                size_t upper_bound = get_upper_bound(ptr->capacity());
                if (ptr->capacity() != upper_bound)
                    ptr->reserve(upper_bound);

                instance().sized_free_list.push(get_index(upper_bound), ptr);
            }
        }
    }


    using unique = std::unique_ptr<T, decltype(&release)>;

    static unique get_unique(size_t size = 0) { return unique(get_raw(size), &release); }

    static unique get_unique_empty() { return unique(nullptr, &release); }

    static unique make_unique(T* ptr) { return unique(ptr, &release); }

private:
    static ArrayCache& instance()
    {
        static thread_local ArrayCache s;
        return s;
    }

    static inline uint32_t get_index(uint32_t size)
    {
        if (size >= 2)
            return sizeof(uint32_t) * 8 - __builtin_clz(size - 1);
        else
            return 0;
    }

    static inline uint32_t get_upper_bound(uint32_t size)
    {
        if (size < 2)
            return size;

        return uint32_t(1) << (sizeof(uint32_t) * 8 - __builtin_clz(size - 1));
    }

    ArrayCache() = default;
    ~ArrayCache() = default;

    ArrayCache(const ArrayCache&) = delete;
    ArrayCache(const ArrayCache&&) = delete;

    ArrayCache& operator=(const ArrayCache&) = delete;
    ArrayCache& operator=(const ArrayCache&&) = delete;


    FreeListVector<T, 32, MaxSize> sized_free_list;
    FreeList<T, MaxSize> empty_free_list;
    FreeList<T, MaxSize> big_free_list;
};

using StringCache = ArrayCache<std::string, 100>;

} // namespace sniper::cache
