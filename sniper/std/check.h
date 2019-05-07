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

#include <boost/stacktrace.hpp>
#include <fmt/format.h>
#include <stdexcept>
#include <string>

namespace sniper {

[[nodiscard]] inline std::string stacktrace_to_string(const boost::stacktrace::stacktrace& st)
{
    std::string frames = "backtrace:\n";
    unsigned idx = 0;
    for (auto& frame : st) {
        frames += fmt::format("#{} {}\n", idx, boost::stacktrace::to_string(frame));
        idx++;
    }

    return frames;
}

template<typename Cond, typename... Args>
inline void check(const Cond& cond, const char* fmt, const Args&... args)
{
    if (!cond)
        throw std::runtime_error{fmt::format(fmt, args...) + "\n"
                                 + stacktrace_to_string(boost::stacktrace::stacktrace())};
}

template<typename Cond, typename Msg>
inline void check(const Cond& cond, const Msg& msg)
{
    if (!cond)
        throw std::runtime_error{fmt::format(msg) + "\n"
                                 + stacktrace_to_string(boost::stacktrace::stacktrace())};
}

template<typename Cond>
inline void check(const Cond& cond)
{
    if (!cond)
        throw std::runtime_error{stacktrace_to_string(boost::stacktrace::stacktrace())};
}

} // namespace sniper
