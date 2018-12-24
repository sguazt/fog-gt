/**
 * \file dcs/math/function/sign.hpp
 *
 * \brief Compute the \e signum function.
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

#ifndef DCS_MATH_FUNCTION_SIGN_HPP
#define DCS_MATH_FUNCTION_SIGN_HPP


#include <cmath>
#include <complex>
#include <dcs/math/function/iszero.hpp>


namespace dcs { namespace math {

/**
 * \brief Compute the \e signum function.
 *
 * The signum function is defined as
 * \f{equation}{
 * sign(x) = \begin{cases}
 *           -1, & x < 0,\\
 *            0, & x = 0,\\
 *           +1, & x > 0.
 *           \end{cases}
 *\f}
 *
 * For complex arguments, the signum function is defined as:
 * \f{equation}{
 * sign(z) = \frac{z}{|z|}
 *\f}
 */
template <typename T>
T sign(T t);

template <typename T>
T sign(T t)
{
	if (iszero(t))
	{
		return 0;
	}
	else
	{
		return (t < 0) ? T(-1) : T(1);
	}
}

template <typename RealT>
::std::complex<RealT> sign(::std::complex<RealT> const& c)
{
	typedef ::std::complex<RealT> complex_type;

	return	(c == 0)
			? complex_type(0)
			: complex_type(c/::std::abs(c));
}

}} // Namespace dcs::math


#endif // DCS_MATH_FUNCTION_SIGN_HPP
