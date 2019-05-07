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

#include "secp.h"

namespace sniper::crypto {

namespace {

const auto crypto_ctx = make_secp256k1_context();

} // namespace

bool check_priv_key(string_view privkey) noexcept
{
    return check_priv_key(crypto_ctx, privkey);
}

bool secp256k1_verify(string_view sig, string_view pubkey, string_view data) noexcept
{
    pubkey_t bin_pubkey;
    if (!parse_pubkey(crypto_ctx, pubkey, bin_pubkey))
        return false;

    signature_t bin_sig;
    if (!parse_sig(crypto_ctx, sig, bin_sig))
        return false;

    signature_t bin_sig_norm;
    normalize_sig(crypto_ctx, bin_sig, bin_sig_norm);

    digest_t digest;
    if (!digest_calc(data, digest))
        return false;

    return verify(crypto_ctx, bin_sig_norm, bin_pubkey, digest);
}

bool secp256k1_sign(string_view raw_key, string_view data, unsigned char* out, size_t& out_len) noexcept
{
    digest_t digest;
    if (!digest_calc(data, digest))
        return false;

    signature_t sig_raw;
    if (!sign(crypto_ctx, sig_raw, digest, raw_key))
        return false;

    return serialize_sig(crypto_ctx, sig_raw, out, out_len);
}

} // namespace sniper::crypto
