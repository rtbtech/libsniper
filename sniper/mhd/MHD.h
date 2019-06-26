/*
 * Copyright (c) 2015 - 2016, AdSniper, Oleg Romanenko (oleg@romanenko.ro)
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

#include <sniper/std/map.h>
#include <sniper/std/string.h>
#include <sniper/std/vector.h>

namespace sniper::mhd {

class MHD
{
public:
    enum Expiration
    {
        EXP_NONE,
        EXP_THISHOUR,
        EXP_THISDAY,
        EXP_THISWEEK,
        EXP_THISYEAR,
        EXP_20YEARS,
        EXP_10MIN
    };

    struct Request final
    {
        string remote_ip;
        string method;
        string url;
        string version;
        string post;

        map<string, string> headers;
        map<string, string> cookies;
        map<string, string> params;
        multimap<string, string> params_multi;

        void clear()
        {
            remote_ip.clear();
            method.clear();
            url.clear();
            version.clear();
            post.clear();

            headers.clear();
            cookies.clear();
            params.clear();
            params_multi.clear();
        }
    };

    struct Response final
    {
        int code = 200;
        string data;

        map<string, string> headers;
        map<string, string> cookies;

        void clear()
        {
            code = 200;
            data.clear();
            headers.clear();
            cookies.clear();
        }
    };


    MHD();
    virtual ~MHD() = default;

    [[nodiscard]] bool start(const string& path = "");
    [[nodiscard]] bool start_test(const string& path);

    virtual bool run(int thread_number, Request& req, Response& resp) = 0;

    [[nodiscard]] static string set_cookie(const string& key, const string& value, int64_t ttl, const string& domain);
    [[nodiscard]] static time_t get_expiration(enum Expiration exp);
    [[nodiscard]] static bool parse_qs(const string& qs, map<string, string>& params);

protected:
    virtual bool init() = 0;
    virtual void fini(){};
    virtual void idle(){};
    virtual void usr1(){};
    virtual void usr2(){};

    void set_host(const string& host);
    void set_port(unsigned int port);
    void set_threads(unsigned int count);
    void set_connection_limit(unsigned int num);
    void set_per_ip_connection_limit(unsigned int num);
    void set_connection_timeout(unsigned int num);

    [[nodiscard]] unsigned int get_threads();
    [[nodiscard]] const string& get_config_path();

private:
    string config_path;
    string host;
    unsigned int port = 8080;
    unsigned int threads_count = 1;
    unsigned int connection_limit = 200000;
    unsigned int per_ip_connection_limit = 0;
    unsigned int connection_timeout = 10; // seconds
};

} // namespace sniper::mhd
