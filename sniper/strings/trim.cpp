// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

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

#include "trim.h"

namespace sniper::strings {

namespace {

inline bool is_oddspace(char c)
{
    return c == '\n' || c == '\t' || c == '\r';
}

} // namespace

string_view trim_left(string_view sp)
{
    // Spaces other than ' ' characters are less common but should be
    // checked.  This configuration where we loop on the ' '
    // separately from oddspaces was empirically fastest.

    while (true) {
        while (!sp.empty() && sp.front() == ' ')
            sp.remove_prefix(1);

        if (!sp.empty() && is_oddspace(sp.front())) {
            sp.remove_prefix(1);
            continue;
        }

        return sp;
    }
}

string_view trim_right(string_view sp)
{
    // Spaces other than ' ' characters are less common but should be
    // checked.  This configuration where we loop on the ' '
    // separately from oddspaces was empirically fastest.

    while (true) {
        while (!sp.empty() && sp.back() == ' ')
            sp.remove_suffix(1);

        if (!sp.empty() && is_oddspace(sp.back())) {
            sp.remove_suffix(1);
            continue;
        }

        return sp;
    }
}

} // namespace sniper::strings
