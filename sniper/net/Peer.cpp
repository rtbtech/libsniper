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
#include <sniper/std/check.h>
#include "Peer.h"
#include "ip.h"

namespace sniper::net {

Peer::Peer(const string& ip, uint16_t port)
{
    get<uint16_t>(_peer) = port;
    check(!ip.empty(), "IP empty");
    check(ip_from_str(ip, get<uint32_t>(_peer)), "Bad ip: {}", ip);
}

Peer::Peer(uint32_t ip, uint16_t port)
{
    get<uint16_t>(_peer) = port;
    get<uint32_t>(_peer) = ip;
}

Peer::Peer(const tuple<uint32_t, uint16_t>& p) : Peer(get<uint32_t>(p), get<uint16_t>(p)) {}

Peer::Peer(const Peer& p)
{
    _hash = p._hash;
}

Peer::Peer(Peer&& p) noexcept
{
    _hash = p._hash;
    p._hash = 0;
}

Peer& Peer::operator=(const Peer& p)
{
    if (this != &p)
        _hash = p._hash;

    return *this;
}

Peer& Peer::operator=(Peer&& p) noexcept
{
    if (this != &p) {
        _hash = p._hash;
        p._hash = 0;
    }

    return *this;
}

uint32_t Peer::ip() const noexcept
{
    return get<uint32_t>(_peer);
}

uint16_t Peer::port() const noexcept
{
    return get<uint16_t>(_peer);
}

uint64_t Peer::hash() const noexcept
{
    return _hash;
}

string Peer::to_string() const
{
    return *this ? fmt::format("{}:{}", ip_to_str(get<uint32_t>(_peer)), get<uint16_t>(_peer)) : "";
}

Peer::operator bool() const noexcept
{
    return _hash;
}

} // namespace sniper::net
