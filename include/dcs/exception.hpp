/**
 * \file dcs/exception.hpp
 *
 * \brief Exceptions support.
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

#ifndef DCS_EXCEPTION_HPP
#define DCS_EXCEPTION_HPP


#include <boost/current_function.hpp>
#include <sstream>
#include <string>


namespace dcs { namespace exception { namespace detail { namespace /*<unnamed>*/ {

inline ::std::string make_msg(::std::string const& m, char const* func_name)
{
	::std::ostringstream oss;
	//oss << "[" << BOOST_CURRENT_FUNCTION << "] " << m;
	oss << "[" << func_name << "] " << m << ".";
	return oss.str();
}

}}}} // Namespace dcs::exception::detail::<unnamed>


//#define DCS_EXCEPTION_THROW(e,m) throw e(::dcs::exception::detail::make_msg(m))
#define DCS_EXCEPTION_THROW(e,m) throw e(::dcs::exception::detail::make_msg(m, BOOST_CURRENT_FUNCTION))

#define DCS_EXCEPTION_RETHROW(e) throw;

#endif // DCS_EXCEPTION_HPP
