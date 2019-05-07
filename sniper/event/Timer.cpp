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

#include "Timer.h"

namespace sniper::event {

TimerOnce::TimerOnce(loop_ptr loop, function<void()>&& cb) :
    TimerDetail(TimerType::Once, std::move(loop), 0ms, std::move(cb))
{}

TimerOnce::TimerOnce(loop_ptr loop, milliseconds t, function<void()>&& cb) :
    TimerDetail(TimerType::Once, std::move(loop), t, std::move(cb))
{}

TimerRepeat::TimerRepeat(loop_ptr loop, function<void()>&& cb) :
    TimerDetail(TimerType::Repeat, std::move(loop), 0ms, std::move(cb))
{}

TimerRepeat::TimerRepeat(loop_ptr loop, milliseconds t, function<void()>&& cb) :
    TimerDetail(TimerType::Repeat, std::move(loop), t, std::move(cb))
{}

TimerNowAndRepeat::TimerNowAndRepeat(loop_ptr loop, function<void()>&& cb) :
    TimerDetail(TimerType::NowAndRepeat, std::move(loop), 0ms, std::move(cb))
{}

TimerNowAndRepeat::TimerNowAndRepeat(loop_ptr loop, milliseconds t, function<void()>&& cb) :
    TimerDetail(TimerType::NowAndRepeat, std::move(loop), t, std::move(cb))
{}

} // namespace sniper::event
