/*
 * Copyright (c) 2020, RTBtech, MediaSniper, Oleg Romanenko (oleg@romanenko.ro)
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

#include <cstdint>
#include <sniper/std/chrono.h>

namespace sniper::http::server2 {

struct Config final
{
    uint32_t recv_buf = 1024 * 1024;
    uint32_t send_buf = 1024 * 1024;

    int backlog = 10000;
    size_t max_conns = 10000;
    seconds conns_clean_interval = 5s;

//    milliseconds keep_alive_timeout = 1min;
//    milliseconds request_read_timeout = 1s;

//    size_t header_max_size = 4 * 1024;
//    size_t body_max_size = 128 * 1024;
//    size_t header_chunk_size = 1024;

    string server_name;

    bool add_server_header = false;
    bool add_date_header = false;

    // Normalizing (tolower)
    bool normalize_headers_names = false;
    bool normalize_headers_values = false;
    bool normalize_path = false;
    bool normalize_method = false;
};


} // namespace sniper::http::server
