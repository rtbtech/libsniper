// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

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

#include <limits.h>
#include <unistd.h>
#include "hostname.h"

namespace sniper::net {

string hostname_full()
{
    char hostname[HOST_NAME_MAX + 1];

    if (gethostname(hostname, HOST_NAME_MAX) == 0)
        return hostname;

    return "";
}

string hostname_short()
{
    string hostname = hostname_full();

    if (!hostname.empty()) {
        size_t pos = hostname.find('.');
        if (pos == 0)
            return "";

        if (pos != string::npos)
            return hostname.substr(0, pos);
    }

    return hostname;
}

} // namespace sniper::net
