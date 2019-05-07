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

#include "hash.h"

namespace sniper::crypto {

void hash_sha256_2(string_view data, array<unsigned char, SHA256_DIGEST_LENGTH>& dst) noexcept
{
    if (!data.empty()) {
        array<unsigned char, SHA256_DIGEST_LENGTH> hash{};

        // First pass
        SHA256(reinterpret_cast<const unsigned char*>(data.data()), data.size(), hash.data());

        // Second pass
        SHA256(hash.data(), hash.size(), dst.data());
    }
}

} // namespace sniper
