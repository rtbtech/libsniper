/*
 * Copyright (c) 2020, RTBtech, MediaSniper, Oleg Romanenko (oleg@romanenko.ro)
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

#include "Response.h"

namespace sniper::http::server2 {

void Response::clear() noexcept {}

void Response::add_header_copy(string_view header) {}

void Response::add_header_nocopy(string_view header) {}

void Response::add_header(cache::String::unique&& header_ptr) {}

void Response::set_data_copy(string_view data) {}

void Response::set_data_nocopy(string_view data) noexcept {}

void Response::set_data(cache::String::unique&& data_ptr) noexcept {}

cache::Vector<Chunk>::unique Response::headers() noexcept
{
    return std::move(_headers);
}

Chunk Response::data() noexcept
{
    return std::move(_data);
}

} // namespace sniper::http::server2
