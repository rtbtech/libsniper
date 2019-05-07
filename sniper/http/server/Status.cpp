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

#include "Status.h"

namespace sniper::http {

string_view http_status(int version, ResponseStatus code)
{
    if (version == 1) {
        switch (static_cast<int>(code)) {
#define XX(num, name, string) \
    case num:                 \
        return string_view("HTTP/1.1 " #num " " #string "\r\n", sizeof("HTTP/1.1 " #num " " #string "\r\n") - 1);
            HTTP_STATUS_MAP(XX)
#undef XX
            default:
                return {};
        }
    }
    else {
        switch (static_cast<int>(code)) {
#define XX(num, name, string) \
    case num:                 \
        return string_view("HTTP/1.0 " #num " " #string "\r\n", sizeof("HTTP/1.0 " #num " " #string "\r\n") - 1);
            HTTP_STATUS_MAP(XX)
#undef XX
            default:
                return {};
        }
    }
}

} // namespace sniper::http
