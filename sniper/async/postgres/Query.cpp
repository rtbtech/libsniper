/*
 * Copyright (c) 2020 - 2022, Oleg Romanenko (oleg@romanenko.ro)
 */

#include <sniper/async/postgres/Query.h>

namespace sniper::async::postgres {

intrusive_ptr<Query> Query::get_intrusive() noexcept
{
    return this;
}

Query& Query::set_id(int n) noexcept
{
    this->id = n;
    return *this;
}

int Query::get_id() const noexcept
{
    return id;
}

void Query::clear() noexcept
{
    id = 0;
}

void Query::then(function<void(const Query&)>&& f) noexcept
{
    this->cb = std::move(f);
}

} // namespace sniper::async::postgres
