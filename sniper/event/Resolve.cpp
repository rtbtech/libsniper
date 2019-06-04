// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

/*
 * Copyright (c) 2019, MetaHash, Oleg Romanenko (oleg@romanenko.ro)
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
#include <sniper/net/ip.h>
#include <sniper/std/check.h>
#include "Resolve.h"

namespace sniper::event {

Resolve::Resolve(event::loop_ptr loop, string_view domain, seconds period) : _loop(std::move(loop)), _domain(domain)
{
    check(_loop, "invalid argument: loop is null");
    check(period > 0s, "period should be > 0");

    _w_timer.set(*_loop);
    _w_timer.set<Resolve, &Resolve::cb_timer>(this);
    _w_timer.start(0, (double)period.count() / 1000.0);

    _w_ready.set(*_loop);
    _w_ready.set<Resolve, &Resolve::cb_ready>(this);
    _w_ready.start();
}

Resolve::~Resolve() noexcept
{
    if (_t.joinable())
        _t.join();
}

void Resolve::cb_ready(ev::async& w, int revents) noexcept
{
    try {
        if (_cb_add) {
            set<uint32_t> add;
            std::set_difference(_tmp.begin(), _tmp.end(), _ip.begin(), _ip.end(), std::inserter(add, add.begin()));

            for (auto& ip : add) {
                try {
                    _cb_add(net::ip_to_str(ip));
                }
                catch (std::exception& e) {
                    log_err(e.what());
                }
                catch (...) {
                    log_err("[Resolve::cb_ready] non std::exception occured in userc cb_add");
                }
            }
        }

        if (_cb_remove) {
            set<uint32_t> remove;
            std::set_difference(_ip.begin(), _ip.end(), _tmp.begin(), _tmp.end(),
                                std::inserter(remove, remove.begin()));

            for (auto& ip : remove) {
                try {
                    _cb_remove(net::ip_to_str(ip));
                }
                catch (std::exception& e) {
                    log_err(e.what());
                }
                catch (...) {
                    log_err("[Resolve::cb_ready] non std::exception occured in userc cb_remove");
                }
            }
        }
    }
    catch (std::exception& e) {
        log_err(e.what());
    }
    catch (...) {
        log_err("[Resolve::cb_ready] non std::exception occured");
    }

    _ip.swap(_tmp);
    _idle = true;
}

void Resolve::cb_timer(ev::timer& w, int revents) noexcept
{
    if (_idle) {
        _idle = false;

        if (_t.joinable())
            _t.join();

        _t = std::thread([this] {
            try {
                auto ip_list = net::resolve_domain(_domain);
                _tmp.clear();
                _tmp.insert(ip_list.begin(), ip_list.end());

                if (_w_ready.is_active())
                    _w_ready.send();
            }
            catch (std::exception& e) {
                log_err(e.what());
            }
            catch (...) {
                log_err("[Resolve::cb_timer] non std::exception occured");
            }
        });
    }
}

} // namespace sniper::event
