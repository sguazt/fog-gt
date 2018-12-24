/**
 * \file dcs/algorithm/reorder.hpp
 *
 * \brief Reorder a given sequence according to a permutation of indices.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 *
 * <hr/>
 *
 * Copyright 2013 Marco Guazzone (marco.guazzone@gmail.com)
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

#ifndef DCS_ALGORITHM_REORDER_HPP
#define DCS_ALGORITHM_REORDER_HPP


#include <algorithm>
#include <dcs/debug.hpp>
#include <iterator>
#include <vector>


namespace dcs { namespace algorithm {

/**
 * \brief Applies the permutation of indices to the given sequence.
 */
template <typename IdxFwdIterT, typename InFwdIterT, typename OutFwdIterT>
void reorder_copy(IdxFwdIterT idx_first, IdxFwdIterT idx_last, InFwdIterT in_first, OutFwdIterT out_first)
{
	while (idx_first != idx_last)
	{
		InFwdIterT in_it(in_first);
		::std::advance(in_it, *idx_first);
		*out_first = *in_it;
		++idx_first;
		++out_first;
	}
}

template <typename IdxFwdIterT, typename InFwdIterT>
void reorder(IdxFwdIterT idx_first, IdxFwdIterT idx_last, InFwdIterT in_first)
{
	typedef typename ::std::iterator_traits<InFwdIterT>::value_type value_type;

	InFwdIterT in_last(in_first);
	::std::advance(in_last, ::std::distance(idx_first, idx_last));
	::std::vector<value_type> tmp(in_first, in_last);
	::std::size_t i(0);
	while (idx_first != idx_last)
	{
		*in_first = tmp[*idx_first];

		++idx_first;
		++in_first;
		++i;
	}
}

}} // Namespace dcs::algorithm


#endif // DCS_ALGORITHM_REORDER_HPP
