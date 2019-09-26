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

#include <openssl/ripemd.h>
#include <openssl/x509.h>
#include <sniper/crypto/secp.h>
#include <sniper/file/load.h>
#include <sniper/std/check.h>
#include <sniper/strings/hex.h>
#include <sniper/strings/trim.h>
#include "Key.h"

namespace sniper::mh {

namespace {

bool build_address(string pubkey, array<unsigned char, 25>& addr)
{
    auto* data = reinterpret_cast<unsigned char*>(pubkey.data());

    if (data && pubkey.size() >= 65) {
        data[pubkey.size() - 65] = 0x04u;

        array<unsigned char, SHA256_DIGEST_LENGTH> sha_1{};
        array<unsigned char, RIPEMD160_DIGEST_LENGTH> r160{};

        SHA256(data + (pubkey.size() - 65), 65, sha_1.data());
        RIPEMD160(sha_1.data(), SHA256_DIGEST_LENGTH, r160.data());

        array<unsigned char, RIPEMD160_DIGEST_LENGTH + 1> wide_h{};
        wide_h[0] = 0;
        for (size_t i = 0; i < RIPEMD160_DIGEST_LENGTH; i++)
            wide_h[i + 1] = r160[i];

        array<unsigned char, SHA256_DIGEST_LENGTH> hash1{};
        SHA256(wide_h.data(), RIPEMD160_DIGEST_LENGTH + 1, hash1.data());

        array<unsigned char, SHA256_DIGEST_LENGTH> hash2{};
        SHA256(hash1.data(), SHA256_DIGEST_LENGTH, hash2.data());

        uint j = 0;
        for (uint i = 0; i < wide_h.size(); i++, j++)
            addr[j] = wide_h[i];

        for (size_t i = 0; i < 4; i++, j++)
            addr[j] = hash2[i];

        return true;
    }

    return false;
}

string build_pubkey(const crypto::evp_key_ptr& evp_key)
{
    unsigned char* bin_pubkey = nullptr;
    auto bin_pubkey_size = i2d_PUBKEY(evp_key.get(), &bin_pubkey);

    if (bin_pubkey && bin_pubkey_size)
        return string((char*)bin_pubkey, bin_pubkey_size);

    return "";
}

} // namespace

Key::Key(const fs::path& p)
{
    // p = empty if sign disabled
    if (p.empty())
        return;

    if (auto hex_key = file::load_file_to_string(p); !hex_key.empty())
        strings::hex2bin_append(strings::trim(hex_key), _bin_privkey_full);

    check(_bin_privkey_full.size() >= 39, "Key: private key invalid size");

    // load openssl private key
    _evp_pkey = crypto::openssl_load_private_key(_bin_privkey_full);
    check(_evp_pkey, "Key: openssl error load private key");

    // create pubkey from privkey
    _bin_pubkey = build_pubkey(_evp_pkey);
    check(!_bin_pubkey.empty(), "Key: openssl error create pubkey");

    // check key curve
    _privkey_type = crypto::parse_curve_oid(_bin_pubkey);
    check(_privkey_type, "Key: private key should use secp256k1 or prime256v1 curve");

    // build hex pubkey
    _hex_pubkey = "0x";
    strings::bin2hex_append(_bin_pubkey, _hex_pubkey);

    // build hex addr
    _hex_addr = "0x";
    array<unsigned char, 25> addr{};
    check(build_address(_bin_pubkey, addr), "Key: cannot build address");
    strings::bin2hex_append(addr.data(), addr.size(), _hex_addr);

    if (_privkey_type == crypto::Curve::secp256k1) {
        _bin_privkey_min = _bin_privkey_full.substr(8, 32);
        check(!_bin_privkey_min.empty(), "Key: empty private key");
        check(crypto::check_priv_key(_bin_privkey_min), "Key: wrong private key");
    }
}

bool Key::sign(string_view data, unsigned char* dst, size_t& len) const noexcept
{
    if (_privkey_type == crypto::Curve::secp256k1)
        return crypto::secp256k1_sign(_bin_privkey_min, data, dst, len);
    else
        return crypto::openssl_sign(_evp_pkey, data, dst, len);
}

string_view Key::hex_addr() const noexcept
{
    return _hex_addr;
}

string_view Key::hex_pubkey() const noexcept
{
    return _hex_pubkey;
}

string_view Key::bin_pubkey() const noexcept
{
    return _bin_pubkey;
}

} // namespace sniper::mh
