/**
 * \file dcs/algorithm/mapping.hpp
 *
 * \brief Algorithms for mappings.
 *
 * Also see:
 * -# <a href="http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2639.pdf">N2639</a>
 * -# <a href="http://home.roadrunner.com/~hinnant/combinations.html">Combinations and Permutations</a>
 * .
 *
 * \author Howard Hinnant (howard.hinnant@gmail.com)
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 *
 * <hr/>
 *
 *  (C) Copyright Howard Hinnant 2005-2011.
 *      [http://home.roadrunner.com/~hinnant/combinations.html]
 *  Use, modification and distribution are subject to the Boost Software License,
 *  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt).
 *
 *  See http://www.boost.org/libs/type_traits for most recent version including
 *  documentation.
 *
 * <hr/>
 *
 * Copyright 2012 Marco Guazzone (marco.guazzone@gmail.com)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DCS_ALGORITHM_MAPPING_HPP
#define DCS_ALGORITHM_MAPPING_HPP


#include <dcs/algorithm/detail/combperm.hpp>


namespace dcs { namespace algorithm {

template <typename BidirectionalIterator, typename T>
inline
bool next_mapping(BidirectionalIterator first,
				  BidirectionalIterator last,
				  T first_value,
				  T last_value)
{
	return detail::next_mapping(first, last, first_value, last_value, detail::default_incrementor<T>());
}

template <typename BidirectionalIterator, typename T, typename Incrementor>
inline
bool next_mapping(BidirectionalIterator first,
				  BidirectionalIterator last,
				  T first_value,
				  T last_value,
				  Incrementor increment)
{
	return detail::next_mapping(first, last, first_value, last_value, increment);
}

template <typename BidirectionalIterator, typename T>
inline
bool prev_mapping(BidirectionalIterator first,
				  BidirectionalIterator last,
				  T first_value,
				  T last_value)
{
	return detail::prev_mapping(first, last, first_value, last_value, detail::default_decrementor<T>());
}

template <typename BidirectionalIterator, typename T, typename Decrementor>
inline
bool prev_mapping(BidirectionalIterator first,
				  BidirectionalIterator last,
				  T first_value,
				  T last_value,
				  Decrementor decrement)
{
	return detail::prev_mapping(first, last, first_value, last_value, decrement);
}

}} // Namespace dcs::algorithm


#endif // DCS_ALGORITHM_MAPPING_HPP
