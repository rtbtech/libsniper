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

#include <sniper/http/server2/Pool.h>
#include <sniper/http/server2/Request.h>
#include <sniper/http/server2/Response.h>
#include <sniper/log/log.h>
#include <sniper/std/check.h>
#include <sniper/std/string.h>
#include <sys/uio.h>
#include "Connection.h"

namespace sniper::http::server2 {

namespace {

const string_view response =
    "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nConnection: keep-alive\r\nAccess-Control-Allow-Origin: *\r\n\r\n";

constexpr uint32_t buf_size = 8192;

} // namespace

Connection::Connection()
{
    _out.set_capacity(128);
    _user.reserve(128);
}

void Connection::clear() noexcept
{
    _loop.reset();
    _pool.reset();
    _fd = -1;
    _closed = true;

    _sent = 0;
    _read = 0;
    _req_count = 0;

    _buf.reset();
    _out.clear();
    _user.clear();
}

void Connection::set(event::loop_ptr loop, intrusive_ptr<Pool> pool, const Config& config, int fd) noexcept
{
    _loop = std::move(loop);
    _pool = std::move(pool);
    _config = config;
    _fd = fd;
    _closed = false;

    _w_read.set(*_loop);
    _w_write.set(*_loop);
    _w_close.set(*_loop);
    _w_user.set(*_loop);

    _w_read.set<Connection, &Connection::cb_read>(this);
    _w_write.set<Connection, &Connection::cb_write>(this);
    _w_close.set<Connection, &Connection::cb_close>(this);
    _w_user.set<Connection, &Connection::cb_user>(this);

    _w_read.start(fd, ev::READ);
    _w_write.set(fd, ev::WRITE);
    _w_read.feed_event(0);

    _buf = make_buffer(buf_size);

    check(_pool, "Pool is nullptr");
    check(_buf, "Buffer is nullptr");
    check(_pool->_cb, "Callback not set");
}


// internal call only from async callbacks
void Connection::close() noexcept
{
    _w_read.stop();
    _w_write.stop();
    _w_close.stop();
    _w_user.stop();
    ::close(_fd);
    _closed = true;

    _out.clear();
    _user.clear();

    if (_pool) {
        auto p = _pool;
        _pool.reset();
        p->detach(this);
    }
}

// call from user
void Connection::disconnect() noexcept
{
    if (!_closed && !_w_close.is_active()) {
        _w_close.start();
        _w_close.feed_event(0);
    }
}

void Connection::cb_close(ev::prepare& w, int revents) noexcept
{
    if (!_closed)
        close();
}

void Connection::cb_read(ev::io& w, int revents) noexcept
{
    cb_read_test(w, revents);
    //    if (_closed)
    //        return;
    //
    //    while (true) {
    //        if (auto count = read(_fd, _cbuf.data() + _read, _cbuf.size() - _read); count > 0) {
    //            _read += count;
    //
    //            string_view data(_cbuf.data(), _read);
    //            while (!data.empty()) {
    //                auto pos = data.find("\r\n\r\n");
    //                if (pos != string_view::npos) {
    //                    // request ready
    //                    data.remove_prefix(pos + 4);
    //                    _req_count++;
    //                }
    //                else {
    //                    break;
    //                }
    //            }
    //
    //            _read = data.size();
    //            if (!data.empty()) {
    //                log_warn("copy {} bytes", data.size());
    //                memcpy(_cbuf.data(), data.data(), _read);
    //            }
    //        }
    //        else if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
    //            if (_req_count && !_w_write.is_active()) {
    //                _w_write.start();
    //                _w_write.feed_event(0);
    //                //                cb_write_int(_w_write);
    //            }
    //            return;
    //        }
    //        else if (count < 0 && errno == EINTR) {
    //            continue;
    //        }
    //        else {
    //            close();
    //            return;
    //        }
    //    }
}


void Connection::cb_read_test(ev::io& w, int revents) noexcept
{
    if (_closed)
        return;

    while (true) {
        if (auto state = _buf->read(_fd); state != BufferState::Error) {
            // buffer full OR eagain

            string_view data = _buf->curr_data();
            while (!data.empty()) {
                if (auto res = _pico.parse((char*)data.data(), data.size()); res == pico::ParseResult::Complete) {
                    // request ready
                    data.remove_prefix(_pico.header_size);
                    _buf->processed += (_pico.header_size);

                    auto req = make_request();
                    auto resp = make_response();

                    if (!req || !resp) {
                        close();
                        return;
                    }

                    if (_out.full())
                        _out.set_capacity(2 * _out.capacity());

                    _user.emplace_back(req, resp);
                    _out.push_back(std::move(resp));

                    _pico.clear();
                }
                else {
                    break;
                }

                //                if (auto pos = data.find("\r\n\r\n"); pos != string_view::npos) {
                //                    // request ready
                //                    data.remove_prefix(pos + 4);
                //                    _buf->processed += (pos + 4);
                //
                //                    auto req = make_request();
                //                    auto resp = make_response();
                //
                //                    if (!req || !resp) {
                //                        close();
                //                        return;
                //                    }
                //
                //                    if (_out.full())
                //                        _out.set_capacity(2 * _out.capacity());
                //
                //                    _user.emplace_back(req, resp);
                //                    _out.push_back(std::move(resp));
                //                }
                //                else {
                //                    break;
                //                }
            }

            // если не пуст
            if (_buf->used) {
                // если есть хвост
                if (!_buf->curr_data().empty()) {
                    // если это не первый запрос в буфере
                    // то переносим его в новый буфер
                    if (_buf->processed) {
                        _buf = make_buffer(buf_size, _buf->curr_data());
                    }
                    // иначе - ничего не делаем - продолжаем читать в этот же буфер
                }
                // если хвоста нет - просто обнуляем буфер
                else {
                    _buf = make_buffer(buf_size);
                }

                if (!_buf) {
                    close();
                    return;
                }
            }

            if (state == BufferState::Again) {
                if (!_user.empty() && !_w_user.is_active()) {
                    _w_user.start();
                    _w_user.feed_event(0);
                }
                return;
            }

            continue;
        }
        else {
            close();
            return;
        }
    }


    //    if (_closed)
    //        return;

    //    while (true) {
    // читаем в буфер до
    // - исчерпания буфера
    // - до еагайн
    // парсим буфер и шарим его по нескольким реквестам, если надо
    // если в буфере


    //        if (auto count = _buf->read(_fd); count > 0) {
    //if (auto count = read(_fd, _req.data() + _read, _req.size() - _read); count > 0) {
    //            _read += count;

    //            do {
    //                auto new_buf = RequestBufCache::get_unique();
    //                new_buf->init(4096); // TODO: set from config
    //                new_buf->copy(*_buf);
    //
    //                auto req = make_request();
    //                auto resp = make_response();
    //
    //                if (!req || !resp) {
    //                    close();
    //                    return;
    //                }
    //
    //                if (_out.full())
    //                    _out.set_capacity(2 * _out.capacity());
    //
    //                _user.emplace_back(req, resp);
    //                _out.push_back(std::move(resp));
    //
    //            } while (false);


    //            string_view data(_req.data(), _read);
    //            while (!data.empty()) {
    //                auto pos = data.find("\r\n\r\n");
    //                if (pos != string_view::npos) {
    //                    // request ready
    //                    data.remove_prefix(pos + 4);
    //
    //                    auto req = make_request();
    //                    auto resp = make_response();
    //
    //                    if (!req || !resp) {
    //                        close();
    //                        return;
    //                    }
    //
    //                    if (_out.full())
    //                        _out.set_capacity(2 * _out.capacity());
    //
    //                    _user.emplace_back(req, resp);
    //                    _out.push_back(std::move(resp));
    //                }
    //                else {
    //                    break;
    //                }
    //            }
    //
    //            _read = data.size();
    //            if (!data.empty()) {
    //                log_warn("copy {} bytes", data.size());
    //                memcpy(_req.data(), data.data(), _read);
    //            }
    //        }
    //        else if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
    //            if (!_user.empty() && !_w_user.is_active()) {
    //                _w_user.start();
    //                _w_user.feed_event(0);
    //            }
    //            return;
    //        }
    //        else if (count < 0 && errno == EINTR) {
    //            continue;
    //        }
    //        else {
    //            close();
    //            return;
    //        }
    //    }
}

//WriteState Connection::cb_writev_int(ev::io& w) noexcept
//{
//    while (_req_count) {
//        iovec iov[32];
//        for (unsigned i = 0; i < _req_count; i++) {
//            iov[i].iov_base = (char*)response.data() + _sent;
//            iov[i].iov_len = response.size() - _sent;
//        }
//
//        if (ssize_t count = writev(_fd, iov, (int)_req_count); count > 0) {
//            _sent = count % response.size();
//            _req_count -= count / response.size();
//
//            for (unsigned i = 0; i < count / response.size(); i++)
//                _out.pop_front();
//        }
//        else if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
//            return WriteState::Again;
//        }
//        else if (count < 0 && errno == EINTR) {
//            continue;
//        }
//        else {
//            close();
//            return WriteState::Error;
//        }
//    }
//
//    return WriteState::Stop;
//}

WriteState Connection::cb_writev_int_resp(ev::io& w) noexcept
{
    while (!_out.empty() && _out.front()->ready) {
        iovec iov[32];
        unsigned iov_count = 0;

        for (auto it = _out.begin(); iov_count < 32 && it != _out.end() && (*it)->ready; ++it) {
            iov[iov_count].iov_base = (char*)response.data() + _sent;
            iov[iov_count].iov_len = response.size() - _sent;
            iov_count++;
        }

        if (!iov_count)
            return WriteState::Stop;

        if (ssize_t count = writev(_fd, iov, (int)iov_count); count > 0) {
            _sent = count % response.size();

            for (unsigned i = 0; i < count / response.size(); i++)
                _out.pop_front();
        }
        else if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
            return WriteState::Again;
        }
        else if (count < 0 && errno == EINTR) {
            continue;
        }
        else {
            close();
            return WriteState::Error;
        }
    }

