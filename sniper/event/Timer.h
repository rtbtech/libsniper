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

#pragma once

#include <sniper/event/TimerDetail.h>

namespace sniper::event {

class TimerOnce final : public TimerDetail
{
public:
    explicit TimerOnce(loop_ptr loop, function<void()>&& cb = {});
    TimerOnce(loop_ptr loop, milliseconds t, function<void()>&& cb = {});
};

class TimerRepeat final : public TimerDetail
{
public:
    explicit TimerRepeat(loop_ptr loop, function<void()>&& cb = {});
    TimerRepeat(loop_ptr loop, milliseconds t, function<void()>&& cb = {});
};

class TimerNowAndRepeat final : public TimerDetail
{
public:
    explicit TimerNowAndRepeat(loop_ptr loop, function<void()>&& cb = {});
    TimerNowAndRepeat(loop_ptr loop, milliseconds t, function<void()>&& cb = {});
};

} // namespace sniper::event
