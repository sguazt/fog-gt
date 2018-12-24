/**
 * \file dcs/algorithm/order.hpp
 *
 * \brief Computes the permutation of indices which makes the given range
 *  ordered.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 *
 * <hr/>
 *
 * Copyright 2009 Marco Guazzone (marco.guazzone@gmail.com)
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

#ifndef DCS_ALGORITHM_ORDER_HPP
#define DCS_ALGORITHM_ORDER_HPP


#include <algorithm>
#include <cstddef>


namespace dcs { namespace algorithm {

namespace detail {

/// Auxiliary comparator used for comparing elements of input sequence during
/// the ordering of indices.
template <typename RandomAccessIteratorT>
struct index_less_than
{
	index_less_than(RandomAccessIteratorT first)
	: first_(first)
	{
	}

	bool operator()(const ::std::size_t a, const ::std::size_t b) const
	{
		return *(first_+a) < *(first_+b);
	}

	RandomAccessIteratorT first_;
};


/// Auxiliary comparator used for comparing elements of input sequence during
/// the ordering of indices; elements are compared according to the given
/// comparator functor.
template <typename RandomAccessIteratorT, typename ComparatorT>
struct index_less_than_cmp
{
	index_less_than_cmp(RandomAccessIteratorT first, ComparatorT comp)
	: first_(first),
	  comp_(comp)
	{
	}

	bool operator()(const ::std::size_t a, const ::std::size_t b) const
	{
		return comp_(*(first_+a), *(first_+b));
	}

	RandomAccessIteratorT first_;
	ComparatorT comp_;
};

} // Namespace detail


/**
 * \brief Computes the permutation of indices which makes the given range
 *  ordered.
 * \tparam RandomAccessIteratorT The type of the iterator to the input sequence.
 * \tparam ForwardIteratorT The type of the iterator to the resulting sequence
 *  of indices.
 * \param first The iterator to the beginning of the input sequence.
 * \param last The iterator to the end of the input sequence.
 * \param result The iterator to the beginning of the resulting sequence of
 *  indices.
 *
 * The input sequence is ordered according to the standard operator \c &lt;.
 */
template <typename RandomAccessIteratorT, typename ForwardIteratorT>
void order(RandomAccessIteratorT first, RandomAccessIteratorT last, ForwardIteratorT result)
{
	ForwardIteratorT result_begin = result;
	for (
		RandomAccessIteratorT it = first;
		it != last;
		++it
	) {
		*result = it-first;
		++result;
	}
	
	::std::sort(
		result_begin,
		result,
		detail::index_less_than<RandomAccessIteratorT&>(first)
	);
}


/**
 * \brief Computes the permutation of indices which makes the given range
 *  ordered.
 * \tparam RandomAccessIteratorT The type of the iterator to the input sequence.
 * \tparam ForwardIteratorT The type of the iterator to the resulting sequence
 *  of indices.
 * \tparam ComparatorT The comparator functor.
 * \param first The iterator to the beginning of the input sequence.
 * \param last The iterator to the end of the input sequence.
 * \param result The iterator to the beginning of the resulting sequence of
 *  indices.
 * \param comp An instance of the comparator functor used for ordering the input
 *  sequence.
 *
 * The input sequence is ordered according to the given comparator functor.
 */
template <typename RandomAccessIteratorT, typename ForwardIteratorT, typename ComparatorT>
void order(RandomAccessIteratorT first, RandomAccessIteratorT last, ForwardIteratorT result, ComparatorT comp)
{
	ForwardIteratorT result_begin = result;
	for (
		RandomAccessIteratorT it = first;
		it != last;
		++it
	) {
		*result = it-first;
		++result;
	}
	
	::std::sort(
		result_begin,
		result,
		detail::index_less_than_cmp<RandomAccessIteratorT&, ComparatorT&>(first, comp)
	);
}

}} // Namespace dcs::algorithm


#endif // DCS_ALGORITHM_ORDER_HPP
