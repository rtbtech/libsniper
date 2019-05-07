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

#include <sniper/cache/ArrayCache.h>
#include <sniper/std/string.h>
#include <sniper/std/tuple.h>

namespace sniper::mh {

class TX final
{
public:
    // set TX from raw binary data and parse it
    bool load_raw_copy(string_view raw);
    bool load_raw_nocopy(string_view raw);

    // set TX from hex fields
    bool load_hex(string_view to, string_view value, string_view fee, string_view nonce, string_view data,
                  string_view pubkey, string_view sign);

    string_view raw() const noexcept;
    string_view to() const noexcept;
    string_view data_to_sign() const noexcept;
    string_view sign() const noexcept;
    string_view pubkey() const noexcept;
    string_view data() const noexcept;

    uint64_t value() const noexcept;
    uint64_t nonce() const noexcept;
    uint64_t fee() const noexcept;

    cache::StringCache::unique&& move() noexcept;

private:
    tuple<string_view, cache::StringCache::unique> _raw{""sv, cache::StringCache::get_unique_empty()};

    string_view _to;
    string_view _data_to_sign;
    string_view _sign;
    string_view _pubkey;
    string_view _data;

    uint64_t _value = 0;
    uint64_t _nonce = 0;
    uint64_t _fee = 0;
};

bool verify(const TX& tx);

} // namespace sniper::mh
