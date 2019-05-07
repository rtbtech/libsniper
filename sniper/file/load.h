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

#include <cstring>
#include <fstream>
#include <sniper/std/filesystem.h>
#include <sniper/std/string.h>

namespace sniper::file {

inline bool load_file_to_string(const fs::path& p, string& out) noexcept
{
    out.clear();

    try {
        std::ifstream ifs(p);

        if (ifs.is_open()) {
            out.assign((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        }
    }
    catch (...) {
    }

    return !out.empty();
}

inline string load_file_to_string(const fs::path& p) noexcept
{
    string out;
    load_file_to_string(p, out);
    return out;
}

template<class Processor>
inline bool load_file_by_line(const fs::path& p, size_t max_line_size, Processor&& processor)
{
    if (error_code ec; !fs::exists(p, ec) || ec)
        return false;

    if (error_code ec; !fs::file_size(p, ec) || ec)
        return false;

    FILE* f = fopen(p.c_str(), "r");
    if (!f)
        return false;

    char* buf = (char*)malloc(max_line_size + 1);
    if (!buf) {
        fclose(f);
        return false;
    }

    while (fgets(buf, max_line_size, f)) {
        size_t size = strlen(buf);
        do {
            if (size > 0 && (buf[size - 1] == '\n' || buf[size - 1] == '\r')) {
                buf[size - 1] = '\0';
                size--;
            }
            else {
                break;
            }
        } while (true);

        if (size == 0)
            continue;

        processor(string_view(buf, size));
    }

    fclose(f);
    free(buf);

    return true;
}

template<class Processor>
inline bool load_file_by_line(const fs::path& p, Processor&& processor)
{
    return load_file_by_line(p, 1024 * 1024, std::forward<Processor>(processor));
}


template<class Processor>
inline bool load_file_by_chunk(const fs::path& p, Processor&& processor)
{
    if (error_code ec; !fs::exists(p, ec) || ec)
        return false;

    size_t file_size = 0;
    {
        error_code ec;
        file_size = fs::file_size(p, ec);
        if (ec || !file_size)
            return false;
    }


    FILE* file = fopen(p.c_str(), "r");
    if (file == nullptr)
        return false;


    size_t file_block_size = 16 * 1024 * 1024;
    char* buf = new char[file_block_size]; // FIXME: exception

    size_t header_size = 4, message_size = 0;

    size_t real_size = 0;
    size_t rd_cnt = 0;
    do {
        /* Check file size before read */
        if (real_size == file_size)
            break;

        /* Read header */
        message_size = 0;
        rd_cnt = fread((char*)&message_size, 1, header_size, file);
        if (rd_cnt == 0)
            break;

        if (feof(file))
            break;

        if (ferror(file))
            break;

        if (rd_cnt != header_size)
            break;

        if (message_size == 0)
            break;

        if (real_size + header_size + message_size > file_size)
            break;


        if (message_size > file_block_size) {
            delete[] buf;
            file_block_size = message_size;
            buf = new char[file_block_size];
        }

        size_t rd = fread(buf, 1, message_size, file);
        if (rd != message_size)
            break;

        processor(string_view(buf, message_size));

        real_size += header_size + message_size;
    } while (true);

    fclose(file);
    delete[] buf;

    return true;
}

} // namespace sniper::file
