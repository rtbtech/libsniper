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

namespace sniper::lz4 {

[[nodiscard]] bool compress_raw_block(string_view src, string& dst);

[[nodiscard]] bool compress_uint32_block(string_view src, string& dst);

[[nodiscard]] bool decompress_raw_block(string_view src, string& dst);

[[nodiscard]] bool decompress_uint32_block(string_view src, string& dst, uint32_t max_size = 128 * 1024);

} // namespace sniper::lz4
