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

#include <sniper/std/string.h>
#include <sniper/std/optional.h>

namespace sniper::crypto {

enum class Curve
{
    secp256k1, // 1.3.132.0.10
    prime256v1 // 1.2.840.10045.3.1.7
};

/* Read curve type from ASN.1 header of public key
 * (DER format)
 *
 * Returns: curve type or nullopt if not match or error
 * In:      pubkey: Public key in DER format
 */
[[nodiscard]] optional<Curve> parse_curve_oid(string_view pubkey);

} // namespace sniper::crypto
