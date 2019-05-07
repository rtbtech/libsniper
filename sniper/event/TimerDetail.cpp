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

#include <sniper/std/check.h>
#include "TimerDetail.h"

namespace sniper::event {

TimerDetail::TimerDetail(TimerType ttype, loop_ptr loop, milliseconds t, function<void()>&& cb) :
    _ttype(ttype), _loop(std::move(loop)), _t(t), _cb(std::move(cb))
{
    check(_loop, "invalid argument: loop is null");

    _w.set(*_loop);
    _w.set<TimerDetail, &TimerDetail::cb_timer>(this);

    _start();
}

TimerDetail::~TimerDetail()
{
    _w.stop();
}

void TimerDetail::start(milliseconds t)
{
    if (t > 0ms)
        _t = t;
    _start();
}

void TimerDetail::stop()
{
    _w.stop();
}

void TimerDetail::restart(milliseconds t)
{
    stop();
    start(t);
}

void TimerDetail::_start()
{
    if (_w.is_active() || _t == 0ms)
        return;

    switch (_ttype) {
        case TimerType::Once:
            _w.start((double)_t.count() / 1000.0, 0);
            break;
        case TimerType::Repeat:
            _w.start((double)_t.count() / 1000.0, (double)_t.count() / 1000.0);
            break;
        case TimerType::NowAndRepeat:
            _w.start(0, (double)_t.count() / 1000.0);
            break;
    }
}

void TimerDetail::cb_timer(ev::timer& w, int revents)
{
    if (_cb)
        _cb();

    if (_ttype == TimerType::Once)
        _w.stop();
}

} // namespace sniper::event
