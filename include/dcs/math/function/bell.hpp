/**
 * \file inc/dcs/math/function/bell.hpp
 *
 * \brief Compute the n-th Bell number.
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

#ifndef DCS_MATH_FUNCTION_BELL_HPP
#define DCS_MATH_FUNCTION_BELL_HPP


#include <dcs/debug.hpp>
#include <boost/math/special_functions/binomial.hpp>
#include <vector>


namespace dcs { namespace math {

namespace detail { namespace /*<unnamed>*/ {

/**
 * \brief Compute the n-th Bell's number with the recursive formula.
 *
 * \tparam RealT The real type of the return value
 * \param n The order of the Bell's number
 * \return the n-th Bell's number
 *
 * The n-th Bell number \f$B_n\f$ can be computed as:
 * \f{equation}
 *  B_n=sum_(k=0)^(n-1){\binom{n-1}{k} B_k}
 * \f}
 *
 * \note This is a naive implementation that should not be used in practice.
 */
template <typename RealT>
RealT bell_rec(unsigned int n)
{
	if (n <= 1)
	{
		return 1;
	}

	RealT Bn = 0;
	for (unsigned int k = 0; k < n; ++k)
	{
		Bn += ::boost::math::binomial_coefficient<RealT>(n-1, k)*bell_rec<RealT>(k);
	}

	return Bn;
}

/**
 * \brief Compute the n-th Bell's number with the triangle method.
 *
 * \tparam RealT The real type of the return value
 * \param n The order of the Bell's number
 * \return the n-th Bell's number
 *
 * For details about the triangle method see [1].
 *
 * References:
 * -# J. Shallit
 *    "A triangle for the Bell numbers",
 *    In: A collection of manuscripts related to the Fibonacci sequence (eds. V.E. Hoggatt, Jr. and M. Bicknell-Johnson), pp. 69-71, Fibonacci Association, 1980
 *    [http://www.fq.math.ca/Books/Collection/shallit.pdf]
 * .
 */
template <typename RealT>
RealT bell_triangle(unsigned int n)
{
	if (n <= 1)
	{
		return 1;
	}

	::std::vector<RealT> Tup(n); // The upper row of Bell triangle
	::std::vector<RealT> Tlo(n); // The lower row of Bell triangle

    Tup[0] = 1;
	for (unsigned int i = 1; i < n; ++i)
	{
		Tlo[0] = Tup[i-1];
		for (unsigned int k = 1; k <= i; ++k)
		{
			Tlo[k] = Tlo[k-1]+Tup[k-1];
			Tup[k-1] = Tlo[k-1];
		}
		Tup[i] = Tlo[i];
	}

DCS_DEBUG_TRACE("B("<<n<<")="<<Tlo.back());
	return Tlo.back();
}

}} // Namespace detail::<unnamed>

#if 0 // Naive implementation, only used for testing/debugging purpose
/**
 * \brief Compute the n-th Bell number
 *
 * \tparam RealT The real type of the return value
 * \param n The order of the Bell's number
 * \return the n-th Bell's number
 *
 * The n-th Bell number \f$B_n\f$ is the number of ways a set of n elements can
 * be partitioned into nonempty subsets.
 * \f$B_n\f$ can be computed as:
 * \f{equation}
 *  B_n=sum_(k=0)^(n-1){\binom{n-1}{k} B_k}
 * \f}
 */
template <typename RealT>
RealT bell_naive(unsigned int n)
{
	return detail::bell_rec<RealT>(n);
}
#endif // if 0

/**
 * \brief Compute the n-th Bell number
 *
 * \tparam RealT The real type of the return value
 * \param n The order of the Bell's number
 * \return the n-th Bell's number
 *
 * The n-th <em>Bell number</em> \f$B_n\f$ (also called the <em>exponential
 * number</em>) is the number of ways a set of n elements can be partitioned
 * into nonempty subsets.
 * \f$B_n\f$ can be computed as:
 * \f{equation}
 *  B_n=sum_(k=0)^(n-1){\binom{n-1}{k} B_k}
 * \f}
 */
template <typename RealT>
RealT bell(unsigned int n)
{
	return detail::bell_triangle<RealT>(n);
}

}} // Namespace dcs::math

#endif // DCS_MATH_FUNCTION_BELL_HPP
