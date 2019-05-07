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

#include <arpa/inet.h>
#include <cstring>
#include <re2/re2.h>
#include <sniper/std/array.h>
#include "ip.h"

namespace sniper::net {

static const re2::RE2 re_ip(
    R"(^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$)");


bool ip_to_str(uint32_t from, std::string& dst)
{
    dst.clear();

    char str[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &from, str, INET_ADDRSTRLEN))
        dst.assign(str);

    return !dst.empty();
}

bool ip_to_str(uint32_t from, char* dst)
{
    dst[0] = '\0';

    return inet_ntop(AF_INET, &from, dst, INET_ADDRSTRLEN);
}

bool ip_to_str(const sockaddr_in& from, std::string& dst)
{
    return ip_to_str(from.sin_addr.s_addr, dst);
}

bool ip_to_str(const sockaddr_in& from, char* dst)
{
    return ip_to_str(from.sin_addr.s_addr, dst);
}

std::string ip_to_str(uint32_t from)
{
    if (string dst; ip_to_str(from, dst))
        return dst;

    return "";
}

std::string ip_to_str(const sockaddr_in& from)
{
    return ip_to_str(from.sin_addr.s_addr);
}

bool ip_from_str(const std::string& from, uint32_t& dst)
{
    return !from.empty() && inet_pton(AF_INET, from.c_str(), &dst) == 1;
}

bool ip_from_str(const std::string& from, sockaddr_in& dst)
{
    return ip_from_str(from, dst.sin_addr.s_addr);
}

bool fill_addr(const std::string& ip, uint16_t port, sockaddr_in& dst)
{
    memset(&dst, 0, sizeof(dst));
    dst.sin_family = AF_INET;
    dst.sin_port = htons(port);

    return ip_from_str(ip, dst);
}

bool is_ip(string_view str)
{
    return (!str.empty() && RE2::PartialMatch(re2::StringPiece(str.data(), str.size()), re_ip));
}

uint32_t ip_from_str(const std::string& from)
{
    if (uint32_t ip = 0; ip_from_str(from, ip))
        return ip;

    return 0;
}

bool ip_from_sv(string_view str, uint32_t& dst)
{
    // 255.255.255.255\0
    constexpr size_t max_ip_str_size = 15;

    size_t len = str.size();
    if (!len || len > max_ip_str_size)
        return false;

    array<char, max_ip_str_size> buf{};
    buf[str.copy(buf.data(), len)] = '\0';

    return inet_pton(AF_INET, buf.data(), &dst) == 1;
}

} // namespace sniper::net
