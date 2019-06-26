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

#include <rapidjson/document.h>
#include <string_view>

namespace sniper {

template<typename T>
inline bool is_string(const T& json, std::string_view key)
{
    return json.HasMember(key) && json[key].IsString() && json[key].GetStringLength();
}

template<typename T>
inline bool is_num(const T& json, std::string_view key)
{
    return json.HasMember(key) && json[key].IsNumber();
}

template<typename T>
inline bool is_obj(const T& json, std::string_view key)
{
    return json.HasMember(key) && json[key].IsObject();
}

template<typename T>
inline bool is_array(const T& json, std::string_view key)
{
    return json.HasMember(key) && json[key].IsArray();
}

template<typename T>
inline std::string_view get_string(const T& json, std::string_view key)
{
    return std::string_view(json[key].GetString(), json[key].GetStringLength());
}

template<typename T>
inline std::string_view get_string_or_empty(const T& json, std::string_view key)
{
    if (is_string(json, key))
        return get_string(json, key);

    return {};
}

template<typename T>
inline int64_t get_int64(const T& json, std::string_view key)
{
    return json[key].GetInt64();
}

template<typename T>
inline int64_t get_int64(const T& json, std::string_view key, int64_t default_int)
{
    if (is_num(json, key))
        return get_int64(json, key);

    return default_int;
}

} // namespace sniper
