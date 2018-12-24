/**
 * \file dcs/math/cumsum.hpp
 *
 * \brief Compute the cumulative sum of the given sequence.
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

#ifndef DCS_MATH_FUNCTION_CUMSUM_HPP
#define DCS_MATH_FUNCTION_CUMSUM_HPP


namespace dcs { namespace math {

template <typename T, typename ForwardIterator1T, typename ForwardIterator2T>
void cumsum(ForwardIterator1T first, ForwardIterator1T last, ForwardIterator2T result)
{
	T sum(0);

	while (first != last)
	{
		sum += *first;
		*result = sum;
		++first;
		++result;
	}
}

}} // Namespace dcs::math


#endif // DCS_MATH_FUNCTION_CUMSUM_HPP
