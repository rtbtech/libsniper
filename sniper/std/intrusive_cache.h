/*
 * Copyright (c) 2018 - 2019, MetaHash, Oleg Romanenko (oleg@romanenko.ro)
 *
 * Original: <boost/smart_ptr/intrusive_ref_counter.hpp>
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

namespace sniper {

template<typename DerivedT, typename CounterPolicyT, typename CacheT>
class intrusive_cache_ref_counter;

template<typename DerivedT, typename CounterPolicyT, typename CacheT>
void intrusive_ptr_add_ref(const intrusive_cache_ref_counter<DerivedT, CounterPolicyT, CacheT>* p) noexcept;

template<typename DerivedT, typename CounterPolicyT, typename CacheT>
void intrusive_ptr_release(intrusive_cache_ref_counter<DerivedT, CounterPolicyT, CacheT>* p) noexcept;


template<typename DerivedT, typename CounterPolicyT, typename CacheT>
class intrusive_cache_ref_counter
{
private:
    //! Reference counter type
    typedef typename CounterPolicyT::type counter_type;
    //! Reference counter
    mutable counter_type m_ref_counter;

public:
    /*!
     * Default constructor
     *
     * \post <tt>use_count() == 0</tt>
     */
    intrusive_cache_ref_counter() noexcept : m_ref_counter(0) {}

    /*!
     * Copy constructor
     *
     * \post <tt>use_count() == 0</tt>
     */
    intrusive_cache_ref_counter(intrusive_cache_ref_counter const&) noexcept : m_ref_counter(0) {}

    /*!
     * Assignment
     *
     * \post The reference counter is not modified after assignment
     */
    intrusive_cache_ref_counter& operator=(intrusive_cache_ref_counter const&) noexcept { return *this; }

    /*!
     * \return The reference counter
     */
    unsigned int use_count() const noexcept { return CounterPolicyT::load(m_ref_counter); }

protected:
    ~intrusive_cache_ref_counter() = default;

    friend void intrusive_ptr_add_ref<DerivedT, CounterPolicyT, CacheT>(
        const intrusive_cache_ref_counter<DerivedT, CounterPolicyT, CacheT>* p) noexcept;
    friend void intrusive_ptr_release<DerivedT, CounterPolicyT, CacheT>(
        intrusive_cache_ref_counter<DerivedT, CounterPolicyT, CacheT>* p) noexcept;
};


template<typename DerivedT, typename CounterPolicyT, typename CacheT>
inline void intrusive_ptr_add_ref(const intrusive_cache_ref_counter<DerivedT, CounterPolicyT, CacheT>* p) noexcept
{
    CounterPolicyT::increment(p->m_ref_counter);
}

template<typename DerivedT, typename CounterPolicyT, typename CacheT>
inline void intrusive_ptr_release(intrusive_cache_ref_counter<DerivedT, CounterPolicyT, CacheT>* p) noexcept
{
    if (CounterPolicyT::decrement(p->m_ref_counter) == 0)
        CacheT::release(static_cast<DerivedT*>(p));
}

} // namespace boost
