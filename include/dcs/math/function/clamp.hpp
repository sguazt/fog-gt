/**
 * \file dcs/math/function/clamp.hpp
 *
 * \brief Functions to clamp a value either up or down.
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

#ifndef DCS_MATH_FUNCTION_CLAMP_HPP
#define DCS_MATH_FUNCTION_CLAMP_HPP


namespace dcs { namespace math {

/// Bind \a x to be not less than \a lb.
template <typename T>
inline
T clamp_down(T x, T lb)
{
	if (x < lb)
	{
		x = lb;
	}

	return x;
}

/// Bind \a x to be not greater than \a ub.
template <typename T>
inline
T clamp_up(T x, T ub)
{
	if (x > ub)
	{
		x = ub;
	}

	return x;
}

/// Bind \a x to be not less than \a lb and greater than \a ub.
template <typename T>
inline
T clamp(T x, T lb, T ub)
{
	return clamp_down(clamp_up(x, ub), lb);

	return x;
}

/// Tells if \a lb is a lower bound of \a x.
template <typename T>
inline
bool clamped_down(T x, T lb)
{
	return x >= lb;
}

/// Tells if \a ub is an upper bound of \a x.
template <typename T>
inline
bool clamped_up(T x, T ub)
{
	return x <= ub;
}

/**
 * \brief Tells if \a lb and \a ub are a lower and an upper bound of \a x,
 *  respectively.
 */
template <typename T>
inline
bool clamped(T x, T lb, T ub)
{
	return clamped_down(x, lb) && clamped_up(x, ub);
}

}} // Namespace dcs::math


#endif // DCS_MATH_FUNCTION_CLAMP_HPP
