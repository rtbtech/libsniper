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

#pragma once

#include <sniper/cache/ArrayCache.h>
#include <sniper/cache/Cache.h>
#include <sniper/pico/Request.h>
#include <sniper/std/memory.h>

namespace sniper::http::server2 {

struct RequestBuf final
{
    void init(size_t size) noexcept;
    void clear() noexcept;
    ssize_t read(int fd) noexcept;
    void copy(const RequestBuf& buf);

    pico::Request pico;
    cache::String::unique head = cache::String::get_unique_empty();
    cache::String::unique body = cache::String::get_unique_empty();
};

using RequestBufCache = cache::STDCache<RequestBuf>;

} // namespace sniper::http::server2
