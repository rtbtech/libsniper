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

#include <map>
#include <unordered_map>

namespace sniper {

using std::map;
using std::multimap;
using std::unordered_map;
using std::unordered_multimap;

struct string_hash
{
    using hash_type = std::hash<std::string_view>;
    using is_transparent = void;

    std::size_t operator()(const char* str) const
    {
        return hash_type{}(str);
    }
    std::size_t operator()(std::string_view str) const
    {
        return hash_type{}(str);
    }
    std::size_t operator()(std::string const& str) const
    {
        return hash_type{}(str);
    }
};

template<typename T>
using string_map = std::unordered_map<std::string, T, string_hash, std::equal_to<>>;

} // namespace sniper
