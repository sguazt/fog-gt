/**
 * \file dcs/cli/simple.hpp
 *
 * \brief A minimal set of functions for a trivial parsing of command line
 *  options passed to programs.
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

#ifndef DCS_CLI_SIMPLE_HPP
#define DCS_CLI_SIMPLE_HPP


#include <algorithm>
#include <dcs/exception.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>


namespace dcs { namespace cli { namespace simple {

/// Find the option \a opt inside the given range of iterators.
template <typename FwdIterT>
FwdIterT find_option(FwdIterT first, FwdIterT last, ::std::string const& opt)
{
	FwdIterT it = ::std::find(first, last, opt);

	return (it != last) ? it : last;
}

/// Get the value of the option \a opt inside the given range of iterators.
template <typename T, typename FwdIterT>
T get_option(FwdIterT first, FwdIterT last, ::std::string const& opt)
{
	FwdIterT it = find_option(first, last, opt);

	if (it == last || ++it == last)
	{
		::std::ostringstream oss;
		oss << "Unable to find option: '" << opt << "'";

		DCS_EXCEPTION_THROW(::std::runtime_error, oss.str());
	}

	T value;

	::std::istringstream iss(*it);
	iss >> value;

	return value;
}

/**
 * \brief Get the value of the option \a opt inside the given range of
 *  iterators.
 *
 * \return The value of the found option, or the given default value if it is
 *  not found.
 */
template <typename T, typename FwdIterT>
T get_option(FwdIterT first, FwdIterT last, ::std::string const& opt, T default_value)
{
	FwdIterT it = find_option(first, last, opt);

	T value(default_value);

	if (it != last && ++it != last)
	{
		::std::istringstream iss(*it);
		iss >> value;
	}

	return value;
}

/// Get a boolean option; also tell if a given option does exist.
template <typename FwdIterT>
bool get_option(FwdIterT first, FwdIterT last, ::std::string const& opt)
{
	FwdIterT it = find_option(first, last, opt);

	return it != last;
}

template <typename T, typename FwdIterT>
::std::vector<T> get_options(FwdIterT first, FwdIterT last, ::std::string const& opt)
{
	::std::vector<T> values;
	bool ko(true);

	FwdIterT it = first;

	while (it != last)
	{
		it = find_option(it, last, opt);

		if (it == last)
		{
			// This is OK if we have already found an occurrence of this option.
			// Otherwise, it is not OK
			break;
		}
		if (++it == last)
		{
			// This is not OK since it is a malformed option
			ko = true;
			break;
		}

		T value;

		::std::istringstream iss(*it);
		iss >> value;

		values.push_back(value);

		ko = false;
	}

	if (ko)
	{
		::std::ostringstream oss;
		oss << "Unable to find option: '" << opt << "'";

		DCS_EXCEPTION_THROW(::std::runtime_error, oss.str());
	}

	return values;
}

template <typename T, typename FwdIterT>
::std::vector<T> get_options(FwdIterT first, FwdIterT last, ::std::string const& opt, T default_value)
{
	::std::vector<T> values;

	FwdIterT it = first;

	while (it != last)
	{
		it = find_option(it, last, opt);

		if (it != last && ++it != last)
		{
			::std::istringstream iss(*it);
			T value;
			iss >> value;
			values.push_back(value);
		}
	}

	if (values.size() == 0)
	{
		values.push_back(default_value);
	}

	return values;
}

}}} // Namespace dcs::cli::simple

#endif // DCS_CLI_SIMPLE_HPP
