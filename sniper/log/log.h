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

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/time.h>
#include <sniper/std/string.h>

namespace sniper {

template<typename... Args>
inline void log(FILE* out, string_view level, const char* fmt, const Args&... args)
{
    fmt::memory_buffer buf;
    fmt::format_to(buf, fmt, args...);
    fmt::print(out, "[{:%Y-%m-%d %H:%M:%S}] [{}] {}\n", fmt::localtime(time(nullptr)), level,
               string_view(buf.data(), buf.size()));
    fflush(out);
}

template<typename T>
inline void log(FILE* out, string_view level, const T& msg)
{
    fmt::print(out, "[{:%Y-%m-%d %H:%M:%S}] [{}] {}\n", fmt::localtime(time(nullptr)), level, msg);
    fflush(out);
}

#ifdef SNIPER_TRACE
template<typename... Args>
inline void log_trace(const char* fmt, const Args&... args)
{
    log(stdout, "trace", fmt, args...);
}

template<typename T>
void log_trace(const T& msg)
{
    log(stdout, "trace", msg);
}
#else
#define log_trace(...) (void)0
#endif

template<typename... Args>
inline void log_info(const char* fmt, const Args&... args)
{
    log(stdout, "info", fmt, args...);
}

template<typename... Args>
inline void log_warn(const char* fmt, const Args&... args)
{
    log(stdout, "warning", fmt, args...);
}

template<typename... Args>
inline void log_err(const char* fmt, const Args&... args)
{
    log(stderr, "error", fmt, args...);
}

template<typename... Args>
inline void log_crit(const char* fmt, const Args&... args)
{
    log(stderr, "critical", fmt, args...);
}

template<typename T>
void log_info(const T& msg)
{
    log(stdout, "info", msg);
}

template<typename T>
void log_warn(const T& msg)
{
    log(stdout, "warning", msg);
}

template<typename T>
void log_err(const T& msg)
{
    log(stderr, "error", msg);
}

template<typename T>
void log_crit(const T& msg)
{
    log(stderr, "critical", msg);
}

} // namespace sniper
