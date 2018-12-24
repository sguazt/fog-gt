/**
 * \file dcs/algorithm/combination.hpp
 *
 * \brief Algorithms for combinations.
 *
 * A combination of size r of a range of size n is a sorted subsequence of size
 * r of the total range, i.e., the ordered (possibly multi-)set of the elements
 * at r positions among the n positions in the range.
 *
 * A combination is without repetition if the r indices in the respective
 * definition are distinct (and necessarily r <= n), and with repetition
 * otherwise.
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

#ifndef DCS_ALGORITHM_COMBINATION_HPP
#define DCS_ALGORITHM_COMBINATION_HPP


#include <algorithm>
#include <cstddef>
#include <dcs/algorithm/detail/combperm.hpp>
#include <dcs/detail/macro_cx11.hpp>
#include <iterator>
#include <limits>
#include <stdexcept>


namespace dcs { namespace algorithm {

template <typename BidirIter, typename Function>
inline
Function for_each_combination(BidirIter first,
							  BidirIter mid,
                     		  BidirIter last,
							  Function f)
{
    detail::bound_range<Function&, BidirIter> wfunc(f, first, mid);
    detail::combine_discontinuous(first,
								  mid,
								  ::std::distance(first, mid),
                                  mid,
								  last,
								  ::std::distance(mid, last),
                                  wfunc);
    return DCS_DETAIL_MACRO_CX11_STD_MOVE_(f);
}

template <typename UIntT>
UIntT count_each_combination(UIntT d1, UIntT d2)
{
    if (d2 < d1)
	{
        ::std::swap(d1, d2);
	}
    if (d1 == 0)
	{
        return 1;
	}
    if (d1 > ::std::numeric_limits<UIntT>::max() - d2)
	{
        throw ::std::overflow_error("overflow in count_each_combination");
	}
    UIntT n = d1 + d2;
    UIntT r = n;
    --n;
    for (UIntT k = 2; k <= d1; ++k, --n)
    {
        // r = r * n / k, known to not not have truncation error
        UIntT g = detail::gcd(r, k);
        r /= g;
        UIntT t = n / (k / g);
        if (r > ::std::numeric_limits<UIntT>::max() / t)
		{
            throw ::std::overflow_error("overflow in count_each_combination");
		}
        r *= t;
    }

    return r;
}

template <typename BidirIter>
inline
::std::size_t count_each_combination(BidirIter first,
									 BidirIter mid,
									 BidirIter last)
{
    return count_each_combination< ::std::size_t >(::std::distance(first, mid),
												   ::std::distance(mid, last));
}

/**
 * \brief Takes a sequence defined by the range [\a first,\a last) such that
 *  [\a first,\a middle) stores a combination, i.e., some sorted subsequence of
 *  [\a first,\a last), and permutes it such that [\a first,\a middle) stores
 *  the next combination of the same size from [\a first,\a last), and
 *  [\a middle,\a last) is sorted.
 *
 * \return \c true if the next combination exists, \c false otherwise.
 *
 * The next combination is found by assuming that the set of all combinations of
 * a given size from [\a first,\a last) is lexicographically sorted with respect
 * to \c operator<.
 * If the next combination does not exist, it transforms [\a first,\a middle)
 * into the smallest combination, leaving the entire range sorted.
 *
 * \pre The type of \a *first shall satisfy the \c Swappable requirements.
 * \pre The ranges [\a first,\a middle) and [\a middle,\a last) shall both be
 *  sorted in ascending order.
 *
 * \note Upon returning \c false, [\a first,\a middle) is back to the smallest
 *  combination, that is, the prefix of the ascendingly sorted range, and the
 *  requirements met for another application of next_combination.
 */
template <typename BidirectionalIterator>
inline
bool next_combination(BidirectionalIterator first,
					  BidirectionalIterator middle,
					  BidirectionalIterator last)
{
	return detail::next_combination(first, middle, middle, last);
}

/**
 * \brief Takes a sequence defined by the range [\a first,\a last) such that
 *  [\a first,\a middle) stores a combination, i.e., some sorted subsequence of
 *  [\a first,\a last), and permutes it such that [\a first,\a middle) stores
 *  the next combination of the same size from [\a first,\a last), and
 *  [\a middle,\a last) is sorted.
 *
 * \return \c true if the next combination exists, \c false otherwise.
 *
 * The next combination is found by assuming that the set of all combinations of
 * a given size from [\a first,\a last) is lexicographically sorted with respect
 * to \a comp.
 * If the next combination does not exist, it transforms [\a first,\a middle)
 * into the smallest combination, leaving the entire range sorted.
 *
 * \pre The type of \a *first shall satisfy the \c Swappable requirements.
 * \pre The ranges [\a first,\a middle) and [\a middle,\a last) shall both be
 *  sorted in ascending order.
 *
 * \note Upon returning \c false, [\a first,\a middle) is back to the smallest
 *  combination, that is, the prefix of the ascendingly sorted range, and the
 *  requirements met for another application of next_combination.
 */
template <typename BidirectionalIterator, typename Compare>
inline
bool next_combination(BidirectionalIterator first,
					  BidirectionalIterator middle,
					  BidirectionalIterator last,
					  Compare comp)
{
	return detail::next_combination(first, middle, middle, last, comp);
}

template <typename BidirectionalIterator>
inline
bool prev_combination(BidirectionalIterator first,
					  BidirectionalIterator middle,
					  BidirectionalIterator last)
{
	return detail::next_combination(middle, last, first, middle);
}

template <typename BidirectionalIterator, typename Compare>
inline
bool prev_combination(BidirectionalIterator first,
					  BidirectionalIterator middle,
					  BidirectionalIterator last,
					  Compare comp)
{
	return detail::next_combination(middle, last, first, middle, comp);
}

template <typename BidirectionalIterator>
bool next_repeat_combination_counts(BidirectionalIterator first,
									BidirectionalIterator last)
{
	BidirectionalIterator current(last);

	while (current != first && *(--current) == 0) ;

	if (current == first)
	{
		if (first != last && * first != 0)
		{
			::std::iter_swap(--last, first);
		}
		return false;
	}
	--(*current);
	::std::iter_swap(--last, current);
	++(*(--current));

	return true ;
}

template <typename BidirectionalIterator>
bool prev_repeat_combination_counts(BidirectionalIterator first,
									BidirectionalIterator last)
{
	if (first == last)
	{
		return false;
	}
	BidirectionalIterator current(--last);
	while (current != first && *(--current) == 0) ;

	if (current == last || current == first && *current == 0)
	{
		if (first != last)
		{
			::std::iter_swap(first , last);
		}
		return false ;
	}
	--(*current);
	++current;
	if (0 != * last )
	{
		::std::iter_swap(current , last);
	}
	++(*current);
	return true ;
}

/*
class lexicographic_variation
{
	public: explicit lexicographic_variation(::std::size_t n)
	: n_(n)
	{
		DCS_ASSERT(n_ > 0,
				   DCS_EXCEPTION_THROW(::std::invalid_argument,
									   "Number of elements must be positive"));
	}


	private: ::std::size_t n_;
}
*/

}} // Namespace dcs::algorithm


#endif // DCS_ALGORITHM_COMBINATION_HPP
