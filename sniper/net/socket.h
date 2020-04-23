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

#include <cstdint>
#include <sniper/net/Peer.h>
#include <sniper/std/string.h>

namespace sniper::net::socket {

[[nodiscard]] bool set_non_blocking(int fd);
[[nodiscard]] bool set_keep_alive(int fd);
[[nodiscard]] bool set_reuse_addr_and_port(int fd);

[[nodiscard]] bool bind(int fd, const string& ip, uint16_t port);
[[nodiscard]] bool bind(int fd, uint32_t ip, uint16_t port);
[[nodiscard]] bool is_connected(int fd);


namespace udp {

[[nodiscard]] int create();

} // namespace udp

namespace tcp {

[[nodiscard]] int create();

[[nodiscard]] bool set_no_delay(int fd);
[[nodiscard]] bool set_recv_buf(int fd, uint32_t size);
[[nodiscard]] bool set_send_buf(int fd, uint32_t size);
[[nodiscard]] bool get_recv_buf(int fd, uint32_t& size);
[[nodiscard]] bool get_send_buf(int fd, uint32_t& size);
[[nodiscard]] bool set_mss(int fd, uint16_t mss);
[[nodiscard]] bool get_mss(int fd, uint16_t& mss);

#ifdef _GNU_SOURCE
[[nodiscard]] bool set_enable_cork(int fd);
[[nodiscard]] bool set_disable_cork(int fd);
[[nodiscard]] bool get_rtt(int fd, uint32_t& rtt);
#endif

[[nodiscard]] int accept(int server_fd, uint32_t& ip, uint16_t& port);
[[nodiscard]] tuple<int, Peer> accept(int server_fd);

[[nodiscard]] int connect(int fd, uint32_t ip, uint16_t port);
[[nodiscard]] bool listen(int fd, int backlog);

} // namespace tcp

} // namespace sniper::net::socket
