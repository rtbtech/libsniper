/*
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

namespace sniper::strings {

/**
 * Returns a subpiece with all whitespace removed from the front of @sp.
 * Whitespace means any of [' ', '\n', '\r', '\t'].
 */
string_view trim_left(string_view sp);

/**
 * Returns a subpiece with all whitespace removed from the back of @sp.
 * Whitespace means any of [' ', '\n', '\r', '\t'].
 */
string_view trim_right(string_view sp);

/**
 * Returns a subpiece with all whitespace removed from the back and front of
 * @sp. Whitespace means any of [' ', '\n', '\r', '\t'].
 */
inline string_view trim(string_view sp)
{
    return trim_left(trim_right(sp));
}

} // namespace sniper::strings
