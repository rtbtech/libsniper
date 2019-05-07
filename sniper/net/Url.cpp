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

#include <fmt/format.h>
#include <sniper/std/check.h>
#include "Url.h"
#include "http_parser.h"

namespace sniper::net {

namespace {

const size_t max_url_size = 1024;
const string_view default_port_str = "80";
const uint16_t default_port_int = 80;
const string_view default_path = "/";
const string_view default_schema = "http";

template<typename T>
void clear_tuple(T& t)
{
    get<string>(t).clear();
    get<string_view>(t) = {};
}

} // namespace

Url::Url()
{
    get<string_view>(_schema) = default_schema;
    get<string_view>(_path) = default_path;
}

Url::Url(string_view url)
{
    check(!url.empty(), "Url: empty url");
    check(url.size() <= max_url_size, "Url: url size > {}", max_url_size);
    check(parse(url), "Url: cannot parse: {}", url);
}

void Url::clear() noexcept
{
    _url.clear();

    clear_tuple(_schema);
    get<string_view>(_schema) = default_schema;

    _domain.clear();

    clear_tuple(_path);
    get<string_view>(_path) = default_path;

    clear_tuple(_query);
    clear_tuple(_fragment);
    clear_tuple(_userinfo);
}

string_view Url::url() const noexcept
{
    return _url;
}

Url::operator bool() const noexcept
{
    return static_cast<bool>(_domain);
}

bool Url::parse(string_view str) noexcept
{
    clear();

    if (str.empty())
        return false;

    try {
        _url = str;
    }
    catch (...) {
        // OOM guard
        perror("[OOM] Cannot copy url");
        return false;
    }

    string_view url(_url);
    http_parser_url u{};
    if (http_parser_parse_url(url.data(), url.size(), 0, &u) != 0)
        return false;

    if (u.field_set & (1 << UF_SCHEMA))
        get<string_view>(_schema) = url.substr(u.field_data[UF_SCHEMA].off, u.field_data[UF_SCHEMA].len);
    else
        get<string_view>(_schema) = default_schema;

    if (u.field_set & (1 << UF_HOST)) {
        if (u.field_set & (1 << UF_PORT))
            _domain.set(url.substr(u.field_data[UF_HOST].off, u.field_data[UF_HOST].len), u.port,
                        url.substr(u.field_data[UF_PORT].off, u.field_data[UF_PORT].len));
        else
            _domain.set(url.substr(u.field_data[UF_HOST].off, u.field_data[UF_HOST].len), default_port_int,
                        default_port_str);
    }
    else {
        return false;
    }

    if (u.field_set & (1 << UF_PATH))
        get<string_view>(_path) = url.substr(u.field_data[UF_PATH].off, u.field_data[UF_PATH].len);
    else
        get<string_view>(_path) = default_path;

    if (u.field_set & (1 << UF_QUERY))
        get<string_view>(_query) = url.substr(u.field_data[UF_QUERY].off, u.field_data[UF_QUERY].len);

    if (u.field_set & (1 << UF_FRAGMENT))
        get<string_view>(_fragment) = url.substr(u.field_data[UF_FRAGMENT].off, u.field_data[UF_FRAGMENT].len);

    if (u.field_set & (1 << UF_USERINFO))
        get<string_view>(_userinfo) = url.substr(u.field_data[UF_USERINFO].off, u.field_data[UF_USERINFO].len);

    return static_cast<bool>(_domain);
}

string_view Url::schema() const noexcept
{
    return get<string_view>(_schema);
}

string_view Url::host() const noexcept
{
    return _domain.name();
}

uint16_t Url::port() const noexcept
{
    return _domain.port();
}

string_view Url::port_sv() const noexcept
{
    return _domain.port_sv();
}

string_view Url::path() const noexcept
{
    return get<string_view>(_path);
}

string_view Url::query() const noexcept
{
    return get<string_view>(_query);
}

string_view Url::fragment() const noexcept
{
    return get<string_view>(_fragment);
}

string_view Url::userinfo() const noexcept
{
    return get<string_view>(_userinfo);
}

void Url::set_schema(string_view schema)
{
    get<string>(_schema) = schema;
    get<string_view>(_schema) = get<string>(_schema);
}

void Url::set_host(string_view host, uint16_t port)
{
    _domain.set(host, port);
}

void Url::set_path(string_view path)
{
    get<string>(_path) = path;
    get<string_view>(_path) = get<string>(_path);
}

void Url::set_query(string_view query)
{
    get<string>(_query) = query;
    get<string_view>(_query) = get<string>(_query);
}

void Url::set_fragment(string_view fragment)
{
    get<string>(_fragment) = fragment;
    get<string_view>(_fragment) = get<string>(_fragment);
}

void Url::set_userinfo(string_view userinfo)
{
    get<string>(_userinfo) = userinfo;
    get<string_view>(_userinfo) = get<string>(_userinfo);
}

const Domain& Url::domain() const noexcept
{
    return _domain;
}

void Url::set_domain(const Domain& domain)
{
    _domain = domain;
}

} // namespace sniper::net
