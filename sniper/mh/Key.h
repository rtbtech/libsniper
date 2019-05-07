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

#include <sniper/std/filesystem.h>
#include <sniper/std/string.h>

namespace sniper::mh {

class Key final
{
public:
    explicit Key(const fs::path& p);

    string_view hex_addr() const noexcept;
    string_view hex_pubkey() const noexcept;
    string_view raw_privkey() const noexcept;

private:
    string _hex_addr;
    string _hex_pubkey;
    string _bin_privkey_full;
    string _bin_privkey_min;
};

} // namespace sniper::mh
