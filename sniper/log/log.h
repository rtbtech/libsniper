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

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace sniper {

template<typename... Args>
inline void log_trace(const char* fmt, const Args&... args)
{
    spdlog::trace(fmt, args...);
}

template<typename... Args>
inline void log_info(const char* fmt, const Args&... args)
{
    spdlog::info(fmt, args...);
}

template<typename... Args>
inline void log_warn(const char* fmt, const Args&... args)
{
    spdlog::warn(fmt, args...);
}

template<typename... Args>
inline void log_err(const char* fmt, const Args&... args)
{
    spdlog::error(fmt, args...);
}

template<typename... Args>
inline void log_crit(const char* fmt, const Args&... args)
{
    spdlog::critical(fmt, args...);
}

template<typename T>
void log_trace(const T& msg)
{
    spdlog::trace(msg);
}

template<typename T>
void log_info(const T& msg)
{
    spdlog::info(msg);
}

template<typename T>
void log_warn(const T& msg)
{
    spdlog::warn(msg);
}

template<typename T>
void log_err(const T& msg)
{
    spdlog::error(msg);
}

template<typename T>
void log_crit(const T& msg)
{
    spdlog::critical(msg);
}

} // namespace sniper
