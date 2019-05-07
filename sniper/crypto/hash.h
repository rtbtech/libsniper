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

#include <openssl/sha.h>
#include <sniper/std/array.h>
#include <sniper/std/string.h>

namespace sniper::crypto {

/* Calculate sha256 twice
 *
 * In:  data: bytes array
 * Out: dst:  32 bytes hash
 */
void hash_sha256_2(string_view data, array<unsigned char, SHA256_DIGEST_LENGTH>& dst) noexcept;

} // namespace sniper::crypto