    return WriteState::Stop;
}


void Connection::cb_write(ev::io& w, int revents) noexcept
{
    if (_closed)
        return;

    if (cb_writev_int_resp(w) == WriteState::Stop)
        w.stop();
}

void Connection::cb_user(ev::prepare& w, int revents) noexcept
{
    if (_closed || _user.empty())
        return;

    auto tmp = cache::Vector<tuple<intrusive_ptr<Request>, intrusive_ptr<Response>>>::get_unique(_user.capacity());
    if (!tmp)
        return;

    tmp->swap(_user);
    auto pool = _pool;

    for (auto& [req, resp] : *tmp) {
        try {
            pool->_cb(intrusive_ptr(this), req, resp);
        }
        catch (std::exception& e) {
            log_err("[Connection] Exception in user callback: {}", e.what());
        }
        catch (...) {
            log_err("[Connection] Exception in user callback");
        }

        if (_closed || _w_close.is_active())
            return;
    }

    if (!_w_write.is_active())
        cb_writev_int_resp(_w_write);

    if (_user.empty())
        w.stop();
}

void Connection::send(const intrusive_ptr<Response>& resp) noexcept
{
    if (resp && !_closed) {
        resp->ready = true;
        if (!_w_write.is_active() && !_out.empty() && _out.front() == resp) {
            _w_write.start();
            _w_write.feed_event(0);
        }
    }
}

// call from pool close
void Connection::detach() noexcept
{
    if (!_closed) {
        _pool.reset();
        close();
    }
}

} // namespace sniper::http::server2
