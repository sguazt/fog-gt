/**
 * \file dcs/math/function/polyval.hpp
 *
 * \brief Evaluation of polynomials.
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

#ifndef DCS_MATH_FUNCTION_POLYVAL_HPP
#define DCS_MATH_FUNCTION_POLYVAL_HPP

namespace dcs { namespace math {

/*
 * \brief Evaluate polynomials in a numerical robust way using the Horner
 *  algorithm.
 *
 * Given a polynomial
 *   \f$P(x)=c_0 + c_1 x + \cdots + c_n x^n\f$
 * the function returns the evaluation of \f$P(x)\f$ for a certain value of
 * \f$x\f$.
 *
 * \tparam RealT The real type.
 * \tparam IterT The coefficient iterator type
 * \param first_coef The iterator to the beginning of the coefficient sequence (i.e., to \f$c_0\f$)
 * \param last_coef The iterator to the end of the coefficient sequence
 * \param The value of the polynomial variable
 * \return The evaluation of the polynomial at the given value \a x.
 */
template <typename RealT, typename IterT>
RealT polyval(IterT first_coef, IterT last_coef, RealT x)
{
	RealT res = 0;
	while (first_coef != last_coef)
	{
		--last_coef;
		res = *last_coef + res*x;
	}

	return res;
}

}} // Namespace dcs::math

#endif // DCS_MATH_FUNCTION_POLYVAL_HPP
