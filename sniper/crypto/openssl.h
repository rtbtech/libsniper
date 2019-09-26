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

#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <sniper/std/memory.h>
#include <sniper/std/string.h>
#include <sniper/std/tuple.h>

namespace sniper::crypto {

using evp_key_ptr = unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;


/* Load public key in DER format
 *
 * Returns: unique_ptr to EVP_PKEY
 * In:      raw public key in der format
 */
[[nodiscard]] inline evp_key_ptr openssl_load_public_key(string_view raw) noexcept
{
    auto* data = reinterpret_cast<const unsigned char*>(raw.data());
    return evp_key_ptr(d2i_PUBKEY(nullptr, &data, raw.size()), &EVP_PKEY_free);
}

/* Load private key
 *
 * Returns: unique_ptr to EVP_PKEY
 * In:      raw private key
 */
[[nodiscard]] inline evp_key_ptr openssl_load_private_key(string_view raw) noexcept
{
    auto* data = reinterpret_cast<const unsigned char*>(raw.data());
    return evp_key_ptr(d2i_AutoPrivateKey(nullptr, &data, raw.size()), &EVP_PKEY_free);
}

/* Veryfy signature of data
 *
 * Returns: true if correct signature, false otherwise
 * In:      sig:    raw ECDSA signature in DER format
 *          pubkey: raw public key in DER format
 *          data:   bytes array
 */
[[nodiscard]] bool openssl_verify(string_view sig, string_view pubkey, string_view data);

/* Sign data
 *
 * Returns: true if sign ok, false otherwise
 * In:      evp_key openssl private key
 *          data:   bytes array
 * Out:     sig:    raw ECDSA signature in DER format
 *          len:    length of signature
 */
[[nodiscard]] bool openssl_sign(const evp_key_ptr& evp_key, string_view data, unsigned char* sig, size_t& len);

} // namespace sniper::crypto
