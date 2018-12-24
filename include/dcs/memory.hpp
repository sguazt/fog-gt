/**
 * \file dcs/memory.hpp
 *
 * \brief Smart pointers are objects which store pointers to dynamically
 * allocated (heap) objects.
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

#ifndef DCS_SMART_PTR_HPP
#define DCS_SMART_PTR_HPP

#include <dcs/macro.hpp>

#ifdef DCS_MACRO_CXX11
// C++11 has smart-pointers
# 	include <memory>
# 	define DCS_MEMORY_NS_ ::std
#else
// Use Boost smart-pointers
# 	include <dcs/detail/config/boost.hpp>
# 	if !DCS_DETAIL_CONFIG_BOOST_CHECK_VERSION(102300)
# 		error "Required Boost library version >= 1.23."
# 	endif
# 	include <boost/smart_ptr.hpp>
# 	include <boost/smart_ptr/make_shared.hpp>
# 	define DCS_MEMORY_NS_ ::boost
#endif // DCS_MACRO_CXX11


namespace dcs {

/// Object ownership shared among multiple pointers (\sa \c boost::shared_ptr).
using DCS_MEMORY_NS_::shared_ptr;
/// Non-owning observers of an object owned by \c shared_ptr.
using DCS_MEMORY_NS_::weak_ptr;
///
using DCS_MEMORY_NS_::static_pointer_cast;
///
using DCS_MEMORY_NS_::dynamic_pointer_cast;
///
using DCS_MEMORY_NS_::const_pointer_cast;
///
using DCS_MEMORY_NS_::get_deleter;
///
using DCS_MEMORY_NS_::swap;
///
//using DCS_MEMORY_NS_::owner_less;
///
using DCS_MEMORY_NS_::enable_shared_from_this;
///
using DCS_MEMORY_NS_::make_shared;
///
using DCS_MEMORY_NS_::allocate_shared;

/// Shared ownership of objects with an embedded reference count.
using ::boost::intrusive_ptr;
/// Array ownership shared among multiple pointers.
using ::boost::shared_array;
/// Simple sole ownership of arrays. Noncopyable.
using ::boost::scoped_array;
/// Simple sole ownership of single objects. Noncopyable.
using ::boost::scoped_ptr;
///
using ::boost::get_pointer;

#ifdef DCS_MACRO_CXX11

// Conversion functions between boost::shared_ptr and std::shared_ptr (and vice versa)

template<typename T>
boost::shared_ptr<T> make_shared_ptr(std::shared_ptr<T>& ptr)
{
    return boost::shared_ptr<T>(ptr.get(), [ptr](T*) mutable {ptr.reset();});
}

template<typename T>
std::shared_ptr<T> make_shared_ptr(boost::shared_ptr<T>& ptr)
{
    return std::shared_ptr<T>(ptr.get(), [ptr](T*) mutable {ptr.reset();});
}

#endif // DCS_MACRO_CXX11

} // Namespace dcs


#endif // DCS_SMART_PTR_HPP
