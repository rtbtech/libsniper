/*
 * Copyright (c) 2018 - 2020, MetaHash, RTBtech, MediaSniper,
 * Oleg Romanenko (oleg@romanenko.ro)
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

#include <sniper/cache/Cache.h>
#include <sniper/pico/common.h>
#include <sniper/std/boost_vector.h>

namespace sniper::pico {

struct Request final
{
    void clear() noexcept;
    [[nodiscard]] ParseResult parse(string_view buf, size_t max_size, bool normalize, bool normalize_vals) noexcept;
    [[nodiscard]] ParseResult parse_head(string_view buf, bool normalize, bool normalize_vals) noexcept;

    bool head_parsed = false;
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

using RequestCache = cache::STDCache<pico::Request>;

} // namespace sniper::pico
