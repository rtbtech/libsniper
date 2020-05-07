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
#include <sniper/std/memory.h>
#include <sniper/std/string.h>

namespace sniper::http::server2 {

struct Config final : public intrusive_unsafe_ref_counter<Config>
{
    uint32_t recv_buf = 1024 * 1024;
    uint32_t send_buf = 1024 * 1024;

    int backlog = 10000;
    size_t max_conns = 10000;

    //    milliseconds keep_alive_timeout = 1min;
    //    milliseconds request_read_timeout = 1s;

    uint32_t buffer_size = 1024 * 1024;
    uint32_t buffer_renew_threshold = 10; // percent
    uint32_t request_max_size = 128 * 1024;

    string server_name = "libsniper";

    bool add_server_header = false;
    bool add_date_header = false;

    // Normalizing (tolower)
    bool normalize = false; // method and headers names
    bool normalize_other = false; // path, headers values
};

inline intrusive_ptr<Config> make_config() noexcept
{
    return make_intrusive<Config>();
}

} // namespace sniper::http::server2
