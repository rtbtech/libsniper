// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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
#include <arpa/inet.h>
#include <microhttpd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sniper/net/ip.h>
#include <string.h>
#include <uriparser/Uri.h>
#include "MHD.h"

namespace sniper::mhd {

/* MHD static callbacks */
static int grab_key_value(void* cls, enum MHD_ValueKind kind, const char* key, const char* value)
{
    if (cls && key) {
        auto* req = (MHD::Request*)cls;

        if (kind == MHD_HEADER_KIND) {
            if (value)
                req->headers[key] = value;
            else
                req->headers[key];
        }
        else if (kind == MHD_COOKIE_KIND) {
            if (value)
                req->cookies[key] = value;
            else
                req->cookies[key];
        }
        else if (kind == MHD_GET_ARGUMENT_KIND) {
            if (value) {
                req->params[key] = value;
                req->params_multi.insert(std::make_pair(key, value));
            }
            else {
                req->params[key];
                req->params_multi.insert(std::make_pair(key, ""));
            }
        }

        return MHD_YES;
    }

    return MHD_NO;
}

static int mhd_request_callback(void* cls, struct MHD_Connection* connection, const char* url, const char* method,
                                const char* version, const char* upload_data, size_t* upload_data_size, void** ptr)
{
    if (!method
        || (0 != strcmp(method, "GET") && 0 != strcmp(method, "POST") && 0 != strcmp(method, "HEAD")
            && 0 != strcmp(method, "PUT")))
        return MHD_NO; /* unexpected method */

    auto* req = (MHD::Request*)*ptr;
    if (req == nullptr) {
        /* do never respond on first call */
        req = new MHD::Request;
        req->method.assign(method);
        if (url)
            req->url.assign(url);
        if (version)
            req->version.assign(version);
        if (upload_data && upload_data_size && *upload_data_size) {
            req->post.assign(upload_data, *upload_data_size);
            *upload_data_size = 0;
        }

        MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, grab_key_value, req);
        MHD_get_connection_values(connection, MHD_COOKIE_KIND, grab_key_value, req);
        MHD_get_connection_values(connection, MHD_HEADER_KIND, grab_key_value, req);

        // REMOTE_ADDR
        const auto* sa = reinterpret_cast<const sockaddr_in*>(
            MHD_get_connection_info(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr);
        req->remote_ip = net::ip_to_str(*sa);

        *ptr = req;
        return MHD_YES;
    }

    /* Continue to load post data */
    if (upload_data && upload_data_size && *upload_data_size) {
        req->post.append(upload_data, *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
    }


    /* Complete */
    *ptr = nullptr;
    MHD::Response resp;


    if (req->headers.count("SNIPER_TEST")) {
        /* test request */
        resp.code = 204;
        resp.headers["SNIPER_TEST"] = "TEST_OK";
    }
    else {
        MHD* mhd_server = (MHD*)cls;
        mhd_server->run(MHD_get_thread_number(connection), *req, resp);
    }


    delete req;
    int rc = MHD_NO;

    /* Process data */
    struct MHD_Response* response = nullptr;
    if (!resp.data.empty()) {
        response = MHD_create_response_from_buffer(resp.data.size(), (char*)resp.data.c_str(), MHD_RESPMEM_MUST_COPY);
    }
    else {
        response = MHD_create_response_from_buffer(0, nullptr, MHD_RESPMEM_PERSISTENT);
    }
    if (response == nullptr)
        return MHD_NO;


    /* Process headers */
    if (!resp.headers.empty()) {
        for (auto& a : resp.headers) {
            rc = MHD_add_response_header(response, a.first.c_str(), a.second.c_str());
            if (rc == MHD_NO)
                return MHD_NO;
        }
    }

    /* Process cookies */
    if (!resp.cookies.empty()) {
        for (auto& a : resp.cookies) {
            if (!a.second.empty()) {
                rc = MHD_add_response_header(response, MHD_HTTP_HEADER_SET_COOKIE, (a.first + "=" + a.second).c_str());
                if (rc == MHD_NO)
                    return MHD_NO;
            }
        }
    }

    if (resp.code < 200)
        resp.code = 204;

    rc = MHD_queue_response(connection, resp.code, response);
    MHD_destroy_response(response);
    return rc;
}

MHD::MHD()
{
    /* Linker bug fix
     * libmicrohttpd/src/microhttpd/internal.c:188: undefined reference to `clock_gettime'
     */
    struct timespec tps
    {};
    clock_gettime(CLOCK_REALTIME, &tps);
}

bool MHD::start(const string& path)
{
    this->config_path = path;
    if (this->config_path.empty())
        this->config_path = "./config";
    if (this->config_path.back() != '/')
        this->config_path += "/";

    if (!init())
        return false;

    /* Block signals */
    sigset_t blocked;
    sigemptyset(&blocked);
    sigaddset(&blocked, SIGUSR1);
    sigaddset(&blocked, SIGUSR2);
    pthread_sigmask(SIG_BLOCK, &blocked, nullptr);


    struct MHD_Daemon* d = nullptr;

    if (host.empty()) {
        d = MHD_start_daemon(MHD_USE_EPOLL_INTERNALLY_LINUX_ONLY | MHD_USE_EPOLL_TURBO, port, nullptr, nullptr,
                             &mhd_request_callback, this, MHD_OPTION_THREAD_POOL_SIZE, (unsigned int)threads_count,
                             MHD_OPTION_CONNECTION_LIMIT, connection_limit, MHD_OPTION_PER_IP_CONNECTION_LIMIT,
                             per_ip_connection_limit, MHD_OPTION_CONNECTION_TIMEOUT, connection_timeout,
                             MHD_OPTION_END);
    }
    else {
        struct sockaddr_in sa
        {};
        sa.sin_family = AF_INET;
        sa.sin_port = htons(port);
        if (inet_pton(AF_INET, host.c_str(), &sa.sin_addr) == 1) {
            d = MHD_start_daemon(MHD_USE_EPOLL_INTERNALLY_LINUX_ONLY | MHD_USE_EPOLL_TURBO, port, nullptr, nullptr,
                                 &mhd_request_callback, this, MHD_OPTION_THREAD_POOL_SIZE, (unsigned int)threads_count,
                                 MHD_OPTION_CONNECTION_LIMIT, connection_limit, MHD_OPTION_PER_IP_CONNECTION_LIMIT,
                                 per_ip_connection_limit, MHD_OPTION_CONNECTION_TIMEOUT, connection_timeout,
                                 MHD_OPTION_SOCK_ADDR, (sockaddr*)&sa, MHD_OPTION_END);
        }
    }

    if (!d) {
        fprintf(stderr, "Cannot start MHD daemon\n");
        return false;
    }


    while (true) {
        sigset_t pending;
        sigpending(&pending);

        if (sigismember(&pending, SIGUSR1)) {
            int sig = SIGUSR1;
            sigwait(&pending, &sig);

            usr1();
        }

        if (sigismember(&pending, SIGUSR2)) {
            int sig = SIGUSR2;
            sigwait(&pending, &sig);

            usr2();
        }

        idle();

        sleep(1);
    }


    MHD_stop_daemon(d);

    fini();

    return true;
}


bool MHD::start_test(const string& path)
{
    this->config_path = path;
    if (this->config_path.empty())
        this->config_path = "./config";
    if (this->config_path.back() != '/')
        this->config_path += "/";

    return init();
}

string MHD::set_cookie(const string& key, const string& value, int64_t ttl, const string& domain)
{
    if (domain.empty())
        return "";

    char buf[50];
    buf[0] = '\0';

    string exp;

    if (ttl > 0) {
        ttl += time(nullptr);
        tm timeinfo{};
        gmtime_r(&ttl, &timeinfo);

        if (strftime(buf, 49, "%a, %d %b %Y %H:%M:%S GMT", &timeinfo)) {
            exp = " Expires=";
            exp += buf;
            exp += ";";
        }
    }

    return key + "=" + value + "; Path=/;" + exp + " Domain=" + domain;
}

time_t MHD::get_expiration(enum Expiration exp)
{
    time_t currenttime;
    struct tm local
    {};

    switch (exp) {
        case EXP_NONE:
            return 0;

        case EXP_THISHOUR:
            currenttime = time(nullptr);
            localtime_r(&currenttime, &local);
            local.tm_min = 59;
            local.tm_sec = 59;

            return mktime(&local) - currenttime;

        case EXP_THISDAY:
            currenttime = time(nullptr);
            localtime_r(&currenttime, &local);
            local.tm_hour = 23;
            local.tm_min = 59;
            local.tm_sec = 59;

            return mktime(&local) - currenttime;

        case EXP_THISWEEK:
            currenttime = time(nullptr);
            localtime_r(&currenttime, &local);
            local.tm_hour = 23;
            local.tm_min = 59;
            local.tm_sec = 59;

            return mktime(&local) + 24 * 3600 * (1 + 6 - local.tm_wday) - currenttime;

        case EXP_10MIN:
            return 10 * 60;

        case EXP_THISYEAR:
            currenttime = time(nullptr);
            localtime_r(&currenttime, &local);
            local.tm_mday = 31;
            local.tm_mon = 11; // Month: 0..11
            local.tm_hour = 23;
            local.tm_min = 59;
            local.tm_sec = 59;

            return mktime(&local) - currenttime;

        case EXP_20YEARS:
            return 20 * 365 * 24 * 3600;

        default:
            return 0;
    }
}

void MHD::set_host(const string& h)
{
    this->host = h;
}

void MHD::set_port(unsigned int p)
{
    this->port = p;
}

void MHD::set_threads(unsigned int count)
{
    this->threads_count = count;
}

void MHD::set_connection_limit(unsigned int num)
{
    this->connection_limit = num;
}

void MHD::set_per_ip_connection_limit(unsigned int num)
{
    this->per_ip_connection_limit = num;
}

void MHD::set_connection_timeout(unsigned int num)
{
    this->connection_timeout = num;
}

bool MHD::parse_qs(const string& qs, map<string, string>& params)
{
    if (qs.empty())
        return false;

    params.clear();

    UriQueryListA *query_list, *it;
    int item_count, i = 0;

    const char* qs_end = strchr(qs.c_str(), '\0');
    if (qs_end && *(qs_end - 1) == '=')
        qs_end--;

    if (uriDissectQueryMallocA(&query_list, &item_count, qs.c_str(), qs_end) != URI_SUCCESS) {
        return false;
    }

    if (!query_list)
        return true;

    for (it = query_list; i < item_count && it != nullptr; it = it->next, i++) {
        if (it->key) {
            if (it->value)
                params.insert(std::pair<string, string>(it->key, it->value));
            else
                params.insert(std::pair<string, string>(it->key, ""));
        }
    }

    uriFreeQueryListA(query_list);

    return true;
}

const string& MHD::get_config_path()
{
    return config_path;
}

unsigned int MHD::get_threads()
{
    return threads_count;
}

} // namespace sniper::mhd
