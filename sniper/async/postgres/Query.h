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

#include <sniper/cache/Cache.h>
#include <sniper/std/functional.h>
#include <sniper/std/memory.h>

namespace sniper::async::postgres {

class Query;
using QueryCache = cache::STDCache<Query>;

class Query final : public intrusive_cache_unsafe_ref_counter<Query, QueryCache>
{
public:
    intrusive_ptr<Query> get_intrusive() noexcept;

    void then(function<void(const Query& resp)>&& f) noexcept;

    Query& set_id(int id) noexcept;
    int get_id() const noexcept;

    void clear() noexcept;

private:
    int id = 0;
    function<void(const Query& resp)> cb;
};

} // namespace sniper::async::postgres
