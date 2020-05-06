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

BufferState Buffer::read(int fd, uint32_t max_size) noexcept
{
    if (!_capacity)
        return BufferState::Error;

    while (true) {
        if (_capacity == _size)
            return BufferState::Full;

        max_size = max_size ? std::min(max_size, _capacity - _size) : _capacity - _size;

        if (auto count = ::read(fd, _data->data() + _size, max_size); count > 0) {
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

intrusive_ptr<Buffer> make_buffer(uint32_t size) noexcept
{
    if (auto dst = BufferCache::get_intrusive(); dst->reserve(size))
        return dst;

    return nullptr;
}

intrusive_ptr<Buffer> make_buffer(uint32_t size, string_view src) noexcept
{
    if (src.size() > size)
        size = src.size();

    if (auto dst = BufferCache::get_intrusive(); dst->reserve(size)) {
        if (!src.empty()) {
            memcpy(dst->_data->data(), src.data(), src.size());
            dst->_size = src.size();
        }
        return dst;
    }

    return nullptr;
}

string_view Buffer::tail(size_t processed) const noexcept
{
    if (_size > processed)
        return string_view(_data->data() + processed, _size - processed);

    return {};
}

bool Buffer::fill(string_view data) noexcept
{
    if (!data.empty() && data.size() <= (_capacity - _size)) {
        memcpy(_data->data() + _size, data.data(), data.size());
        _size += data.size();
        return true;
    }

    return false;
}

intrusive_ptr<Buffer> renew_buffer(const intrusive_ptr<Buffer>& buf, size_t& processed) noexcept
{
    // если есть хвост
    if (buf->size() > processed) {
        // если это не первый запрос в буфере
        // то переносим его в новый буфер
        if (processed) {
            auto new_buf = make_buffer(buf->capacity(), buf->tail(processed));
            processed = 0;
            return new_buf;
        }
        else if (buf->size() == buf->capacity()) {
            // Если буфер полон и там всего один запрос.
            // запрос не поместился в буфер - закрываем соединение
            return nullptr;
        }
        // иначе - ничего не делаем - продолжаем читать в этот же буфер
    }
    else { // если хвоста нет - просто обнуляем буфер
        processed = 0;
        return make_buffer(buf->capacity());
    }

    return buf;
}

} // namespace sniper::http
