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

namespace sniper::cache {

template<class T>
class Singleton final
{
public:
    static T& get() { return instance().item; }

    Singleton(const Singleton&) = delete;
    Singleton(const Singleton&&) = delete;

    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(const Singleton&&) = delete;

private:
    static Singleton& instance()
    {
        static Singleton<T> s;
        return s;
    }

    Singleton() = default;
    ~Singleton() = default;

    T item;
};


template<class T>
class ThreadLocalSingleton final
{
public:
    static T& get() { return instance().item; }

    ThreadLocalSingleton(const ThreadLocalSingleton&) = delete;
    ThreadLocalSingleton(const ThreadLocalSingleton&&) = delete;

    ThreadLocalSingleton& operator=(const ThreadLocalSingleton&) = delete;
    ThreadLocalSingleton& operator=(const ThreadLocalSingleton&&) = delete;

private:
    static ThreadLocalSingleton& instance()
    {
        static thread_local ThreadLocalSingleton<T> s;
        return s;
    }

    ThreadLocalSingleton() = default;
    ~ThreadLocalSingleton() = default;

    T item;
};


template<class T>
using S = Singleton<T>;

template<class T>
using TLS = ThreadLocalSingleton<T>;

} // namespace sniper::cache
