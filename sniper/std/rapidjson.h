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
#include <sniper/std/string.h>

namespace sniper::json {

template<typename T>
[[nodiscard]] inline bool is_string(const T& json, string_view key) noexcept
{
    return json.HasMember(key) && json[key].IsString() && json[key].GetStringLength();
}

template<typename T>
[[nodiscard]] inline bool is_string(const T& json) noexcept
{
    return json.IsString() && json.GetStringLength();
}

template<typename T>
[[nodiscard]] inline bool is_num(const T& json, string_view key) noexcept
{
    return json.HasMember(key) && json[key].IsNumber();
}

template<typename T>
[[nodiscard]] inline bool is_bool(const T& json, string_view key) noexcept
{
    return json.HasMember(key) && json[key].IsBool();
}

template<typename T>
[[nodiscard]] inline bool is_obj(const T& json, string_view key) noexcept
{
    return json.HasMember(key) && json[key].IsObject();
}

template<typename T>
[[nodiscard]] inline bool is_array(const T& json, string_view key) noexcept
{
    return json.HasMember(key) && json[key].IsArray();
}

template<typename T>
[[nodiscard]] inline string_view get_sv(const T& json, string_view key) noexcept
{
    return string_view(json[key].GetString(), json[key].GetStringLength());
}

template<typename T>
[[nodiscard]] inline string_view get_sv(const T& json, string_view key, string_view default_str) noexcept
{
    if (is_string(json, key))
        return get_sv(json, key);

    return default_str;
}

template<typename T>
[[nodiscard]] inline string get_string(const T& json, string_view key) noexcept
{
    return string(json[key].GetString(), json[key].GetStringLength());
}

template<typename T>
[[nodiscard]] inline int64_t get_int64(const T& json, string_view key) noexcept
{
    return json[key].GetInt64();
}

template<typename T>
[[nodiscard]] inline int64_t get_int32(const T& json, string_view key) noexcept
{
    return json[key].GetInt();
}

template<typename T>
[[nodiscard]] inline int64_t get_int64(const T& json, string_view key, int64_t default_int) noexcept
{
    if (is_num(json, key))
        return get_int64(json, key);

    return default_int;
}

template<typename T>
[[nodiscard]] inline int32_t get_int32(const T& json, string_view key, int32_t default_int) noexcept
{
    if (is_num(json, key))
        return get_int32(json, key);

    return default_int;
}

template<typename T>
[[nodiscard]] inline bool get_bool(const T& json, string_view key) noexcept
{
    return json[key].GetBool();
}

template<typename T>
[[nodiscard]] inline bool get_bool(const T& json, string_view key, bool default_value) noexcept
{
    if (is_bool(json, key))
        return get_bool(json, key);

    return default_value;
}

} // namespace sniper::json
