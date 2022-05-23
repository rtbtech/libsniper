/*
 * Copyright (c) 2020 - 2022, Oleg Romanenko (oleg@romanenko.ro)
 */

#include <sniper/async/postgres/DB.h>

namespace sniper::async::postgres {

Query& postgres::DB::query()
{
    return *(new Query());
}

} // namespace sniper::async::postgres
