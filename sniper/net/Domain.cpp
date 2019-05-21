// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

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

#include <fmt/format.h>
#include <sniper/xxhash/utils.h>
#include "Domain.h"
#include "ip.h"

namespace sniper::net {

Domain::Domain() : _hash(xxhash::na64)
{
    _name.reserve(64);
    get<1>(_port).reserve(5);
}

Domain::Domain(string_view name, uint16_t port, string_view port_sv)
{
    set(name, port, port_sv);
}

void Domain::clear() noexcept
{
    _name.clear();
    get<0>(_port) = 80;
    get<1>(_port).clear();
    _hash = xxhash::na64;
    nodes.clear();
}

void Domain::set(string_view name, uint16_t port, string_view port_sv)
{
    if (name.empty() || !port)
        return;

    _name = name;
    get<uint16_t>(_port) = port;

    if (!port_sv.empty()) {
        get<string>(_port) = port_sv;
    }
    else {
        fmt::format_int fport(port);
        get<string>(_port).assign(fport.data(), fport.size());
    }

    if (uint32_t ip; is_ip(_name) && ip_from_str(_name, ip))
        nodes.emplace_back(ip, get<uint16_t>(_port));

    // hash
    auto state = xxhash::xxh64_create_state();
    xxhash::xxh64_update(state, _name);
    xxhash::xxh64_update(state, get<string>(_port));
    _hash = xxhash::xxh64_digest(state);
}

uint16_t Domain::port() const noexcept
{
    return get<uint16_t>(_port);
}

string_view Domain::name() const noexcept
{
    return _name;
}

string_view Domain::port_sv() const noexcept
{
    return get<string>(_port);
}

Domain::operator bool() const noexcept
{
    return !_name.empty() && !get<string>(_port).empty();
}

uint64_t Domain::hash() const noexcept
{
    return _hash;
}

} // namespace sniper::net
