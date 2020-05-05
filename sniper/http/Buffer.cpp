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

#include "Buffer.h"

namespace sniper::http {

void Buffer::clear() noexcept
{
    _size = 0;
}

bool Buffer::reserve(size_t s) noexcept
{
    if (s != _capacity) {
        _capacity = 0;
        if (_data = cache::String::get_unique(s); _data) {
            _data->resize(s);
            _capacity = s;
        }
    }

    return _capacity;
}
size_t Buffer::capacity() const noexcept
{
    return _capacity;
}

size_t Buffer::size() const noexcept
{
    return _size;
}

BufferState Buffer::read(int fd, bool processed) noexcept
{
    if (!_capacity)
        return BufferState::Error;

    while (true) {
        if (_capacity == _size)
            return processed ? BufferState::Full : BufferState::Error;

        if (auto count = ::read(fd, _data->data() + _size, _capacity - _size); count > 0) {
            _size += count;
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

} // namespace sniper::http
