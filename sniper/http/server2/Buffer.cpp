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

#include <sniper/log/log.h>
#include "Buffer.h"

namespace sniper::http::server2 {

Buffer::~Buffer()
{
    if (_d)
        free(_d);
}

void Buffer::clear() noexcept
{
    processed = 0;
    used = 0;
}

bool Buffer::init(size_t s) noexcept
{
    if (s != size) {
        if (_d)
            free(_d);

        _d = (char*)malloc(s);
        size = s;
    }

    return _d;
}

string_view Buffer::all_data() const noexcept
{
    if (used)
        return string_view(_d, used);

    return {};
}

string_view Buffer::curr_data() const noexcept
{
    if (used)
        return string_view(_d + processed, used - processed);

    return {};
}

bool Buffer::full() const noexcept
{
    return size == used;
}

BufferState Buffer::read(int fd) noexcept
{
    while (true) {
        if (full())
            return processed ? BufferState::Full : BufferState::Error;

        if (auto count = ::read(fd, _d + used, size - used); count > 0) {
            used += count;
        }
        else if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return BufferState::Again;
        }
        else if (count < 0 && errno == EINTR) {
            continue;
        }
        else {
            return BufferState::Error;
        }
    }
}

} // namespace sniper::http::server2
