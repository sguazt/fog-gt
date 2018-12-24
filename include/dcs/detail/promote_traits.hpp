//
//  Copyright (c) 2000-2002
//  Joerg Walter, Mathias Koch
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  The authors gratefully acknowledge the support of
//  GeNeSys mbH & Co. KG in producing this work.
//

#ifndef DCS_DETAIL_PROMOTE_TRAITS_HPP
#define DCS_DETAIL_PROMOTE_TRAITS_HPP


#include <boost/mpl/at.hpp>
#include <cstddef>
#include <dcs/detail/type_deduction.hpp>


namespace dcs { namespace detail {

// Use Joel de Guzman's type deduction
template<typename X, typename Y>
class promote_traits
{
	private: typedef type_deduction_detail::base_result_of<X, Y> base_type;
	private: static typename base_type::x_type x;
	private: static typename base_type::y_type y;
	private: static const std::size_t size = sizeof(
			type_deduction_detail::test<
				typename base_type::x_type
			  , typename base_type::y_type
			>(x + y)     // Use x+y to stand of all the arithmetic actions
		);
	private: static const std::size_t index = (size / sizeof (char)) - 1;
	private: typedef typename ::boost::mpl::at_c<typename base_type::types, index>::type id;
	public: typedef typename id::type promote_type;
};

}} // Namespace dcs::detail


#endif // DCS_DETAIL_PROMOTE_TRAITS_HPP
