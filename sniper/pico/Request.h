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

#include <sniper/pico/common.h>
#include <sniper/std/boost_vector.h>

namespace sniper::pico {

struct Request final
{
    void clear() noexcept;
    [[nodiscard]] ParseResult parse(char* data, size_t size) noexcept;

    size_t header_size = 0;
    size_t content_length = 0;
    bool keep_alive = false;

    int minor_version = 0;
    string_view method;
    string_view path;
    string_view qs;
    string_view fragment;

    static_vector<pair_sv, MAX_HEADERS> headers;
    small_vector<pair_sv, MAX_PARAMS> params;
};


} // namespace sniper::pico
