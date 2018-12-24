/**
 * \file dcs/math/sum.hpp
 *
 * \brief Compute the sum of the given sequence.
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

#ifndef DCS_MATH_FUNCTION_SUM_HPP
#define DCS_MATH_FUNCTION_SUM_HPP


namespace dcs { namespace math {

//template <typename ForwardIteratorT, typename T>
template <typename T, typename ForwardIteratorT>
T sum(ForwardIteratorT first, ForwardIteratorT last)
{
	T sum(0);

	while (first != last)
	{
		sum += *first;
		++first;
	}

	return sum;
}

}} // Namespace dcs::math


#endif // DCS_MATH_FUNCTION_SUM_HPP
