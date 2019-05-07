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

#include <cstring>
#include "asn1.h"

namespace sniper::crypto {

// 1.2.840.10045.2.1
const unsigned char oid_pubkey[] = {0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01};

// 1.2.840.10045.3.1.7
const unsigned char oid_prime256v1[] = {0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07};

// 1.3.132.0.10
const unsigned char oid_secp256k1[] = {0x06, 0x05, 0x2B, 0x81, 0x04, 0x00, 0x0A};

optional<Curve> parse_curve_oid(string_view raw)
{
    auto data = reinterpret_cast<const unsigned char*>(raw.data());
    auto data_size = raw.size();

    if (data_size < 4)
        return std::nullopt;

    if (data[0] == 0x30) { // SEQUENCE 1
        if (data[1] & (1u << 7u))
            return std::nullopt;

        if (data[2] == 0x30) { // SEQUENCE 2
            if (data[3] & (1u << 7u))
                return std::nullopt;

            size_t seq2_size = data[3];
            data_size -= 4;
            data += 4;

            // oid = ec_pubkey
            if (seq2_size < std::size(oid_pubkey) || data_size < std::size(oid_pubkey)
                || memcmp(data, oid_pubkey, std::size(oid_pubkey)) != 0)
                return std::nullopt;

            data_size -= std::size(oid_pubkey);
            seq2_size -= std::size(oid_pubkey);
            data += std::size(oid_pubkey);

            // test secp256k1
            if (seq2_size >= std::size(oid_secp256k1) && data_size >= std::size(oid_secp256k1)
                && memcmp(data, oid_secp256k1, std::size(oid_secp256k1)) == 0)
                return Curve::secp256k1;

            // test prime256v1
            if (seq2_size >= std::size(oid_prime256v1) && data_size >= std::size(oid_prime256v1)
                && memcmp(data, oid_prime256v1, std::size(oid_prime256v1)) == 0)
                return Curve::prime256v1;
        }
    }

    return std::nullopt;
}

} // namespace sniper::crypto
