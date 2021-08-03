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

#include <boost/smart_ptr.hpp>
#include <boost/smart_ptr/atomic_shared_ptr.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <boost/smart_ptr/make_local_shared.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include <boost/version.hpp>
#include <memory>
#include <sniper/std/intrusive_cache.h>

namespace sniper {

using boost::atomic_shared_ptr;
using boost::dynamic_pointer_cast;
using boost::intrusive_ptr;
using boost::local_shared_ptr;
using boost::make_local_shared;
using boost::make_shared;
using boost::shared_ptr; // use boost::shared_ptr instead of std::shared_ptr
using boost::static_pointer_cast;
using std::make_unique;
using std::unique_ptr;

template<typename T>
using local_ptr = boost::local_shared_ptr<T>;

template<typename T>
using intrusive_unsafe_ref_counter = boost::intrusive_ref_counter<T, boost::thread_unsafe_counter>;

template<typename T, typename CacheT>
using intrusive_cache_unsafe_ref_counter = intrusive_cache_ref_counter<T, boost::thread_unsafe_counter, CacheT>;


// make_local for single objects
template<typename T, typename... Args>
inline typename boost::detail::lsp_if_not_array<T>::type make_local(Args&&... args)
{
    return boost::make_local_shared<T>(std::forward<Args>(args)...);
}

// make_intrusive for single objects
template<typename T, typename... _Args>
inline intrusive_ptr<T> make_intrusive(_Args&&... __args)
{
    return intrusive_ptr<T>(new T(std::forward<_Args>(__args)...));
}

template<typename T, typename... _Args>
inline intrusive_ptr<T> make_intrusive_noexcept(_Args&&... __args) noexcept
{
    return intrusive_ptr<T>(new (std::nothrow) T(std::forward<_Args>(__args)...));
}

} // namespace sniper

#if BOOST_VERSION <= 107500
namespace std {

template<typename T>
struct hash<sniper::intrusive_ptr<T>>
{
    size_t operator()(const sniper::intrusive_ptr<T>& k) const { return std::hash<T*>()(k.get()); }
};

} // namespace std
#endif
