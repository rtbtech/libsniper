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

#include <sniper/std/memory.h>
#include <sniper/std/string.h>
#include <sniper/xxhash/xxhash.h>

namespace sniper::xxhash {

constexpr uint32_t na32 = 4273230090U; // xxh32("n/a")
constexpr uint64_t na64 = 6221376636056135903ULL; // xxh64("n/a")

using xxh64_state_ptr = unique_ptr<XXH64_state_t, decltype(&XXH64_freeState)>;


inline uint64_t xxh64(string_view data, uint64_t seed = 0)
{
    if (data.empty())
        return seed;

    return XXH64(data.data(), data.size(), seed);
}

inline uint32_t xxh32(string_view data, uint32_t seed = 0)
{
    if (data.empty())
        return seed;

    return XXH32(data.data(), data.size(), seed);
}

template<typename T>
inline uint64_t xxh64_int(T data, uint64_t seed = 0)
{
    return XXH64((char*)&data, sizeof(T), seed);
}

template<typename T>
inline uint32_t xxh32_int(T data, uint32_t seed = 0)
{
    return XXH32((char*)&data, sizeof(T), seed);
}

inline xxh64_state_ptr xxh64_create_state(uint64_t seed = 0)
{
    xxh64_state_ptr state(XXH64_createState(), XXH64_freeState);
    XXH64_reset(state.get(), seed);

    return state;
}

template<typename State>
inline uint64_t xxh64_digest(const State& state)
{
    return XXH64_digest(state.get());
}

template<typename State>
inline void xxh64_update(const State& state, string_view data)
{
    XXH64_update(state.get(), data.data(), data.size());
}

template<typename State, typename T>
inline void xxh64_update_int(const State& state, T data)
{
    XXH64_update(state.get(), reinterpret_cast<char*>(&data), sizeof(T));
}

} // namespace sniper::xxhash
