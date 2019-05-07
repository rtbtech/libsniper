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

#include <utility>
#include <vector>

namespace sniper::cache {

template<class T, unsigned MaxSize = 100>
class FreeList
{
public:
    FreeList() { free_list.reserve(MaxSize); }

    ~FreeList()
    {
        for (auto* e : free_list)
            delete e;
    }

    template<typename... _Args>
    T* pop(_Args&&... __args)
    {
        T* e = nullptr;
        if (free_list.empty()) {
            e = new T(std::forward<_Args>(__args)...);
        }
        else {
            e = free_list.back();
            free_list.pop_back();
        }

//        _inuse++;
        return e;
    }

    void push(T* e)
    {
        if (free_list.size() >= MaxSize)
            delete e;
        else
            free_list.push_back(e);

//        if (_inuse)
//            _inuse--;
    }

    size_t size() const { return free_list.size(); }
    size_t count_inuse() const { return _inuse; }

    void clear()
    {
        for (auto* e : free_list)
            delete e;

        free_list.clear();
    }

private:
    size_t _inuse = 0;
    std::vector<T*> free_list;
};


template<class T, unsigned N, unsigned MaxSize = 100>
class FreeListVector
{
public:
    FreeListVector() { free_list_v.resize(N); }

    template<typename... _Args>
    T* pop(size_t index, _Args&&... __args)
    {
        if (free_list_v[index].size())
            return free_list_v[index].pop(std::forward<_Args>(__args)...);

        for (size_t i = index + 1; i < N; i++) {
            if (free_list_v[i].size())
                return free_list_v[i].pop(std::forward<_Args>(__args)...);
        }

        return free_list_v[index].pop(std::forward<_Args>(__args)...);
    }

    void push(size_t index, T* e) { free_list_v[index].push(e); }

    size_t size() const { return free_list_v.size(); }

    void clear()
    {
        for (auto& f : free_list_v)
            f.clear();
    }

private:
    std::vector<FreeList<T, MaxSize>> free_list_v;
};

} // namespace sniper::cache
