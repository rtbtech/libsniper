// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

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

//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <arpa/inet.h>
#include <cstdlib>
#include <netdb.h>
#include <sniper/log/log.h>
#include <sniper/net/ip.h>
#include <sniper/std/check.h>
#include "Pool.h"
#include "Request.h"
#include "Response.h"

namespace sniper::http::client {

namespace {

small_vector<uint32_t, 8> resolve_hostname(const string& hostname)
{
    SPDLOG_TRACE(__PRETTY_FUNCTION__);

    small_vector<uint32_t, 8> ip;

    hostent* h = nullptr;
    in_addr** addr_list = nullptr;

    size_t buf_len = 1024;
    char* buf = (char*)malloc(buf_len);
    if (!buf)
        return ip;

    int rc, err;
    hostent hbuf{};
    while ((rc = gethostbyname_r(hostname.c_str(), &hbuf, buf, buf_len, &h, &err)) == ERANGE) {
        /* expand buf */
        buf_len *= 2;
        char* tmp = (char*)realloc(buf, buf_len);
        if (tmp == nullptr)
            goto finish;
        else
            buf = tmp;
    }

    if (rc != 0)
        goto finish;

    if (!h || h->h_addrtype != AF_INET)
        return ip;

    addr_list = (struct in_addr**)h->h_addr_list;
    for (int i = 0; addr_list[i] != nullptr; i++)
        ip.emplace_back(addr_list[i]->s_addr);

finish:
    free(buf);

    return ip;
}

} // namespace

Pool::Pool(event::loop_ptr loop, PoolConfig config, const net::Domain& domain, bool is_proxy,
           const function<void(intrusive_ptr<Request>&&, intrusive_ptr<Response>&&)>& cb) :
    _loop(std::move(loop)),
    _config(config), _domain(domain), _is_proxy(is_proxy), _cb(cb)
{
    SPDLOG_TRACE(__PRETTY_FUNCTION__);

    if (_domain.nodes.empty()) {
        // TODO: sync, async resolv
        auto ip_list = resolve_hostname((string)_domain.name());
        for (auto& ip : ip_list)
            _domain.nodes.emplace_back(ip, _domain.port());
    }

    check(!_domain.nodes.empty(), "[Client:Pool] empty ip list");

    for (auto& node : _domain.nodes) {
        for (size_t i = 0; i < _config.conns_per_ip && _connecting.size() < _config.max_conns; i++) {
            _connecting.emplace_back(_loop, _config.connection, node, _is_proxy, _cb);
            _connecting.back().connect();
        }

        if (_connecting.size() >= _config.max_conns)
            break;
    }

    _w.set(*_loop);
    _w.set<Pool, &Pool::cb_prepare>(this);

    _out.reserve(100);
}

void Pool::send(intrusive_ptr<Request>&& req)
{
    SPDLOG_TRACE(__PRETTY_FUNCTION__);

    _out.emplace_back(std::move(req));
    if (!_w.is_active())
        _w.start();
}

void Pool::cb_prepare(ev::prepare& w, int revents)
{
    SPDLOG_TRACE(__PRETTY_FUNCTION__);

    _w.stop();

    auto err = cache::ArrayCache<vector<intrusive_ptr<Request>>>::get_unique(_out.size());
    for (auto&& r : _out) {
        if (!_send(std::move(r))) // should not invoke CB
            err->emplace_back(std::move(r));
    }
    _out.clear();

    for (auto&& r : *err) {
        try {
            _cb(std::move(r), ResponseCache::get_raw());
        }
        catch (std::exception& e) {
            log_err("[Client:Pool] Exception in user callback: {}", e.what());
        }
        catch (...) {
            log_err("[Client:Pool] Exception in user callback");
        }
    }
}

bool Pool::_send(intrusive_ptr<Request>&& req)
{
    SPDLOG_TRACE(__PRETTY_FUNCTION__);

    if (_connecting.empty() && _active.empty())
        return false;

    // Try to send req to 1st active conn
    if (!_connecting.empty() && _connecting.front().status() == ConnectionStatus::Ready) {
        _active.splice(_active.end(), _connecting, _connecting.begin());
        return _active.back().send(std::move(req));
    }

    // Look for ready conn from active
    // remove closed and connecting conns
    // Call connect on closed conns
    for (auto it = _active.begin(); it != _active.end();) {
        if (it->status() == ConnectionStatus::Ready) {
            _active.splice(_active.end(), _active, it);
            return _active.back().send(std::move(req));
        }
        else if (it->status() == ConnectionStatus::Closed || it->status() == ConnectionStatus::Connecting) {
            auto curr = it;
            ++it;
            _connecting.splice(_connecting.end(), _active, curr);

            if (_connecting.back().status() == ConnectionStatus::Closed)
                _connecting.back().connect();
            continue;
        }

        ++it;
    }

    // Look for ready conn from connecting
    // Remove ready conns
    // Run connect on closed conns
    for (auto it = _connecting.begin(); it != _connecting.end(); ++it) {
        if (it->status() == ConnectionStatus::Ready) {
            _active.splice(_active.end(), _connecting, it);
            return _active.back().send(std::move(req));
        }
        else if (it->status() == ConnectionStatus::Closed) {
            it->connect();
        }
    }

    // Try to send req to 1st connecting conn
    for (auto it = _connecting.begin(); it != _connecting.end(); ++it) {
        if (it->status() == ConnectionStatus::Connecting) {
            _connecting.splice(_connecting.end(), _connecting, it);
            return _connecting.back().send(std::move(req));
        }
    }

    return false;
}

string Pool::debug_info() const
{
    SPDLOG_TRACE(__PRETTY_FUNCTION__);

    string out;

    //    out += fmt::format("Pool {}:{}\n", _domain.name(), _domain.port());
    out += "Pool\n";
    out += fmt::format("\tConns total: {}, active: {}, connecting: {}\n", _active.size() + _connecting.size(),
                       _active.size(), _connecting.size());
    out += fmt::format("\tOut queue: {}\n", _out.size());

    out += "\tActive:\n";
    for (auto& conn : _active)
        out += conn.debug_info();

    out += "\tConnecting:\n";
    for (auto& conn : _connecting)
        out += conn.debug_info();

    return out;
}

} // namespace sniper::http::client
