/*
 * Copyright (c) 2020 - 2022, Oleg Romanenko (oleg@romanenko.ro)
 */

#pragma once

#include <sniper/cache/Cache.h>
#include <sniper/event/Loop.h>
#include <sniper/log/log.h>
#include <sniper/std/functional.h>
#include <sniper/std/memory.h>
#include <sniper/std/vector.h>

namespace sniper::async::wait {

template<class Query>
class Group;

template<class Query>
using GroupsCache = cache::STDCache<Group<Query>>;

template<class Query>
class Group final : public intrusive_cache_unsafe_ref_counter<Group<Query>, GroupsCache<Query>>
{
public:
    void clear() noexcept;

    explicit Group();
    ~Group();

    void set(const event::loop_ptr& loop, unsigned size);

    void add(Query& p);

    void then(function<void(const vector<intrusive_ptr<Query>>&)>&& cb);

private:
    void cb_done(ev::prepare& w, [[maybe_unused]] int revents) noexcept;
    function<void(const vector<intrusive_ptr<Query>>&)> cb_queries;

    unsigned complete = 0;
    vector<intrusive_ptr<Query>> queries;

    event::loop_ptr loop;
    ev::prepare w_done;
};

template<class Query>
Group<Query>& make_group(const event::loop_ptr& loop, unsigned cnt)
{
    auto* group = GroupsCache<Query>::get_raw();
    group->set(loop, cnt);

    return *group;
}


template<class Query>
Group<Query>::Group()
{
    queries.reserve(10);
    w_done.set<Group<Query>, &Group<Query>::cb_done>(this);
}

template<class Query>
Group<Query>::~Group()
{
    clear();
}

template<class Query>
void Group<Query>::clear() noexcept
{
    complete = 0;
    queries.clear();

    if (loop)
        w_done.stop();

    loop.reset();
}

template<class Query>
void Group<Query>::set(const event::loop_ptr& l, unsigned int size)
{
    clear();

    loop = l;
    w_done.set(*loop);

    queries.reserve(size);
}

template<class Query>
void Group<Query>::then(function<void(const vector<intrusive_ptr<Query>>&)>&& cb)
{
    cb_queries = std::move(cb);
}

template<class Query>
void Group<Query>::add(Query& p)
{
    queries.emplace_back(std::move(p.get_intrusive()));
    queries.back()->then([this](const auto& resp) {
        complete++;

        if (complete == queries.size()) {
            if (!w_done.is_active()) {
                w_done.start();
                w_done.feed_event(0);
            }
        }
    });
}

template<class Query>
void Group<Query>::cb_done(ev::prepare& w, int revents) noexcept
{
    w.stop();

    if (cb_queries && !queries.empty()) {
        try {
            cb_queries(queries);
        }
        catch (std::exception& e) {
            log_err("[wait::Group] Exception in user callback: {}", e.what());
        }
        catch (...) {
            log_err("[wait::Group] Exception in user callback");
        }
    }

    GroupsCache<Query>::release(this);
}

} // namespace sniper::async::wait
