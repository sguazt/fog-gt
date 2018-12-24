#ifndef DCS_ALGORITHM_SLICE_HPP
#define DCS_ALGORITHM_SLICE_HPP


#include <boost/core/enable_if.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/tti/has_member_function.hpp>
#include <boost/tti/has_type.hpp>
#include <vector>


namespace dcs { namespace algorithm {

namespace detail {

//BOOST_TTI_HAS_TYPE(value_type)
BOOST_TTI_HAS_TYPE(key_type)
BOOST_TTI_HAS_TYPE(mapped_type)
BOOST_TTI_HAS_MEMBER_FUNCTION(at)

template <typename T>
struct is_not_associative_with_at
{
	typedef typename boost::mpl::and_< boost::mpl::not_<typename has_type_key_type<T>::type>,
									   typename has_member_function_at<T,typename T::reference,boost::mpl::vector<typename T::size_type> >::type>::type type;
};

template <typename T>
struct is_associative_not_map
{
	typedef typename boost::mpl::and_< typename has_type_key_type<T>::type,
									   boost::mpl::not_<typename has_type_mapped_type<T>::type> >::type type;
};

template <typename T>
struct is_associative_map
{
	typedef typename has_type_mapped_type<T>::type type;
};

/*
template <typename T>
struct is_associative
{
	typedef typename has_type_key_type<T>::type type;
};

template <typename T>
struct is_associative_not_map
{
	typedef typename boost::mpl::and_< typename has_type_key_type<T>::type,
									   boost::mpl::not_<typename has_type_mapped_type<T>::type> >::type type;
};
*/

} // Namespace detail

template <typename ContainerT, typename IterT>
typename boost::enable_if< typename detail::is_not_associative_with_at<ContainerT>::type,
							std::vector<typename ContainerT::value_type> >::type
slice(ContainerT const& c, IterT first, IterT last)
{
	std::vector<typename ContainerT::value_type> res;

	while (first != last)
	{
		res.push_back(c.at(*first));
		++first;
	}

	return res;
}

template <typename ContainerT, typename IterT>
typename boost::enable_if< typename detail::is_associative_not_map<ContainerT>::type,
							std::vector<typename ContainerT::value_type> >::type
slice(ContainerT const& c, IterT first, IterT last)
{
	std::vector<typename ContainerT::value_type> res;

	while (first != last)
	{
		res.push_back(*(c.find(*first)));
		++first;
	}

	return res;
}

template <typename ContainerT, typename IterT>
typename boost::enable_if< typename detail::is_associative_map<ContainerT>::type,
						   std::vector<typename ContainerT::mapped_type> >::type
slice(ContainerT const& c, IterT first, IterT last)
{
	std::vector<typename ContainerT::mapped_type> res;

	while (first != last)
	{
		res.push_back(c.at(*first));
		++first;
	}

	return res;
}

/*
template <typename ContainerT>
typename boost::enable_if<typename detail::is_map<ContainerT>::value,
						  std::vector<typename ContainerT::value_type> >::type slice(ContainerT const& c)
{
	return std::vector<typename ContainerT::value_type>();
}

template <typename ContainerT>
typename boost::enable_if<typename detail::is_associative_not_map<ContainerT>::value,
						  std::vector<typename ContainerT::value_type> >::type slice(ContainerT const& c)
{
	return std::vector<typename ContainerT::value_type>();
}
*/

}} // Namespace dcs::algorithm

 
#endif // DCS_ALGORITHM_SLICE_HPP
