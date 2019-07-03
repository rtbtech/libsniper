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
#include <sniper/std/boost_vector.h>
#include <sniper/std/string.h>

namespace sniper::net {

[[nodiscard]] bool ip_to_str(uint32_t from, string& dst);
[[nodiscard]] bool ip_to_str(uint32_t from, char* dst);

[[nodiscard]] bool ip_to_str(const sockaddr_in& from, string& dst);
[[nodiscard]] bool ip_to_str(const sockaddr_in& from, char* dst);

[[nodiscard]] string ip_to_str(uint32_t from);
[[nodiscard]] string ip_to_str(const sockaddr_in& from);

[[nodiscard]] bool ip_from_str(const string& from, uint32_t& dst);
[[nodiscard]] bool ip_from_str(const string& from, sockaddr_in& dst);
[[nodiscard]] uint32_t ip_from_str(const string& from);

[[nodiscard]] bool ip_from_sv(string_view str, uint32_t& dst);

[[nodiscard]] bool fill_addr(const string& ip, uint16_t port, sockaddr_in& dst);
void fill_addr(uint32_t ip, uint16_t port, sockaddr_in& dst);

[[nodiscard]] bool is_ip(string_view str);

/* Returns: ipv4 list for domain
 *
 */
[[nodiscard]] small_vector<uint32_t, 8> resolve_domain(const string& domain);

} // namespace sniper::net
