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
#include <xxhash.h>

namespace sniper::xxhash {

constexpr uint32_t na32 = 4273230090UL; // xxh32("n/a")
constexpr uint64_t na64 = 6221376636056135903ULL; // xxh64("n/a")

using xxh64_state_ptr = unique_ptr<XXH64_state_t, decltype(&XXH64_freeState)>;

[[nodiscard]] uint64_t xxh64(string_view data, uint64_t seed = 0) noexcept;
[[nodiscard]] uint32_t xxh32(string_view data, uint32_t seed = 0) noexcept;

template<typename T>
[[nodiscard]] inline uint64_t xxh64_int(T data, uint64_t seed = 0) noexcept
{
    return XXH64((char*)&data, sizeof(T), seed);
}

template<typename T>
[[nodiscard]] inline uint32_t xxh32_int(T data, uint32_t seed = 0) noexcept
{
    return XXH32((char*)&data, sizeof(T), seed);
}

[[nodiscard]] xxh64_state_ptr xxh64_create_state(uint64_t seed = 0) noexcept;

template<typename State>
[[nodiscard]] inline uint64_t xxh64_digest(const State& state) noexcept
{
    return XXH64_digest(state.get());
}

template<typename State>
inline void xxh64_update(const State& state, string_view data) noexcept
{
    XXH64_update(state.get(), data.data(), data.size());
}

template<typename State, typename T>
inline void xxh64_update_int(const State& state, T data) noexcept
{
    XXH64_update(state.get(), reinterpret_cast<char*>(&data), sizeof(T));
}

} // namespace sniper::xxhash
