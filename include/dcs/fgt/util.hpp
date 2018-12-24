/* vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */

/**
 * \file dcs/fgt/util.hpp
 *
 * \brief Various utility functions.
 *  
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 *  
 * <hr/>
 *  
 * Copyright 2017 Marco Guazzone (marco.guazzone@gmail.com)
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

#ifndef DCS_FGT_UTIL_HPP
#define DCS_FGT_UTIL_HPP


#include <cmath>
#include <dcs/math/function/sign.hpp>
#include <sstream>
#include <string>


namespace dcs { namespace fgt {

template <typename T>
std::string stringify(const T& t)
{   
    std::ostringstream oss;
        
    oss << t;

    return oss.str();
}

template <typename T>
T relative_increment(T x, T r)
{
    return math::sign(x)*std::abs(x/r)-math::sign(r);
}

}} // Namespace dcs::fgt


#endif // DCS_FGT_UTIL_HPP
