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

#include <openssl/ec.h>
#include <sniper/crypto/asn1.h>
#include <sniper/crypto/openssl.h>
#include <sniper/crypto/secp.h>
#include <sniper/strings/atoi.h>
#include <sniper/strings/hex.h>
#include "TX.h"
#include "varint.h"

namespace sniper::mh {

bool TX::load_raw_copy(string_view raw)
{
    get<1>(_raw) = cache::StringCache::get_unique(raw.size());
    get<1>(_raw)->assign(raw);

    return load_raw_nocopy(*get<1>(_raw));
}

bool TX::load_raw_nocopy(string_view raw)
{
    get<0>(_raw) = raw;

    size_t data_to_sign_size = 0;

    // to
    if (raw.size() < 25)
        return false;
    _to = raw.substr(0, 25);
    data_to_sign_size += 25;
    raw.remove_prefix(25);

    // value
    if (auto count = read_varint(raw, _value); count) {
        data_to_sign_size += count;
        raw.remove_prefix(count);
    }
    else
        return false;

    // fee
    if (auto count = read_varint(raw, _fee); count) {
        data_to_sign_size += count;
        raw.remove_prefix(count);
    }
    else
        return false;

    // nonce
    if (auto count = read_varint(raw, _nonce); count) {
        data_to_sign_size += count;
        raw.remove_prefix(count);
    }
    else
        return false;

    // data
    {
        uint64_t data_size = 0;
        if (auto count = read_varint(raw, data_size); count && data_size + count < raw.size()) {
            _data = raw.substr(count, data_size);
            data_to_sign_size += count + data_size;
            raw.remove_prefix(count + data_size);
        }
        else
            return false;
    }

    // sign
    {
        uint64_t sign_size = 0;
        if (auto count = read_varint(raw, sign_size); count && sign_size + count < raw.size()) {
            _sign = raw.substr(count, sign_size);
            raw.remove_prefix(count + sign_size);
        }
        else
            return false;
    }

    // pubkey
    {
        uint64_t pubkey_size = 0;
        if (auto count = read_varint(raw, pubkey_size); count && pubkey_size + count <= raw.size()) {
            _pubkey = raw.substr(count, pubkey_size);
            raw.remove_prefix(count + pubkey_size);
        }
        else
            return false;
    }

    _data_to_sign = get<0>(_raw).substr(0, data_to_sign_size);

    return true;
}

bool TX::load_hex(string_view to, string_view value, string_view fee, string_view nonce, string_view data,
                  string_view pubkey, string_view sign)
{
    if (auto res = strings::fast_atoi64(value); res)
        _value = *res;
    else if (!value.empty())
        return false;

    if (auto res = strings::fast_atoi64(nonce); res)
        _nonce = *res;
    else if (!nonce.empty())
        return false;

    if (auto res = strings::fast_atoi64(fee); res)
        _fee = *res;
    else if (!fee.empty())
        return false;

    uint64_t to_size = strings::hex2bin_size(to.size());
    uint64_t data_size = strings::hex2bin_size(data.size());
    uint64_t sign_size = strings::hex2bin_size(sign.size());
    uint64_t pubkey_size = strings::hex2bin_size(pubkey.size());

    if (!to_size || !sign_size || !pubkey_size)
        return false;

    // size of value + fee + nonce = 24 bytes max + 3 byte (varint header)
    get<1>(_raw) = cache::StringCache::get_unique(to_size + data_size + sign_size + pubkey_size + 27);
    auto& raw = *get<1>(_raw);
    raw.clear();

    // to
    if (auto pos = strings::hex2bin_append(to, raw); std::get<1>(pos) != 25)
        return false;

    // nums
    append_varint(_value, raw);
    append_varint(_fee, raw);
    append_varint(_nonce, raw);

    // data
    append_varint(data_size, raw);
    auto data_pos = strings::hex2bin_append(data, raw);

    // sign
    append_varint(sign_size, raw);
    auto sign_pos = strings::hex2bin_append(sign, raw);

    // pubkey
    append_varint(pubkey_size, raw);
    auto pubkey_pos = strings::hex2bin_append(pubkey, raw);

    _data_to_sign = string_view(raw.data(), std::get<0>(data_pos) + std::get<1>(data_pos));
    _sign = string_view(raw.data() + std::get<0>(sign_pos), std::get<1>(sign_pos));
    _pubkey = string_view(raw.data() + std::get<0>(pubkey_pos), std::get<1>(pubkey_pos));
    _data = string_view(raw.data() + std::get<0>(data_pos), std::get<1>(data_pos));

    get<0>(_raw) = *get<1>(_raw);

    return true;
}

string_view TX::raw() const noexcept
{
    return get<0>(_raw);
}

string_view TX::to() const noexcept
{
    return _to;
}

string_view TX::data_to_sign() const noexcept
{
    return _data_to_sign;
}

string_view TX::sign() const noexcept
{
    return _sign;
}

string_view TX::pubkey() const noexcept
{
    return _pubkey;
}

string_view TX::data() const noexcept
{
    return _data;
}

uint64_t TX::value() const noexcept
{
    return _value;
}

uint64_t TX::nonce() const noexcept
{
    return _nonce;
}

uint64_t TX::fee() const noexcept
{
    return _fee;
}

cache::StringCache::unique&& TX::move() noexcept
{
    if (get<1>(_raw)) {
        get<0>(_raw) = {};
        _to = {};
        _data_to_sign = {};
        _sign = {};
        _pubkey = {};
        _data = {};
        _value = 0;
        _nonce = 0;
        _fee = 0;
    }

    return std::move(get<1>(_raw));
}

bool verify(const TX& tx)
{
    if (tx.pubkey().empty() || tx.data_to_sign().empty() || tx.sign().empty())
        return false;

    if (auto curve = crypto::parse_curve_oid(tx.pubkey()); curve) {
        if (*curve == crypto::Curve::prime256v1)
            return crypto::openssl_verify(tx.sign(), tx.pubkey(), tx.data_to_sign());
        else if (*curve == crypto::Curve::secp256k1)
            return crypto::secp256k1_verify(tx.sign(), tx.pubkey(), tx.data_to_sign());
    }

    return false;
}

} // namespace sniper::mh
