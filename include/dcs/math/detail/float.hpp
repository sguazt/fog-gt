/**
 * \file dcs/math/detail/float.hpp
 *
 * \brief Utilities for floating-point comparison.
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

#ifndef DCS_MATH_DETAIL_FLOAT_HPP
#define DCS_MATH_DETAIL_FLOAT_HPP


#include <algorithm>
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/utility/enable_if.hpp>
//#include <cfloat>
#include <cmath>


namespace dcs { namespace math { namespace detail {

/// See also:
/// - http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
/// - http://www.petebecker.com/js/js200012.html
/// - http://code.google.com/p/googletest/source/browse/trunk/include/gtest/internal/gtest-internal.h
/// - http://www.parashift.com/c++-faq-lite/newbie.html#faq-29.16
/// - http://adtmag.com/articles/2000/03/16/comparing-floats-how-to-determine-if-floating-quantities-are-close-enough-once-a-tolerance-has-been.aspx
/// - http://www.boost.org/doc/libs/1_47_0/libs/test/doc/html/utf/testing-tools/floating_point_comparison.html
/// - http://learningcppisfun.blogspot.com/2010/04/comparing-floating-point-numbers.html
/// - http://floating-point-gui.de/errors/comparison/
/// - https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/
/// .


/**
 * \brief x is approximately equal to y.
 *
 * Inspired by [1]:
 * \f[
 *  $x \approx y\,\text{ if and only if } |y-x|\le\epsilon\max(|x|,|y|)
 * \f]
 *
 * References:
 * -# Knuth, "The Art of Computer Programming: Vol.2" 3rd Ed, 1998, Sec. 4.2.2.
 * .
 */
template <typename T>
inline
typename ::boost::enable_if<
	::boost::is_floating_point<T>,
	bool
>::type approximately_equal(T x, T y, T tol)
{
	// Try first with standard comparison (handles the case when both x and y are zero)
	if (x == y)
	{
		return true;
	}

	// Handle degenerate cases
	if (::std::isnan(x) || ::std::isnan(y))
	{
		// According to IEEE, NaN are different even by itself
		return false;
	}
	if (::std::isinf(x) && ::std::isinf(y))
	{
		// According to IEEE, Infinite operands of the same sign shall compare equal
		return true;
	}
	if ((::std::isinf(x) && ::std::isfinite(y)) || (::std::isfinite(x) && ::std::isinf(y)))
	{
		// Infinity vs non-infinite operands are different
		return false;
	}

	return ::std::abs(x-y) <= (::std::max(::std::abs(x), ::std::abs(y))*tol);
}


/**
 * \brief x is definitely equal to y.
 *
 * Inspired by [1]:
 * \f[
 *  $x \sim y\,\text{ if and only if } |y-x|\le\epsilon\min(|x|,|y|)
 * \f]
 *
 * References:
 * -# Knuth, "The Art of Computer Programming: Vol.2" 3rd Ed, 1998, Sec. 4.2.2.
 * .
 */
template <typename T>
inline
typename ::boost::enable_if<
	::boost::is_floating_point<T>,
	bool
>::type essentially_equal(T x, T y, T tol)
{
	// Try first with standard comparison (handles the case when both x and y are zero)
	if (x == y)
	{
		return true;
	}
	// Handle degenerate cases
	if (::std::isnan(x) || ::std::isnan(y))
	{
		// According to IEEE, NaN are different even by itself
		return false;
	}
	if (::std::isinf(x) && ::std::isinf(y))
	{
		// According to IEEE, Infinite operands of the same sign shall compare equal
		return true;
	}
	if ((::std::isinf(x) && ::std::isfinite(y)) || (::std::isfinite(x) && ::std::isinf(y)))
	{
		// Infinity vs non-infinite operands are different
		return false;
	}

	return ::std::abs(x-y) <= (::std::min(::std::abs(x), ::std::abs(y))*tol);
}


/**
 * \brief x is definitely greater than y.
 *
 * Inspired by [1]:
 * \f[
 *  $x \succ y\,\text{ if and only if } x-y > \epsilon\max(|x|,|y|)
 * \f]
 *
 * References:
 * -# Knuth, "The Art of Computer Programming: Vol.2" 3rd Ed, 1998, Sec. 4.2.2.
 * .
 */
template <typename T>
inline
typename ::boost::enable_if<
	::boost::is_floating_point<T>,
	bool
>::type definitely_greater(T x, T y, T tol)
{
	// Try first with standard comparison
	if (x <= y)
	{
		return false;
	}

	// Handle degenerate cases
	if (::std::isnan(x) || ::std::isnan(y))
	{
		// According to IEEE, NaN are different even by itself
		return false;
	}
	if (::std::isinf(x) && ::std::isfinite(y))
	{
		return true;
	}
	if (::std::isfinite(x) && ::std::isinf(y))
	{
		return false;
	}

	return (x-y) > (::std::max(::std::abs(x), ::std::abs(y))*tol);
}


/**
 * \brief x is definitely less than y.
 *
 * Inspired by [1]:
 * \f[
 *  $x \prec y\,\text{ if and only if } y-x > \epsilon\max(|x|,|y|)
 * \f]
 *
 * References:
 * -# Knuth, "The Art of Computer Programming: Vol.2" 3rd Ed, 1998, Sec. 4.2.2.
 * .
 */
template <typename T>
inline
typename ::boost::enable_if<
	::boost::is_floating_point<T>,
	bool
>::type definitely_less(T x, T y, T tol)
{
	// Try first with standard comparison
	if (x >= y)
	{
		return false;
	}

	// Handle degenerate cases
	if (::std::isnan(x) || ::std::isnan(y))
	{
		// According to IEEE, NaN are different even by itself
		return false;
	}
	if (::std::isinf(x) && ::std::isfinite(y))
	{
		return false;
	}
	if (::std::isfinite(x) && ::std::isinf(y))
	{
		return true;
	}

	return (y-x) > (::std::max(::std::abs(x), ::std::abs(y))*tol);
}

}}} // Namespace dcs::math::detail


#endif // DCS_MATH_DETAIL_FLOAT_HPP
