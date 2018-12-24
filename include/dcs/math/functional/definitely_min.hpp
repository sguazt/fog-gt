/**
 * \file dcs/math/functional/definitely_min.hpp
 *
 * \brief Functor for returning the definitely minimum of two numbers
 *  (with respect to floating-point comparison).
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 *
 * <hr/>
 *
 * Copyright 2014 Marco Guazzone (marco.guazzone@gmail.com)
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

#ifndef DCS_MATH_FUNCTIONAL_DEFINITELY_MIN_HPP
#define DCS_MATH_FUNCTIONAL_DEFINITELY_MIN_HPP


#include <dcs/math/traits/float.hpp>
#include <functional>


namespace dcs { namespace math {

template <typename T>
struct definitely_min: ::std::binary_function <T,T,T>
{
	T operator()(T const& x, T const& y) const
    {
		return float_traits<T>::definitely_min(x, y);
	}

	T operator()(T const& x, T const& y, T const& tol) const
    {
		return float_traits<T>::definitely_min(x, y, tol);
	}
};

}} // Namespace dcs::math


#endif // DCS_MATH_FUNCTIONAL_DEFINITELY_MIN_HPP
