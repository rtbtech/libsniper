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

#include <atomic>
#include <memory>

namespace sniper::threads {

class Spinlock
{
public:
    void lock()
    {
        while (state_.exchange(Locked, std::memory_order_acquire) == Locked) {
            /* busy-wait */
        }
    }

    void unlock() { state_.store(Unlocked, std::memory_order_release); }
    [[nodiscard]] bool try_lock() { return state_.exchange(Locked, std::memory_order_acquire) != Locked; }

private:
    typedef enum
    {
        Locked,
        Unlocked
    } LockState;

    std::atomic<LockState> state_ {Unlocked};
};

} // namespace sniper::threads
