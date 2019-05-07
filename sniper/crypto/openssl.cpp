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

#include <sniper/crypto/digest.h>
#include "openssl.h"

namespace sniper::crypto {

bool openssl_verify(string_view sig, string_view pubkey, string_view data)
{
    auto evp_key = crypto::openssl_load_public_key(pubkey);
    if (!evp_key)
        return false;

    auto* ec_key = EVP_PKEY_get0_EC_KEY(evp_key.get());
    if (!ec_key)
        return false;

    crypto::digest_t digest;
    if (!crypto::digest_calc(data, digest))
        return false;

    return ECDSA_verify(0, get<0>(digest).data(), get<1>(digest), reinterpret_cast<const unsigned char*>(sig.data()),
                        sig.size(), ec_key)
           == 1;
}

} // namespace sniper::crypto
