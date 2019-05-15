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
inline bool is_string(const T& json, std::string_view str)
{
    return json.HasMember(str) && json[str].IsString() && json[str].GetStringLength();
}

template<typename T>
inline bool is_num(const T& json, std::string_view str)
{
    return json.HasMember(str) && json[str].IsNumber();
}

template<typename T>
inline bool is_obj(const T& json, std::string_view str)
{
    return json.HasMember(str) && json[str].IsObject();
}

template<typename T>
inline bool is_array(const T& json, std::string_view str)
{
    return json.HasMember(str) && json[str].IsArray();
}

template<typename T>
inline std::string_view get_string(const T& json, std::string_view str)
{
    return std::string_view(json[str].GetString(), json[str].GetStringLength());
}

template<typename T>
inline std::string_view get_string_or_empty(const T& json, std::string_view str)
{
    if (is_string(json, str))
        return get_string(json, str);

    return {};
}

template<typename T>
inline int64_t get_int64(const T& json, std::string_view str)
{
    return json[str].GetInt64();
}

template<typename T>
inline int64_t get_int64(const T& json, std::string_view str, int64_t default_int)
{
    if (is_num(json, str))
        return get_int64(json, str);

    return default_int;
}

} // namespace sniper
