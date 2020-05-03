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

#include <sniper/pico/picohttpparser.h>
#include <sniper/strings/ascii_case.h>
#include <sniper/strings/atoi.h>
#include "Request.h"

namespace sniper::pico {

namespace {

const string_view header_content_length = "content-length";
const string_view header_connection = "connection";
const string_view header_connection_keep_alive = "keep-alive";
const string_view header_connection_close = "close";

pair_sv parse_param(string_view param)
{
    if (auto pos = param.find_first_of('='); pos != string_view::npos)
        return make_pair(param.substr(0, pos), param.substr(pos + 1));

    return make_pair(param, "");
}

small_vector<pair_sv, MAX_PARAMS> parse_qs(string_view qs)
{
    small_vector<pair_sv, MAX_PARAMS> out;

    while (!qs.empty()) {
        if (size_t pos = qs.find_first_of("&"); pos != string_view::npos) {
            if (pos)
                out.emplace_back(parse_param(qs.substr(0, pos)));

            qs.remove_prefix(pos + 1);
        }
        else {
            out.emplace_back(parse_param(qs));
            break;
        }
    }

    return out;
}

} // namespace

void Request::clear() noexcept
{
    header_size = 0;
    content_length = 0;
    keep_alive = false;
    minor_version = 0;
    method = {};
    path = {};
    qs = {};
    fragment = {};
    headers.clear();
    params.clear();
}

ParseResult Request::parse(char* data, size_t size) noexcept
{
    if (!data || !size)
        return ParseResult::Err;

    if (size < 5)
        return ParseResult::Partial;


    const char* pico_method = nullptr;
    size_t pico_method_len = 0;

    const char* pico_path = nullptr;
    size_t pico_path_len = 0;

    struct phr_header pico_headers[MAX_HEADERS];
    size_t num_headers = sizeof(pico_headers) / sizeof(headers[0]);

    int pico_minor_version = -1;

    int ssize = phr_parse_request(data, size, &pico_method, &pico_method_len, &pico_path, &pico_path_len,
                                  &pico_minor_version, pico_headers, &num_headers, 0);

    if (ssize > 0) {
        header_size = ssize;

        if (pico_minor_version == 0 || pico_minor_version == 1)
            minor_version = pico_minor_version;

        if (pico_method && pico_method_len) {
            if (normalize_method)
                strings::to_lower_ascii(const_cast<char*>(pico_method), pico_method_len);
            method = string_view(pico_method, pico_method_len);
        }

        if (pico_path && pico_path_len) {
            if (normalize_path)
                strings::to_lower_ascii(const_cast<char*>(pico_path), pico_path_len);
            path = string_view(pico_path, pico_path_len);

            // fragment
            if (auto pos = path.find_first_of('#'); pos != string_view::npos) {
                if (pos + 1 != path.size())
                    fragment = path.substr(pos + 1);

                path.remove_suffix(path.size() - pos);
            }

            // qs
            if (auto pos = path.find_first_of('?'); pos != string_view::npos) {
                if (pos + 1 != path.size())
                    qs = path.substr(pos + 1);

                path.remove_suffix(path.size() - pos);
            }

            // params
            if (!qs.empty())
                params = parse_qs(qs);
        }
        else {
            path = "/";
        }

        if (minor_version == 1)
            keep_alive = true;

        bool content_length_found = false;
        bool connection_found = false;

        for (unsigned i = 0; i < num_headers; i++) {
            if (normalize_headers_names)
                strings::to_lower_ascii(const_cast<char*>(pico_headers[i].name), pico_headers[i].name_len);
            if (normalize_headers_values)
                strings::to_lower_ascii(const_cast<char*>(pico_headers[i].value), pico_headers[i].value_len);

            string_view key(pico_headers[i].name, pico_headers[i].name_len);
            string_view val(pico_headers[i].value, pico_headers[i].value_len);

            // content-length
            if (!content_length_found && strings::iequals(key, header_content_length)) {
                content_length_found = true;
                if (auto len = strings::fast_atoi64(val); len)
                    content_length = *len;
                else
                    return ParseResult::Err;
            }

            // connection
            if (!connection_found && strings::iequals(key, header_connection)) {
                connection_found = true;

                if (minor_version == 0 && strings::iequals(val, header_connection_keep_alive))
                    keep_alive = true;
                else if (minor_version == 1 && strings::iequals(val, header_connection_close))
                    keep_alive = false;
            }

            headers.emplace_back(key, val);
        }

        return ParseResult::Complete;
    }
    else if (ssize == -2) {
        return ParseResult::Partial;
    }
    else {
        return ParseResult::Err;
    }
}

} // namespace sniper::pico
