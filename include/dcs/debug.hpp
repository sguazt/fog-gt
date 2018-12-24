/**
 * \file dcs/debug.hpp
 *
 * \brief Provides helper functions and macros for debugging purpose.
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

#ifndef DCS_DEBUG_HPP
#define DCS_DEBUG_HPP

#if !defined(NDEBUG) && !defined(DCS_NO_DEBUG)
//@{ for C99 and C++0x
//# 	include <cstdio>
//# 	define DCS_DEBUG_WHERESTR_  "[file %s, function %s, line %d]: "
//#	define DCS_DEBUG_WHEREARG_  __FILE__, __func__, __LINE__
//#	define DCS_DEBUG_PRINT2_(...)       fprintf(stderr, __VA_ARGS__)
//#	define DCS_DEBUG_PRINT(fmt, ...)  DCS_DEBUG_PRINT2_(DCS_DEBUG_WHERESTR_ fmt, DCS_DEBUG_WHEREARG_, __VA_ARGS__)
//@} for C99 and C++0x

#   include <algorithm>
#	include <cassert>
#	include <iostream>
#	include <iterator>
# 	include <set>
# 	include <sstream>
# 	include <string>
# 	include <typeinfo>
# 	include <vector>

/// Macro for telling whether we are in debug mode.
# 	define DCS_DEBUG /**/
/// Macro for getting the underlying debug output stream.
#	ifndef DCS_DEBUG_STREAM
# 		define DCS_DEBUG_STREAM ::std::cerr
#	endif // DCS_DEBUG_STREAM
/// Macro for expanding its argument \c x
#	define DCS_DEBUG_EXPAND__(x) x
/// Macro for writing to standard error its argument \c x
#	if !defined(DCS_DEBUG_LEVEL) || (DCS_DEBUG_LEVEL > 0)
#		define DCS_DEBUG_TRACE(x) DCS_DEBUG_STREAM << "[Debug (" << __FILE__ << ":" << __func__ << ":" << __LINE__ << ")>> " << DCS_DEBUG_EXPAND__(x) << ::std::endl;
#	else
#		define DCS_DEBUG_TRACE(x) /**/
#	endif // !defined(DCS_DEBUG_LEVEL) ...
/// Macro for writing to standard error its argument \c x only if the debugging level is greater than or equal to \c l
#	ifdef DCS_DEBUG_LEVEL
#		define DCS_DEBUG_TRACE_L(l,x) if (DCS_DEBUG_EXPAND__(l) <= DCS_DEBUG_LEVEL) { DCS_DEBUG_STREAM << "[Debug (" << __FILE__ << ":" << __func__ << ":" << __LINE__ << ")>> " << DCS_DEBUG_EXPAND__(x) << ::std::endl; }
#	else
#		define DCS_DEBUG_TRACE_L(l,x) DCS_DEBUG_TRACE(x)
#	endif // DCS_DEBUG_LEVEL
/// Macro for writing to standard error its argument \c x only if condition \c c is \c true
#   define DCS_DEBUG_TRACE_IF(c,x) if ((c)) { DCS_DEBUG_TRACE(x); }
/// Macro for setting flags \c x of the underlying debug output stream.
# 	define DCS_DEBUG_STREAM_SETFLAGS(x) DCS_DEBUG_STREAM.setf((x))
/// Macro for converting the given argumento into a string.
# 	define DCS_DEBUG_STRINGIFY(x) #x
/// Macro for converting the a type into a string.
# 	define DCS_DEBUG_TYPE2STR(x) typeid(x).name()
/// Macro for executing one or more statements only when debug is enabled.
#	define DCS_DEBUG_DO(x) x
/// Macro for assertion
#	define DCS_DEBUG_ASSERT(x) assert(x)

#else // NDEBUG

# 	include <string>

/// Macro for telling that we are in debug mode.
# 	undef DCS_DEBUG /**/
/// Macro for telling what level of debugging to enable.
# 	undef DCS_DEBUG_LEVEL /**/
/// Macro for writing to standard error its argument \c x
#	define DCS_DEBUG_TRACE(x) /**/
/// Macro for writing to standard error its argument \c x only when the
/// debugging level is greater than or equal to \c l.
#	define DCS_DEBUG_TRACE_L(l,x) /**/
/// Macro for getting the underlying debug output stream.
# 	define DCS_DEBUG_STREAM /**/
/// Macro for setting flags \c x of the underlying debug output stream.
# 	define DCS_DEBUG_STREAM_SETFLAGS(x) /**/
/// Macro for converting the given argumento into a string.
# 	define DCS_DEBUG_STRINGIFY(x) /**/
/// Macro for converting the a type into a string.
# 	define DCS_DEBUG_TYPE2STR(x) /**/
/// Macro for executing one or more statements only when debug is enabled.
#	define DCS_DEBUG_DO(x) /**/
/// Macro for assertion
#	define DCS_DEBUG_ASSERT(x) /**/
#endif // NDEBUG

namespace dcs { namespace debug {

#ifdef DCS_DEBUG

template <typename FwdItT>
::std::string to_string(FwdItT first, FwdItT last)
{
	typedef typename ::std::iterator_traits<FwdItT>::value_type value_type;

    ::std::ostringstream oss;
    ::std::copy(first,
                last,
                ::std::ostream_iterator<value_type>(oss, " "));

    return oss.str();
}

template <typename T>
::std::string to_string(T const& t)
{
    ::std::ostringstream oss;
	oss << t;
    return oss.str();
}

template <typename T>
::std::string to_string(::std::vector<T> const& v)
{
	//return to_string(v.begin(), v.end());
	const std::size_t n = v.size();
    ::std::ostringstream oss;
	oss << "[";
	for (std::size_t i = 0; i < n; ++i)
	{
		if (i > 0)
		{
			oss << ",";
		}
		oss << v[i];
	}
	oss << "]";
	return oss.str();
}

template <typename T>
::std::string to_string(::std::set<T> const& s)
{
	return to_string(s.begin(), s.end());
}

#else // DCS_DEBUG

template <typename FwdItT>
::std::string to_string(FwdItT first, FwdItT last)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( first );
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( last );

    return "";
}

template <typename T>
::std::string to_string(T const& t)
{
	DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( t );

    return "";
}

#endif // DCS_DEBUG

}} // Namespace dcs::debug

#endif // DCS_DEBUG_HPP
