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

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <sniper/std/array.h>
#include <sniper/std/memory.h>
#include <sniper/std/string.h>
#include <sniper/std/tuple.h>

namespace sniper::crypto {

static const EVP_MD* digest_alg = EVP_sha256();
using digest_ctx_ptr = unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;
using digest_t = tuple<array<unsigned char, EVP_MAX_MD_SIZE>, unsigned>;


/* Create smart pointer to digest context
 *
 * Returns: unique_ptr to digest context (EVP_MD_CTX)
 */
[[nodiscard]] inline digest_ctx_ptr make_digest_context()
{
    return digest_ctx_ptr(EVP_MD_CTX_new(), &EVP_MD_CTX_free);
}

/* Reset digest context
 *
 * Returns: true - if successful, false - otherwise
 * In:      ctx: digest context
 */
inline bool digest_reset(const digest_ctx_ptr& ctx) noexcept
{
    return EVP_MD_CTX_reset(ctx.get()) == 1;
}

/* Init digest context with SHA256
 *
 * Returns: true - if successful, false - otherwise
 * In:      ctx: digest context
 */
[[nodiscard]] inline bool digest_init(const digest_ctx_ptr& ctx) noexcept
{
    return EVP_DigestInit_ex(ctx.get(), digest_alg, nullptr) == 1;
}

/* Update digest context with data
 *
 * Returns: true - if successful, false - otherwise
 * In:      ctx:  digest context
 *          data: bytes array
 */
[[nodiscard]] inline bool digest_update(const digest_ctx_ptr& ctx, string_view data) noexcept
{
    return EVP_DigestUpdate(ctx.get(), data.data(), data.size()) == 1;
}

/* Finalize digest context and calculate digest
 *
 * Returns: true - if successful, false - otherwise
 * In:      ctx:    digest context
 * Out:     digest: array for hash and unsigned for hash length
 */
[[nodiscard]] inline bool digest_final(const digest_ctx_ptr& ctx, digest_t& digest) noexcept
{
    return EVP_DigestFinal_ex(ctx.get(), get<0>(digest).data(), &get<1>(digest)) == 1;
}

/* Calculate digest for data
 * Wrapper for digest_init, digest_update, digest_final
 *
 * Returns: true - if successful, false - otherwise
 * In:      ctx:    digest context
 *          data:   bytes array
 * Out:     digest: array for hash and unsigned for hash length
 */
[[nodiscard]] inline bool digest_calc(const digest_ctx_ptr& ctx, string_view data, digest_t& digest) noexcept
{
    return digest_init(ctx) && digest_update(ctx, data) && digest_final(ctx, digest)
           && get<1>(digest) == SHA256_DIGEST_LENGTH;
}

/* Calculate digest for data
 * Wrapper for digest_init, digest_update, digest_final
 * Use internal thread_local digest context (so thread safe)
 *
 * Returns: true - if successful, false - otherwise
 * In:      data:   bytes array
 * Out:     digest: array for hash and unsigned for hash length
 */
[[nodiscard]] bool digest_calc(string_view data, digest_t& digest) noexcept;

} // namespace sniper::crypto
