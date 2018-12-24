/**
 * \file dcs/math/functional/close_to.hpp
 *
 * \brief Functor for checking if two numbers are close to each other (with
 *  respect to floating-point comparison).
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
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

#ifndef DCS_MATH_FUNCTIONAL_CLOSE_TO_HPP
#define DCS_MATH_FUNCTIONAL_CLOSE_TO_HPP


#include <cmath>
#include <functional>
#include <limits>


namespace dcs { namespace math {

/**
 * \brief Check if two real numbers are close each other, with respect to a
 *  given tolerance.
 */
//TODO: add a specialization for complex numbers (hints: use enable_if).
template <typename T>
struct close_to: ::std::binary_function <T,T,bool>
{
	bool operator()(T const& x, T const& y) const
    {
		return this->operator()(x, y, T(100)*::std::numeric_limits<T>::epsilon());
	}

	bool operator()(T const& x, T const& y, T const& tol) const
    {
		if (::std::isnan(x) || ::std::isnan(y))
		{
			// According to IEEE, NaN are different event by itself
			return false;
		}
		return ::std::abs(x-y) <= (::std::max(static_cast<T>(::std::abs(x)), static_cast<T>(::std::abs(y)))*tol);
	}
}; // close_to

}} // Namespace dcs::math


#endif // DCS_MATH_FUNCTIONAL_CLOSE_TO_HPP
