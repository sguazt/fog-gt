/**
 * \file dcs/math/function/lerp.hpp
 *
 * \brief Linear interpolation.
 *
 * Perform the linear interpolation between two inputs (x0,x1) for a parameter
 * (t) in the range [0,1] 
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

#ifndef DCS_MATH_FUNCTION_LERP_HPP
#define DCS_MATH_FUNCTION_LERP_HPP


namespace dcs { namespace math {

/**
 * Computes the linear combination of x0 and x1 with parameter t:
 *  (1-t)*x0 + t*x1
 */
template <typename RealT> 
inline 
RealT lerp(RealT x0, RealT x1, RealT t) 
{ 
	return x0+(x1-x0)*t; 
} 

}} // Namespace dcs::math


#endif // DCS_MATH_FUNCTION_LERP_HPP
