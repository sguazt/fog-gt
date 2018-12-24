/**
 * \file inc/dcs/math/function/iszero.hpp
 *
 * \brief Functions to test for the zero.
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

#ifndef DCS_MATH_FUNCTION_ISZERO_HPP
#define DCS_MATH_FUNCTION_ISZERO_HPP


#include <cmath>
#include <complex>
//#include <limits>


namespace dcs { namespace math {

/**
 * \brief Tells if the given real number is zero.
 *
 * \tparam RealT The real type of the argument
 * \param x The number to test for zero
 * \return \c true if \a x is zero, \c false otherwise.
 */
template <typename RealT>
bool iszero(RealT x)
{
    //return ::std::fabs(x) < ::std::numeric_limits<RealT>::epsilon();
    return ::std::fpclassify(x) == FP_ZERO;
}

/**
 * \brief Tells if the given complex number is zero.
 *
 * \tparam RealT The real type of the complex argument
 * \param z The number to test for zero
 * \return \c true if \a z is zero, \c false otherwise.
 */
template <typename RealT>
bool iszero(::std::complex<RealT> const& z)
{
    return iszero(z.real()) && iszero(z.imag());
}

}} // Namespace dcs::math

#endif // DCS_MATH_FUNCTION_ISZERO_HPP
