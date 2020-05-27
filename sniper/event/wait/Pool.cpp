// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

/*
 * Copyright (c) 2020, MediaSniper, Oleg Romanenko (oleg@romanenko.ro)
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

#include <sniper/cache/ArrayCache.h>
#include <sniper/log/log.h>
#include <sniper/std/check.h>
#include "Pool.h"
#include "Group.h"

namespace sniper::event::wait {

Pool::Pool(event::loop_ptr loop) : _loop(std::move(loop))
{
    _w_done.set(*_loop);
    _w_done.set<Pool, &Pool::cb_done>(this);
}

Pool::~Pool()
{
    close();
}

void Pool::close() noexcept
{
    _w_done.stop();

    for (auto& wg : _groups)
        wg.second->detach();

    _groups.clear();
    _done.clear();
}

void Pool::add(intrusive_ptr<Group>&& wg)
{
    if (!wg || wg->_timeout == 0ms || !wg->_count) {
        wg->clear();
        return;
    }

    wg->set(*_loop);
    wg->set<Pool, &Pool::cb_timeout>(this);
    if (wg->_timeout > 0ms)
        wg->start((double)wg->_timeout.count() / 1000.0);

    _groups.emplace(wg.get(), std::move(wg));
}

void Pool::done(Group* wg)
{
    check(wg, "[Wait] wait group is nullptr");

    if (auto it = _groups.find(wg); it != _groups.end()) {
        _done.emplace_back(std::move(it->second));
        _groups.erase(it);

        if (!_w_done.is_active()) {
            _w_done.start();
            _w_done.feed_event(0);
        }
    }
}

void Pool::cb_timeout(ev::timer& w, int revents) noexcept
{
    log_trace(__PRETTY_FUNCTION__);

    w.stop();

    auto* wg = static_cast<wait::Group*>(&w);
    wg->set_timeout();

    if (auto it = _groups.find(wg); it != _groups.end()) {
        _done.emplace_back(std::move(it->second));
        _groups.erase(it);

        if (!_w_done.is_active()) {
            _w_done.start();
            _w_done.feed_event(0);
        }
    }
}

void Pool::cb_done(ev::prepare& w, int revents) noexcept
{
    log_trace(__PRETTY_FUNCTION__);

    w.stop();

    if (!_cb) {
        _done.clear();
        return;
    }

    auto tmp = cache::ArrayCache<vector<intrusive_ptr<wait::Group>>>::get_unique(_done.capacity());
    tmp->swap(_done);

    for (auto&& wg : *tmp) {
        try {
            _cb(std::move(wg));
        }
        catch (std::exception& e) {
            log_err("[Wait] Exception in user callback: {}", e.what());
        }
        catch (...) {
            log_err("[Wait] Exception in user callback");
        }
    }
}


} // namespace sniper::event::wait
