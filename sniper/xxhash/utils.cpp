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

#include "utils.h"

namespace sniper::xxhash {

uint64_t xxh64(string_view data, uint64_t seed) noexcept
{
    if (data.empty())
        return seed;

    return XXH64(data.data(), data.size(), seed);
}

uint32_t xxh32(string_view data, uint32_t seed) noexcept
{
    if (data.empty())
        return seed;

    return XXH32(data.data(), data.size(), seed);
}

xxh64_state_ptr xxh64_create_state(uint64_t seed) noexcept
{
    xxh64_state_ptr state(XXH64_createState(), XXH64_freeState);
    XXH64_reset(state.get(), seed);

    return state;
}

} // namespace sniper::xxhash
