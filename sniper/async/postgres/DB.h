/*
 * Copyright (c) 2020 - 2022, Oleg Romanenko (oleg@romanenko.ro)
 */

#pragma once

#include <sniper/async/postgres/Query.h>
#include <sniper/std/memory.h>

namespace sniper::async::postgres {

class DB
{
public:
    Query& query();
};

} // namespace sniper::async::postgres
