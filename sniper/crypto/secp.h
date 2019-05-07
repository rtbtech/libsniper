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

#include <secp256k1.h>
#include <sniper/crypto/digest.h>
#include <sniper/std/array.h>
#include <sniper/std/memory.h>
#include <sniper/std/string.h>

namespace sniper::crypto {

using secp256k1_ctx_ptr = unique_ptr<secp256k1_context, decltype(&secp256k1_context_destroy)>;
using pubkey_t = secp256k1_pubkey;
using signature_t = secp256k1_ecdsa_signature;


/* Create smart pointer to secp256k1 context
 *
 * Returns: unique_ptr to secp256k1 context
 */
[[nodiscard]] inline secp256k1_ctx_ptr make_secp256k1_context()
{
    return secp256k1_ctx_ptr(secp256k1_context_create(SECP256K1_CONTEXT_VERIFY | SECP256K1_CONTEXT_SIGN),
                             &secp256k1_context_destroy);
}

/* Parse public key in DER format
 *
 * Returns: true - if successful, false - otherwise
 * In:      ctx:    context
 *          data:   raw public key with or without DER header. Uncompressed
 * Out:     pubkey: parsed public key
 */
[[nodiscard]] inline bool parse_pubkey(const secp256k1_ctx_ptr& ctx, string_view data, pubkey_t& pubkey) noexcept
{
    if (data.size() > 65)
        data.remove_prefix(data.size() - 65);

    return secp256k1_ec_pubkey_parse(ctx.get(), &pubkey, reinterpret_cast<const unsigned char*>(data.data()),
                                     data.size())
           == 1;
}

/* Parse ECDSA signature in DER format
 *
 * Returns: true - if successful, false - otherwise
 * In:      ctx:  context
 *          data: raw signature in DER format
 * Out:     sig:  parsed signature
 */
[[nodiscard]] inline bool parse_sig(const secp256k1_ctx_ptr& ctx, string_view data, signature_t& sig) noexcept
{
    return secp256k1_ecdsa_signature_parse_der(ctx.get(), &sig, reinterpret_cast<const unsigned char*>(data.data()),
                                               data.size())
           == 1;
}

/* Convert a signature to a normalized lower-S form.
 *
 * Returns: 1 - if sigin was not normalized, 0 - if it already was
 * In:      sig_in: signature
 * Out:     sig_out: normalized signature
 */
inline int normalize_sig(const secp256k1_ctx_ptr& ctx, const signature_t& sig_in, signature_t& out) noexcept
{
    return secp256k1_ecdsa_signature_normalize(ctx.get(), &out, &sig_in);
}

/* Verify signature
 *
 * Returns: true - if correct signature, false - otherwise
 * In:      ctx:    context
 *          sig:    signature
 *          pubkey: public key
 *          digest: digest of data
 */
[[nodiscard]] inline bool verify(const secp256k1_ctx_ptr& ctx, const signature_t& sig, const pubkey_t& pubkey,
                                 const digest_t& digest) noexcept
{
    return secp256k1_ecdsa_verify(ctx.get(), &sig, get<0>(digest).data(), &pubkey) == 1;
}

/* Verify ECDSA private key
 *
 * Returns: true - if key valid, false - otherwise
 * In:      ctx:     context
 *          privkey: raw 32 bytes of private key
 */
[[nodiscard]] inline bool check_priv_key(const secp256k1_ctx_ptr& ctx, string_view privkey) noexcept
{
    return secp256k1_ec_seckey_verify(ctx.get(), reinterpret_cast<const unsigned char*>(privkey.data())) == 1;
}

/* Sign digest
 *
 * Returns: true - if successful, false - otherwise
 * In:      ctx:     context
 *          digest:  digest of data
 *          privkey: raw 32 bytes of private key
 * Out:     sig:     signature
 */
[[nodiscard]] inline bool sign(const secp256k1_ctx_ptr& ctx, signature_t& sig, const digest_t& digest,
                               string_view privkey) noexcept
{
    return secp256k1_ecdsa_sign(ctx.get(), &sig, get<0>(digest).data(),
                                reinterpret_cast<const unsigned char*>(privkey.data()), nullptr, nullptr)
           == 1;
}

/* Serialize ECDSA signature in DER format
 *
 * Returns: true - if successful, false - otherwise
 * In:      ctx:  context
 *          sig:  signature
 * Out:     data: bytes array of signature in DER format
 *          len:  size of serialized signature in bytes
 */
[[nodiscard]] inline bool serialize_sig(const secp256k1_ctx_ptr& ctx, const signature_t& sig, unsigned char* data,
                                        size_t& len) noexcept
{
    return secp256k1_ecdsa_signature_serialize_der(ctx.get(), data, &len, &sig) == 1;
}

/* Verify ECDSA private key
 * Use internal thread_local context (so thread safe)
 *
 * Returns: true - if key valid, false - otherwise
 * In:      privkey: raw 32 bytes of private key
 */
[[nodiscard]] bool check_priv_key(string_view privkey) noexcept;

/* Verify signature
 * Use internal thread_local context (so thread safe)
 *
 * Returns: true - if correct signature, false - otherwise
 * In:      sig:    raw signature in DER format
 *          pubkey: raw public key in DER format
 *          data:   bytes array of data
 */
[[nodiscard]] bool secp256k1_verify(string_view sig, string_view pubkey, string_view data) noexcept;

/* Sign data
 * Use internal thread_local context (so thread safe)
 *
 * Returns: true - if successful, false - otherwise
 * In:      private_key: raw 32 bytes of private key
 *          data:        bytes array of data
 * Out:     out:         bytes array of signature in DER format
 *          len:         size of serialized signature in bytes
 */
[[nodiscard]] bool secp256k1_sign(string_view private_key, string_view data, unsigned char* out, size_t& out_len) noexcept;

} // namespace sniper::crypto
