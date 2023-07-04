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

namespace sniper::strings {

template<class Container>
inline bool split(string_view str, string_view delim, Container& out)
{
    out.clear();

    if (str.empty() || delim.empty())
        return false;

    size_t pos = str.find_first_of(delim);
    if (pos == string_view::npos)
        return false;

    for (; pos != string_view::npos; pos = str.find_first_of(delim)) {
        if (!pos)
            out.emplace_back();
        else
            out.emplace_back(str.substr(0, pos));

        str.remove_prefix(pos + 1);
    }

    out.emplace_back(str);

    return !out.empty();
}

template<class Container>
inline bool split(string_view str, string_view delim, Container& out, size_t max_size)
{
    out.clear();

    if (str.empty() || delim.empty())
        return false;

    size_t pos = str.find_first_of(delim);
    if (pos == string_view::npos)
        return false;

    for (; pos != string_view::npos; pos = str.find_first_of(delim)) {
        if (out.size() == max_size)
            return false;

        if (!pos)
            out.emplace_back();
        else
            out.emplace_back(str.substr(0, pos));

        str.remove_prefix(pos + 1);
    }

    if (out.size() == max_size)
        return false;
    else
        out.emplace_back(str);

    return !out.empty();
}

#if __cplusplus >= 202002L
inline bool split2(const char* s, const char* delims, vector<string_view>& out)
{
    out.clear();

    if (!s || !delims)
        return false;

    char const* q = strpbrk(s, delims);
    if (!q)
        return false;

    for (; q != nullptr; q = strpbrk(s, delims)) {
        if (s != q)
            out.emplace_back(s, q);
        else
            out.emplace_back();

        s = q + 1;
    }

    if (s)
        out.emplace_back(s);
    else
        out.emplace_back();

    return !out.empty();
}
#endif

} // namespace sniper::strings
