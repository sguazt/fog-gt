/**
 * \file dcs/algorithm/subset.hpp
 *
 * \brief Generate the power set of a set in lexicographic order.
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

#ifndef DCS_COMMONS_ALGORITHM_SUBSET_HPP
#define DCS_COMMONS_ALGORITHM_SUBSET_HPP

#include <algorithm>
#include <boost/dynamic_bitset.hpp>
#include <boost/math/special_functions/binomial.hpp>
#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/exception.hpp>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <vector>


namespace dcs { namespace algorithm {

/**
 * \brief Traits class for subset types
 *
 * \tparam ValueT The value type
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename ValueT>
struct subset_traits
{
	typedef ValueT value_type;
	typedef ::std::vector<value_type> subset_container;///< DEPRECATED
	typedef typename subset_container::iterator subset_container_iterator;///< DEPRECATED
	typedef typename subset_container::const_iterator subset_container_const_iterator;///< DEPRECATED
	typedef ::std::vector<value_type> element_container;
	typedef typename element_container::iterator element_iterator;
	typedef typename element_container::const_iterator element_const_iterator;
}; // subset_traits


/**
 * \brief Class to generate in lexicographic order all subsets
 *
 * Given a set N={0,1,...,n-1} of n elements, this class iteratively generates
 * all subset S of N, possibly included the empty set, in lexicographic order,
 * that is, to generate a subset containing the i-th element we generate all
 * subset containing the preceding 0,1,...(i-1)-th elements.
 * For instance, for a set of 4 elements, the subset generation in lexicographic
 * order produces the following sequence:
 * <pre>
 *  \emptyset,
 *  {0},
 *  {1},
 *  {0,1},
 *  {2},
 *  {0,2},
 *  {1,2},
 *  {0,1,2},
 *  {3},
 *  {0,3},
 *  {1,3},
 *  {2,3},
 *  {0,1,3},
 *  {0,2,3}
 *  {1,2,3},
 *  {0,1,2,3}
 * </pre>
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
class lexicographic_subset
{
	public: class const_iterator;


	friend class const_iterator;


	private: typedef lexicographic_subset self_type;
	private: typedef ::boost::dynamic_bitset<> impl_type;
	private: typedef typename impl_type::size_type size_type;
	private: typedef unsigned long word_type;


	public: explicit lexicographic_subset(::std::size_t n, bool empty_set=true)
	: n_(n),
	  empty_set_(empty_set),
	  bits_(n, empty_set ? 0 : 1),
	  has_prev_(false),
	  has_next_(n_ > 0 ? true : false)
	{
		DCS_ASSERT(n_ > 0,
				   DCS_EXCEPTION_THROW(::std::invalid_argument,
									   "Number of elements must be positive"));
	}

	public: ::std::size_t max_size() const
	{
		return n_;
	}

	public: ::std::size_t size() const
	{
		return bits_.count();
	}

	public: ::std::size_t count() const
	{
		const ::std::size_t c = 1 << n_; // 2^n

		return empty_set_ ? c : (c-1);
	}

	public: self_type& operator++()
	{
		DCS_ASSERT(has_next_,
				   DCS_EXCEPTION_THROW(::std::overflow_error,
									   "No following subsets"));

		has_next_ = bits_.count() < n_;

		if (has_next_)
		{
			bits_ = impl_type(n_, bits_.to_ulong()+1);
		}

		has_prev_ = bits_.to_ulong() > (empty_set_ ? 0 : 1);

		return *this;
	}

	public: bool has_next() const
	{
		return has_next_;
	}

	public: self_type& operator--()
	{
		DCS_ASSERT(has_prev_,
				   DCS_EXCEPTION_THROW(::std::underflow_error,
									   "No preceding subsets"));

		has_prev_ = bits_.to_ulong() > (empty_set_ ? 0 : 1);

		if (has_prev_)
		{
			bits_ = impl_type(n_, bits_.to_ulong()-1);
		}

		has_next_ = bits_.to_ulong() < n_;

		return *this;
	}

	public: bool has_prev() const
	{
		return has_prev_;
	}

	public: typename ::std::vector<size_type> operator()() const
	{
		::std::vector<size_type> subset;

		for (size_type pos = bits_.find_first();
			 pos != impl_type::npos;
			 pos = bits_.find_next(pos))
		{
			subset.push_back(pos);
		}

		return subset;
	}

	//public: template <typename ElemT, typename IterT>
	public: template <typename ElemT>
			typename subset_traits<ElemT>::element_container operator()(::std::vector<ElemT> const& v) const
	{
		DCS_ASSERT(v.size() == n_,
				   DCS_EXCEPTION_THROW(::std::invalid_argument,
									   "Size does not match"));

		typename subset_traits<ElemT>::element_container subset;

		for (size_type pos = bits_.find_first();
			 pos != impl_type::npos;
			 pos = bits_.find_next(pos))
		{
			subset.push_back(v[pos]);
		}

		return subset;
	}

	public: template <typename IterT>
			typename subset_traits< typename ::std::iterator_traits<IterT>::value_type >::element_container operator()(IterT first, IterT last) const
	{
			return this->operator()(::std::vector<typename ::std::iterator_traits<IterT>::value_type>(first, last));
	}

	public: const_iterator begin() const
	{
		return const_iterator(this, bits_.any() ? bits_.find_first() : impl_type::npos);
	}

	public: const_iterator end() const
	{
		return const_iterator(this, impl_type::npos);
	}


	public: class const_iterator: public ::std::iterator< ::std::bidirectional_iterator_tag,
														 typename lexicographic_subset::impl_type::size_type const>
	{
		private: typedef ::std::iterator< ::std::bidirectional_iterator_tag,
										  typename lexicographic_subset::impl_type::size_type const> base_type;
		private: typedef lexicographic_subset container_type;
		private: typedef typename container_type::impl_type bitset_type;
		private: typedef typename container_type::size_type size_type;
		public: typedef typename base_type::value_type value_type;
		public: typedef typename base_type::difference_type difference_type;
		public: typedef typename base_type::pointer pointer;
		public: typedef typename base_type::reference reference;
		public: typedef typename base_type::iterator_category iterator_category;


		public: const_iterator(container_type const* p_subset, size_type pos)
		: p_sub_(p_subset),
		  pos_(pos)
		{
		}

		public: reference operator*() const
		{
			return pos_;
		}

		public: pointer operator->() const
		{
			return &(operator*());
		}

		public: const_iterator& operator++()
		{
			pos_ = p_sub_->bits_.find_next(pos_);

			return *this;
		}

		public: const_iterator operator++(int)
		{
			const_iterator tmp = *this;

			operator++();

			return tmp;
		}

		public: const_iterator& operator--()
		{
			size_type pos = bitset_type::npos;
			for (size_type pos2 = p_sub_->bits_.find_first();
				 pos2 != pos_;
				 pos2 = p_sub_->bits_.find_next(pos2))
			{
				pos = pos2;
			}

			pos_ = pos;

			return *this;
		}

		public: const_iterator operator--(int)
		{
			const_iterator tmp = *this;

			operator--();

			return tmp;
		}

		public: const_iterator operator+(difference_type n) const
		{
			const_iterator it = *this;

			if (n > 0)
			{
				while (n--)
				{
					++it;
				}
			}
			else
			{
				while (n++)
				{
					--it;
				}
			}

			return it;
		}

		public: const_iterator operator-(difference_type n) const
		{
			return operator+(-n);
		}

		public: const_iterator& operator+=(difference_type n)
		{
			if (n > 0)
			{
				while (n--)
				{
					operator++();
				}
			}
			else
			{
				while (n++)
				{
					operator--();
				}
			}

			return *this;
		}

		public: const_iterator& operator-=(difference_type n)
		{
			return operator+=(-n);
		}

		friend
		bool operator==(const_iterator const& lhs, const_iterator const& rhs)
		{
			return lhs.p_sub_ == rhs.p_sub_ && lhs.pos_ == rhs.pos_;
		}

		friend
		bool operator!=(const_iterator const& lhs, const_iterator const& rhs)
		{
			return !(lhs == rhs);
		}


		private: container_type const* p_sub_;
		private: size_type pos_;
	}; // const_iterator


	private: ::std::size_t n_; ///< The max number of elements
	private: bool empty_set_; ///< Flag to enable or disable the inclusion of the empty set
	private: impl_type bits_; ///< The subset implementation
	private: bool has_prev_;
	private: bool has_next_;
}; // lexicographic_subset


/**
 * \brief Class to generate in lexicographic order all subsets of a specific size
 *
 * Given a set N={0,1,...,n-1} of n elements, this class iteratively generates
 * all subset S of N of size 0<=k<=n, in lexicographic order.
 * For instance, for a set of 4 elements, the generation of subset of size 2 in
 * lexicographic order produces the following sequence:
 * <pre>
 *  \emptyset,
 *  {0,1},
 *  {0,2},
 *  {1,2},
 *  {0,3},
 *  {1,3},
 *  {2,3}
 * </pre>
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
class lexicographic_k_subset
{
	public: class const_iterator;


	friend class const_iterator;


	private: typedef lexicographic_k_subset self_type;
	private: typedef ::boost::dynamic_bitset<> impl_type;
	private: typedef typename impl_type::size_type size_type;
	private: typedef unsigned long word_type;


	public: explicit lexicographic_k_subset(::std::size_t n, ::std::size_t k)
	: n_(n),
	  k_(k),
	  bits_(n, (1 << k_)-1),
	  has_prev_(false),
	  has_next_(n_ > 0 && k_ > 0 ? true : false)
	{
		DCS_ASSERT(n_ > 0,
				   DCS_EXCEPTION_THROW(::std::invalid_argument,
									   "Number of elements must be positive"));
		DCS_ASSERT(n_ >= k,
				   DCS_EXCEPTION_THROW(::std::invalid_argument,
									   "Size of subset must be non negative"));
	}

	public: ::std::size_t max_size() const
	{
		return k_;
	}

	public: ::std::size_t size() const
	{
		return bits_.count();
	}

	public: ::std::size_t count() const
	{
		return static_cast<std::size_t>(boost::math::binomial_coefficient<double>(n_, k_));
	}

	public: self_type& operator++()
	{
		DCS_ASSERT(has_next_,
				   DCS_EXCEPTION_THROW(::std::overflow_error,
									   "No following subsets"));

		if (k_ > 0)
		{
			unsigned long v = bits_.to_ulong();

			const unsigned long lo = v & ~(v - 1);       // lowest one bit
			const unsigned long lz = (v + lo) & ~v;      // lowest zero bit above lo
			v |= lz;                     // add lz to the set
			v &= ~(lz - 1);              // reset bits below lz
			v |= (lz / lo / 2) - 1;      // put back right number of bits at end

			has_next_ = !(v & 1 << n_);

			if (has_next_)
			{
				bits_ = impl_type(n_, v);
			}

			has_prev_ = (bits_.to_ulong() != static_cast<unsigned long>((1 << k_)-1));
		}

		return *this;
	}

	public: bool has_next() const
	{
		return has_next_;
	}

	public: self_type& operator--()
	{
		DCS_ASSERT(has_prev_,
				   DCS_EXCEPTION_THROW(::std::underflow_error,
									   "No preceding subsets"));

		if (k_ > 0)
		{
			const unsigned long w = bits_.to_ulong();
			const unsigned long min = (1 << k_)-1;
			unsigned long v = min;
			unsigned long p = v;

			while (v != w)
			{
				p = v;

				const unsigned long lo = v & ~(v - 1);       // lowest one bit
				const unsigned long lz = (v + lo) & ~v;      // lowest zero bit above lo
				v |= lz;                     // add lz to the set
				v &= ~(lz - 1);              // reset bits below lz
				v |= (lz / lo / 2) - 1;      // put back right number of bits at end
			}

			has_prev_ = (p != min) || (w != min);

//			if (has_prev_)
//			{
				bits_ = impl_type(n_, p);
//			}

			has_next_ = !(p & 1 << n_);
		}

		return *this;
	}

	public: bool has_prev() const
	{
		return has_prev_;
	}

	//public: template <typename ElemT, typename IterT>
	public: template <typename ElemT>
			typename subset_traits<ElemT>::element_container operator()(::std::vector<ElemT> const& v) const
	{
		DCS_ASSERT(v.size() == n_,
				   DCS_EXCEPTION_THROW(::std::invalid_argument,
									   "Size does not match"));

		typename subset_traits<ElemT>::element_container subset;

		for (size_type pos = bits_.find_first();
			 pos != impl_type::npos;
			 pos = bits_.find_next(pos))
		{
			subset.push_back(v[pos]);
		}

		return subset;
	}

	public: template <typename IterT>
			typename subset_traits< typename ::std::iterator_traits<IterT>::value_type >::element_container operator()(IterT first, IterT last) const
	{
			return this->operator()(::std::vector<typename ::std::iterator_traits<IterT>::value_type>(first, last));
	}

	public: const_iterator begin() const
	{
		return const_iterator(this, bits_.any() ? bits_.find_first() : impl_type::npos);
	}

	public: const_iterator end() const
	{
		return const_iterator(this, impl_type::npos);
	}


	public: class const_iterator: public ::std::iterator< ::std::bidirectional_iterator_tag,
														 typename lexicographic_k_subset::impl_type::size_type const>
	{
		private: typedef ::std::iterator< ::std::bidirectional_iterator_tag,
										  typename lexicographic_k_subset::impl_type::size_type const> base_type;
		private: typedef lexicographic_k_subset container_type;
		private: typedef typename container_type::impl_type bitset_type;
		private: typedef typename container_type::size_type size_type;
		public: typedef typename base_type::value_type value_type;
		public: typedef typename base_type::difference_type difference_type;
		public: typedef typename base_type::pointer pointer;
		public: typedef typename base_type::reference reference;
		public: typedef typename base_type::iterator_category iterator_category;


		public: const_iterator(container_type const* p_subset, size_type pos)
		: p_sub_(p_subset),
		  pos_(pos)
		{
		}

		public: reference operator*() const
		{
			return pos_;
		}

		public: pointer operator->() const
		{
			return &(operator*());
		}

		public: const_iterator& operator++()
		{
			pos_ = p_sub_->bits_.find_next(pos_);

			return *this;
		}

		public: const_iterator operator++(int)
		{
			const_iterator tmp = *this;

			operator++();

			return tmp;
		}

		public: const_iterator& operator--()
		{
			size_type pos = bitset_type::npos;
			for (size_type pos2 = p_sub_->bits_.find_first();
				 pos2 != pos_;
				 pos2 = p_sub_->bits_.find_next(pos2))
			{
				pos = pos2;
			}

			pos_ = pos;

			return *this;
		}

		public: const_iterator operator--(int)
		{
			const_iterator tmp = *this;

			operator--();

			return tmp;
		}

		public: const_iterator operator+(difference_type n) const
		{
			const_iterator it = *this;

			if (n > 0)
			{
				while (n--)
				{
					++it;
				}
			}
			else
			{
				while (n++)
				{
					--it;
				}
			}

			return it;
		}

		public: const_iterator operator-(difference_type n) const
		{
			return operator+(-n);
		}

		public: const_iterator& operator+=(difference_type n)
		{
			if (n > 0)
			{
				while (n--)
				{
					operator++();
				}
			}
			else
			{
				while (n++)
				{
					operator--();
				}
			}

			return *this;
		}

		public: const_iterator& operator-=(difference_type n)
		{
			return operator+=(-n);
		}

		friend
		bool operator==(const_iterator const& lhs, const_iterator const& rhs)
		{
			return lhs.p_sub_ == rhs.p_sub_ && lhs.pos_ == rhs.pos_;
		}

		friend
		bool operator!=(const_iterator const& lhs, const_iterator const& rhs)
		{
			return !(lhs == rhs);
		}


		private: container_type const* p_sub_;
		private: size_type pos_;
	}; // const_iterator


	private: ::std::size_t n_; ///< The number of elements of the set
	private: ::std::size_t k_; ///< The max number of elements of the subset
	private: impl_type bits_; ///< The subset implementation
	private: bool has_prev_;
	private: bool has_next_;
}; // lexicographic_k_subset


template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os,
													lexicographic_subset const& subset)
{
	os << '(';

	if (subset.size() > 0)
	{
		if (subset.size() > 1)
		{
			::std::copy(subset.begin(),
						subset.end()-1,
						::std::ostream_iterator< ::std::size_t >(os, " "));
		}

		os << *(subset.end()-1);
	}

	os << ')';

	return os;
}

template <typename CharT, typename CharTraitsT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os,
													lexicographic_k_subset const& subset)
{
	os << '(';

	if (subset.size() > 0)
	{
		if (subset.size() > 1)
		{
			::std::copy(subset.begin(),
						subset.end()-1,
						::std::ostream_iterator< ::std::size_t >(os, " "));
		}

		os << *(subset.end()-1);
	}

	os << ')';

	return os;
}

template <typename BidiIterT, typename SubsetT>
inline
typename subset_traits< typename ::std::iterator_traits<BidiIterT>::value_type >::element_container
next_subset(BidiIterT first,
			BidiIterT last,
			SubsetT& subset)
{
	typedef typename ::std::iterator_traits<BidiIterT>::value_type value_type;
	typedef typename subset_traits<value_type>::element_container element_container;

	element_container subs = subset(first, last);

	++subset;

	return subs;
}

template <typename BidiIterT, typename SubsetT>
inline
typename subset_traits< typename ::std::iterator_traits<BidiIterT>::value_type >::element_container
prev_subset(BidiIterT first,
			BidiIterT last,
			SubsetT& subset)
{
	typedef typename ::std::iterator_traits<BidiIterT>::value_type value_type;
	typedef typename subset_traits<value_type>::element_container element_container;

	element_container subs = subset(first, last);

	--subset;

	return subs;
}

/*
inline
::std::size_t count_subsets(n)
{
	const ::std::size_t m = n >> 1;
	const bool flag = ((m << 1) - n) > 0;

	::std::size_t c = 0;
	for (std::size_t i = 0; i < m; ++i)
	{
		c += 
	}
}
*/

}} // Namespace dcs::algorithm

#endif // DCS_COMMONS_ALGORITHM_SUBSET_HPP
