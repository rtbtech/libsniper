/*
 * Copyright (c) 2018 - 2020, MetaHash, MediaSniper, Oleg Romanenko (oleg@romanenko.ro)
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

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <sniper/std/chrono.h>
#include <sniper/std/string.h>

namespace sniper {

template<typename... Args>
inline void _log(FILE* out, string_view level, const char* fmt, const Args&... args)
{
    auto tp = high_resolution_clock::now();
    milliseconds ms = duration_cast<milliseconds>(tp.time_since_epoch());
    seconds s = duration_cast<seconds>(ms);

    fmt::memory_buffer buf;
    fmt::format_to(buf, fmt, args...);
    fmt::print(out, "[{:%Y-%m-%d %H:%M:%S}.{}] [{}] {}\n", fmt::localtime(s.count()), ms.count() % 1000, level,
               string_view(buf.data(), buf.size()));
    fflush(out);
}

#ifdef SNIPER_TRACE
template<typename Format>
inline void log_trace(Format msg)
{
    _log(stdout, "trace", "{}", msg);
}

template<typename Format, typename... Args>
inline void log_trace(Format fmt, const Args&... args)
{
    _log(stdout, "trace", fmt, args...);
}
#else
#define log_trace(...) (void)0
#endif

template<typename Format>
inline void log_info(Format msg)
{
    _log(stdout, "info", "{}", msg);
}

template<typename Format>
inline void log_warn(Format msg)
{
    _log(stdout, "warning", "{}", msg);
}

template<typename Format>
inline void log_err(Format msg)
{
    _log(stderr, "error", "{}", msg);
}

template<typename Format>
inline void log_crit(Format msg)
{
    _log(stderr, "critical", "{}", msg);
}

template<typename Format, typename... Args>
inline void log_info(Format fmt, const Args&... args)
{
    _log(stdout, "info", fmt, args...);
}

template<typename Format, typename... Args>
inline void log_warn(Format fmt, const Args&... args)
{
    _log(stdout, "warning", fmt, args...);
}

template<typename Format, typename... Args>
inline void log_err(Format fmt, const Args&... args)
{
    _log(stderr, "error", fmt, args...);
}

template<typename Format, typename... Args>
inline void log_crit(Format fmt, const Args&... args)
{
    _log(stderr, "critical", fmt, args...);
}

} // namespace sniper
