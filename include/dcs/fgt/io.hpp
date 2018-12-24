/* vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */

/**
 * \file dcs/fgt/io.hpp
 *
 * \brief Input/Output utilities.
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

#ifndef DCS_FGT_IO_HPP
#define DCS_FGT_IO_HPP


#include <iostream>
#include <vector>


namespace dcs { namespace fgt {

template <typename CharT, typename CharTraitsT, typename T>
std::basic_ostream<CharT,CharTraitsT>& operator<<(std::basic_ostream<CharT,CharTraitsT>& os, const std::vector<T>& v)
{
    typedef typename std::vector<T>::const_iterator iterator;

    os << "[";
    for (iterator it = v.begin(), end_it = v.end();
         it != end_it;
         ++it)
    {
        if (it != v.begin())
        {
            os << ", ";
        }
        os << *it;
    }
    os << "]";

    return os;
}

}} // Namespace dcs::fgt

#endif // DCS_FGT_IO_HPP
