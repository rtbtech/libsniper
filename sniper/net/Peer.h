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

#include <netinet/in.h>
#include <sniper/std/string.h>
#include <sniper/std/tuple.h>

namespace sniper::net {

class Peer final
{
public:
    Peer() = default;
    Peer(const string& ip, uint16_t port);
    Peer(uint32_t ip, uint16_t port);
    explicit Peer(tuple<uint32_t, uint16_t> p);
    explicit Peer(const sockaddr_in& sa);
    Peer(const Peer& p);
    Peer(Peer&& p) noexcept;

    [[nodiscard]] uint32_t ip() const noexcept;
    [[nodiscard]] uint16_t port() const noexcept;
    [[nodiscard]] uint64_t hash() const noexcept;

    [[nodiscard]] string to_string() const;

    explicit operator bool() const noexcept;

    Peer& operator=(const Peer& p);
    Peer& operator=(Peer&& p) noexcept;

private:
    union
    {
        tuple<uint32_t, uint16_t> _peer{0, 0};
        uint64_t _hash;
    };
};

inline bool operator==(const Peer& lhs, const Peer& rhs)
{
    return lhs.hash() == rhs.hash();
}

inline bool operator!=(const Peer& lhs, const Peer& rhs)
{
    return !(lhs == rhs);
}

} // namespace sniper::net

namespace std {

template<>
struct hash<::sniper::net::Peer>
{
    std::size_t operator()(const ::sniper::net::Peer& host) const { return host.hash(); }
};

} // namespace std
