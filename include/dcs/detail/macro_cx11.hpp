/**
 * \file dcs/detail/cx11_macro.hpp
 *
 * \brief A collection of macros that replace C++11 functions/types when they
 *  are not available.
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

#ifndef DCS_DETAIL_MACRO_CX11_HPP
#define DCS_DETAIL_MACRO_CX11_HPP

#if __cplusplus >= 201103L // C++11 or new

#include <cstdint>
#include <iterator>
#include <utility>

# define DCS_DETAIL_MACRO_CX11_STD_MOVE_ ::std::move
# define DCS_DETAIL_MACRO_CX11_STD_NEXT_ ::std::next
# define DCS_DETAIL_MACRO_CX11_STD_PREV_ ::std::prev
# define DCS_DETAIL_MACRO_CX11_STD_UINTMAX_T_ ::std::uintmax_t

#else // Old C++ standard

#include <boost/cstdint.hpp>
#include <boost/move/move.hpp>
#include <boost/next_prior.hpp>

# define DCS_DETAIL_MACRO_CX11_STD_MOVE_ ::boost::move
# define DCS_DETAIL_MACRO_CX11_STD_NEXT_ ::boost::next
# define DCS_DETAIL_MACRO_CX11_STD_PREV_ ::boost::prior
# define DCS_DETAIL_MACRO_CX11_STD_UINTMAX_T_ ::boost::uintmax_t

#endif // __cplusplus

#endif // DCS_DETAIL_MACRO_CX11_HPP
