/*
 * Copyright (c) 2020 - 2022, Oleg Romanenko (oleg@romanenko.ro)
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
