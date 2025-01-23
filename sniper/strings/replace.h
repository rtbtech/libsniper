/*
* Copyright (c) 2020, Oleg Romanenko (oleg@romanenko.ro)
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
#include <sniper/std/optional.h>

namespace sniper::strings {

inline char test_macro_char(char c)
{
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9') || c == '_';
}

template<class Func>
inline void replace_macro(string_view src, string& dst, Func&& func)
{
    enum class State
    {
        Text,
        Macro
    };

    if (dst.capacity() < src.size() * 2)
        dst.reserve(src.size() * 2);

    size_t src_size = src.size();
    size_t idx = 0;
    size_t idx_macro_begin = 0;
    State state = State::Text;
    while (idx < src_size) {
        auto curr_char = src[idx];

        switch (state) {
            case State::Text:
                if (curr_char == '{') {
                    state = State::Macro;
                    idx_macro_begin = idx;
                }
                else {
                    dst.push_back(curr_char);
                    idx++;
                }
                break;

            case State::Macro:
                if (curr_char == '}') {
                    if (idx - idx_macro_begin > 1) {
                        optional<string_view> res = func(src.substr(idx_macro_begin + 1, idx - idx_macro_begin - 1));
                        if (res) {
                            dst.append(res->data(), res->size());
                        }
                        else {
                            auto macro = src.substr(idx_macro_begin, idx - idx_macro_begin + 1);
                            dst.append(macro.data(), macro.size());
                        }
                        state = State::Text;
                        idx++;
                    }
                    else {
                        auto macro = src.substr(idx_macro_begin, idx - idx_macro_begin + 1);
                        dst.append(macro.data(), macro.size());

                        state = State::Text;
                        idx++;
                    }
                }
                else if (curr_char == '{' && idx > idx_macro_begin) {
                    auto macro = src.substr(idx_macro_begin, idx - idx_macro_begin);
                    dst.append(macro.data(), macro.size());

                    state = State::Text;
                }
                else if (idx == idx_macro_begin || test_macro_char(curr_char)) {
                    if (idx + 1 < src_size) {
                        idx++;
                    }
                    else {
                        auto macro = src.substr(idx_macro_begin, idx - idx_macro_begin + 1);
                        dst.append(macro.data(), macro.size());

                        state = State::Text;
                        idx++;
                    }
                }
                else {
                    auto macro = src.substr(idx_macro_begin, idx - idx_macro_begin + 1);
                    dst.append(macro.data(), macro.size());

                    state = State::Text;
                    idx++;
                }
                break;

            default:
                idx++;
                break;
        }
    }
}

} // namespace sniper::strings
