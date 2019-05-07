/*
 * Copyright (c) 2018 - 2019, MetaHash
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

namespace sniper::mh {

uint8_t append_varint(uint64_t value, string& out);

uint8_t read_varint(string_view data, uint64_t &varint);

} // namespace sniper::mh
