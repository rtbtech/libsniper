/*
 * Copyright (c) 2020 - 2022, Oleg Romanenko (oleg@romanenko.ro)
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

#include <ev++.h>
#include <sniper/std/memory.h>

namespace sniper::async {

struct Loop : public ev::dynamic_loop, intrusive_unsafe_ref_counter<Loop>
{
    explicit Loop(unsigned int flags = ev::AUTO) : ev::dynamic_loop(flags) {}
};

using loop_ptr = intrusive_ptr<Loop>;

inline loop_ptr make_loop(unsigned int flags = ev::AUTO)
{
    return make_intrusive<Loop>(flags);
}

} // namespace sniper::async
