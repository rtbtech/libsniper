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
#include <sniper/crypto/asn1.h>
#include <sniper/crypto/secp.h>
#include <sniper/file/load.h>
#include <sniper/std/check.h>
#include <sniper/strings/hex.h>
#include <sniper/strings/trim.h>
#include "Key.h"

namespace sniper::mh {

namespace {

bool build_address(unsigned char* data, size_t size, array<unsigned char, 25>& addr)
{
    if (data && size >= 65) {
        data[size - 65] = 0x04u;

        array<unsigned char, SHA256_DIGEST_LENGTH> sha_1{};
        array<unsigned char, RIPEMD160_DIGEST_LENGTH> r160{};

        SHA256(data + (size - 65), 65, sha_1.data());
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

} // namespace

Key::Key(const fs::path& p)
{
    // p = empty if sign disabled
    if (p.empty())
        return;

    if (auto hex_key = file::load_file_to_string(p); !hex_key.empty()) {
        strings::hex2bin_append(strings::trim(hex_key), _bin_privkey_full);
        check(_bin_privkey_full.size() >= 39, "Key: private key invalid size");

        _bin_privkey_min = _bin_privkey_full.substr(8, 32);
    }

    check(!_bin_privkey_min.empty(), "Key: empty private key");
    check(crypto::check_priv_key(_bin_privkey_min), "Key: wrong private key");


    // openssl pubkey
    {
        auto* data = reinterpret_cast<const unsigned char*>(_bin_privkey_full.data());
        auto* evp_key = d2i_AutoPrivateKey(nullptr, &data, _bin_privkey_full.size());
        check(evp_key, "Key: openssl error load private key");

        unsigned char* bin_pubkey = nullptr;
        auto bin_pubkey_size = i2d_PUBKEY(evp_key, &bin_pubkey);
        check(bin_pubkey && bin_pubkey_size, "Key: openssl error create pubkey");

        auto curve = crypto::parse_curve_oid(string_view(reinterpret_cast<const char*>(bin_pubkey), bin_pubkey_size));
        check(curve && curve == crypto::Curve::secp256k1, "Key: private key should use secp256k1 curve");

        _hex_pubkey = "0x";
        strings::bin2hex_append(bin_pubkey, bin_pubkey_size, _hex_pubkey);

        _hex_addr = "0x";
        array<unsigned char, 25> addr{};
        check(build_address(bin_pubkey, bin_pubkey_size, addr), "Key: cannot build address");
        strings::bin2hex_append(addr.data(), addr.size(), _hex_addr);

        EVP_PKEY_free(evp_key);
        free(bin_pubkey);
    }
}

string_view Key::hex_addr() const noexcept
{
    return _hex_addr;
}

string_view Key::hex_pubkey() const noexcept
{
    return _hex_pubkey;
}

string_view Key::raw_privkey() const noexcept
{
    return _bin_privkey_min;
}

} // namespace sniper::mh
