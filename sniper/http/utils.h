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

#include <sniper/http/client/Response.h>
#include <sniper/std/chrono.h>

namespace sniper::http {

[[nodiscard]] intrusive_ptr<client::Response> sync_get(string_view url, milliseconds timeout = 1s) noexcept;
[[nodiscard]] intrusive_ptr<client::Response> sync_head(string_view url, milliseconds timeout = 1s) noexcept;
[[nodiscard]] intrusive_ptr<client::Response> sync_post(string_view url, string_view data,
                                                        milliseconds timeout = 1s) noexcept;
[[nodiscard]] intrusive_ptr<client::Response> sync_put(string_view url, string_view data,
                                                       milliseconds timeout = 1s) noexcept;

} // namespace sniper::http
