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

#include "RequestBuf.h"

namespace sniper::http::server2 {

void RequestBuf::init(size_t size) noexcept
{
    if (head = cache::String::get_unique(size); head)
        head->resize(size);
}

void RequestBuf::clear() noexcept
{
    pico.clear();
    head.reset();
    body.reset();
}

ssize_t RequestBuf::read(int fd) noexcept
{
    return -1;
}

void RequestBuf::copy(const RequestBuf& buf) {}


} // namespace sniper::http::server2