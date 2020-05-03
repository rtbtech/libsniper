/*
 * toLowerAscii8, toLowerAscii32, toLowerAscii64, to_lower_ascii
 * from Facebook Folly library
 * https://github.com/facebook/folly/blob/master/folly/String.cpp
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
#include <strings.h>

namespace sniper::strings {

void to_lower_ascii(char* str, size_t length) noexcept;

inline void to_lower_ascii(string& str) noexcept
{
    // str[0] is legal also if the string is empty.
    to_lower_ascii(&str[0], str.size());
}

string to_lower_ascii_copy(string_view str);

inline bool iequals(string_view a, string_view b) noexcept
{
    return strncasecmp(a.data(), b.data(), std::min(a.size(), b.size())) == 0;
}

} // namespace sniper::strings
