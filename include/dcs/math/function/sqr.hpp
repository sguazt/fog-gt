/**
 * \file dcs/math/function/sqr.hpp
 *
 * \brief The square function.
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

#ifndef DCS_MATH_FUNCTION_SQR_HPP
#define DCS_MATH_FUNCTION_SQR_HPP


namespace dcs { namespace math {

/**
 * \brief The square function.
 * \tparam The numerical type of input and output values.
 * \param The numerical value to be squared.
 * \return The square of \a value.
 */
template <typename ValueT>
inline ValueT sqr(ValueT value)
{
	return value*value;
}

}} // Namespace dcs::math


#endif // DCS_MATH_FUNCTION_SQR_HPP
