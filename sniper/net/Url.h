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

#include <sniper/net/Domain.h>

namespace sniper::net {

class Url final
{
public:
    Url();
    explicit Url(string_view url);
    Url(const Url&) = delete;
    Url(Url&&) = delete;

    void clear() noexcept;
    [[nodiscard]] bool parse(string_view url) noexcept;

    [[nodiscard]] string_view url() const noexcept;
    [[nodiscard]] string_view schema() const noexcept;
    [[nodiscard]] string_view host() const noexcept;
    [[nodiscard]] uint16_t port() const noexcept;
    [[nodiscard]] string_view port_sv() const noexcept;
    [[nodiscard]] const Domain& domain() const noexcept;
    [[nodiscard]] string_view path() const noexcept;
    [[nodiscard]] string_view query() const noexcept;
    [[nodiscard]] string_view fragment() const noexcept;
    [[nodiscard]] string_view userinfo() const noexcept;

    void set_schema(string_view schema);
    void set_host(string_view host, uint16_t port = 80);
    void set_path(string_view path);
    void set_query(string_view query);
    void set_fragment(string_view fragment);
    void set_userinfo(string_view userinfo);
    void set_domain(const Domain& domain);

    explicit operator bool() const noexcept;

    Url& operator=(const Url&) = delete;
    Url& operator=(Url&&) = delete;

private:
    string _url;
    tuple<string_view, string> _schema;
    Domain _domain;
    tuple<string_view, string> _path;
    tuple<string_view, string> _query;
    tuple<string_view, string> _fragment;
    tuple<string_view, string> _userinfo;
};

} // namespace sniper::net
