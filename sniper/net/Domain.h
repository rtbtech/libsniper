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

#include <sniper/net/Peer.h>
#include <sniper/std/boost_vector.h>
#include <sniper/std/string.h>
#include <sniper/std/tuple.h>

namespace sniper::net {

class Domain final
{
public:
    Domain();

    void clear() noexcept;
    void set(string_view name, uint16_t port = 80, string_view port_sv = {});

    [[nodiscard]] uint16_t port() const noexcept;
    [[nodiscard]] string_view name() const noexcept;
    [[nodiscard]] string_view port_sv() const noexcept;
    [[nodiscard]] uint64_t hash() const noexcept;

    explicit operator bool() const noexcept;

    small_vector<Peer, 8> nodes;

private:
    string _name;
    tuple<uint16_t, string> _port;
    uint64_t _hash;
};

inline bool operator==(const Domain& lhs, const Domain& rhs)
{
    return lhs.hash() == rhs.hash() && lhs.name() == rhs.name() && lhs.port() == rhs.port();
}

inline bool operator!=(const Domain& lhs, const Domain& rhs)
{
    return !(lhs == rhs);
}

} // namespace sniper::net

namespace std {

template<>
struct hash<::sniper::net::Domain>
{
    std::size_t operator()(const ::sniper::net::Domain& domain) const { return domain.hash(); }
};

} // namespace std
