/**
 * \file dcs/math/traits/float.hpp
 *
 * \brief Traits class for floating-point type.
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

#ifndef DCS_MATH_TRAITS_FLOAT_HPP
#define DCS_MATH_TRAITS_FLOAT_HPP


#include <boost/type_traits/is_floating_point.hpp>
#include <boost/utility/enable_if.hpp>
#include <dcs/math/detail/float.hpp>
#include <limits>


namespace dcs { namespace math {

template <typename T, typename Enable_ = void>
struct float_traits;

template <typename T>
struct float_traits<T, typename ::boost::enable_if< ::boost::is_floating_point<T> >::type>
{
	/// Default tolerance for floating-point comparison.
	static const T tolerance;

	static bool approximately_equal(T x, T y, T tol)
	{
		return detail::approximately_equal(x, y, tol);
	}


	static bool approximately_equal(T x, T y)
	{
		return detail::approximately_equal(x, y, tolerance);
	}


	static bool essentially_equal(T x, T y, T tol)
	{
		return detail::essentially_equal(x, y, tol);
	}


	static bool essentially_equal(T x, T y)
	{
		return detail::essentially_equal(x, y, tolerance);
	}


	static bool definitely_less(T x, T y, T tol)
	{
		return detail::definitely_less(x, y, tol);
	}


	static bool definitely_less(T x, T y)
	{
		return detail::definitely_less(x, y, tolerance);
	}


	/// \deprecated Use \c approximately_less_equal or \c essentially_less_equal
	static bool definitely_less_equal(T x, T y, T tol)
	{
		return definitely_less(x, y, tol) || approximately_equal(x, y, tol);
	}


	/// \deprecated Use \c approximately_less_equal or \c essentially_less_equal
	static bool definitely_less_equal(T x, T y)
	{
		return definitely_less_equal(x, y, tolerance);
	}


	static bool approximately_less_equal(T x, T y, T tol)
	{
		return definitely_less(x, y, tol) || approximately_equal(x, y, tol);
	}


	static bool approximately_less_equal(T x, T y)
	{
		return approximately_less_equal(x, y, tolerance);
	}


	static bool essentially_less_equal(T x, T y, T tol)
	{
		return definitely_less(x, y, tol) || essentially_equal(x, y, tol);
	}


	static bool essentially_less_equal(T x, T y)
	{
		return essentially_less_equal(x, y, tolerance);
	}


	static bool definitely_greater(T x, T y, T tol)
	{
		return detail::definitely_greater(x, y, tol);
	}


	static bool definitely_greater(T x, T y)
	{
		return detail::definitely_greater(x, y, tolerance);
	}


	/// \deprecated Use \c approximately_less_equal or \c essentially_less_equal
	static bool definitely_greater_equal(T x, T y, T tol)
	{
		return definitely_greater(x, y, tol) || approximately_equal(x, y, tol);
	}


	/// \deprecated Use \c approximately_greater_equal or \c essentially_greater_equal
	static bool definitely_greater_equal(T x, T y)
	{
		return definitely_greater_equal(x, y, tolerance);
	}

	static bool approximately_greater_equal(T x, T y, T tol)
	{
		return definitely_greater(x, y, tol) || approximately_equal(x, y, tol);
	}


	static bool approximately_greater_equal(T x, T y)
	{
		return approximately_greater_equal(x, y, tolerance);
	}

	static bool essentially_greater_equal(T x, T y, T tol)
	{
		return definitely_greater(x, y, tol) || essentially_equal(x, y, tol);
	}


	static bool essentially_greater_equal(T x, T y)
	{
		return essentially_greater_equal(x, y, tolerance);
	}

	/// \deprecated Use \c definitely_min
	static T min(T x, T y, T tol = tolerance)
	{
		if (definitely_less(x, y, tol))
		{
			return x;
		}
		return y;
	}

	/// \deprecated Use \c definitely_max
	static T max(T x, T y, T tol = tolerance)
	{
		if (definitely_greater(x, y, tol))
		{
			return x;
		}
		return y;
	}

	static T definitely_min(T x, T y, T tol = tolerance)
	{
		if (definitely_less(x, y, tol))
		{
			return x;
		}
		return y;
	}

	static T definitely_max(T x, T y, T tol = tolerance)
	{
		if (definitely_greater(x, y, tol))
		{
			return x;
		}
		return y;
	}

/*
bool is_nan(float f)
{
    return (*reinterpret_cast<uint32_t*>(&f) & 0x7f800000) == 0x7f800000 && (*reinterpret_cast<uint32_t*>(&f) & 0x007fffff) != 0;
}

bool is_finite(float f)
{
    return (*reinterpret_cast<uint32_t*>(&f) & 0x7f800000) != 0x7f800000;
}

// if this symbol is defined, NaNs are never equal to anything (as is normal in IEEE floating point)
// if this symbol is not defined, NaNs are hugely different from regular numbers, but might be equal to each other
#define UNEQUAL_NANS 1
// if this symbol is defined, infinites are never equal to finite numbers (as they're unimaginably greater)
// if this symbol is not defined, infinities are 1 ULP away from +/- FLT_MAX
#define INFINITE_INFINITIES 1
//
// test whether two IEEE floats are within a specified number of representable values of each other
// This depends on the fact that IEEE floats are properly ordered when treated as signed magnitude integers
bool equal_float(float lhs, float rhs, uint32_t max_ulp_difference)
{
#ifdef UNEQUAL_NANS
	if(is_nan(lhs) || is_nan(rhs))
	{
		return false;
	}
#endif
#ifdef INFINITE_INFINITIES
	if((is_finite(lhs) && !is_finite(rhs)) || (!is_finite(lhs) && is_finite(rhs)))
	{
		return false;
	}
#endif
	int32_t left(*reinterpret_cast<int32_t*>(&lhs));
	// transform signed magnitude ints into 2s complement signed ints
	if(left < 0)
	{
		left = 0x80000000 - left;
	}
	int32_t right(*reinterpret_cast<int32_t*>(&rhs));
	// transform signed magnitude ints into 2s complement signed ints
	if(right < 0)
	{
		right = 0x80000000 - right;
	}
	if(static_cast<uint32_t>(std::abs(left - right)) <= max_ulp_difference)
	{
		return true;
	}
	return false;
}
*/
};

template <typename T>
const T float_traits<T, typename ::boost::enable_if< ::boost::is_floating_point<T> >::type>::tolerance = static_cast<T>(100)*::std::numeric_limits<T>::epsilon();

}} // Namespace dcs::math


#endif // DCS_MATH_TRAITS_FLOAT_HPP
