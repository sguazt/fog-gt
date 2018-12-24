/**
 * \file dcs/algorithm/detail/combperm.hpp
 *
 * \brief Combinatorics-related algorithms.
 *
 * Generate combinations, permutations and partitions.
 *
 * A permutation of size r of a range of size n is a (not necessarily) sorted
 * subsequence of size r of the total range, i.e., a subsequence of elements at
 * r positions among the n positions in the range.
 *
 * A combination of size r of a range of size n is a sorted subsequence of size
 * r of the total range, i.e., the ordered (possibly multi-)set of the elements
 * at r positions among the n positions in the range.
 *
 * A permutation or combination is without repetition if the r indices in the
 * respective definition are distinct (and necessarily r ≤ n), and with
 * repetition otherwise.
 *
 *  Also see:
 *  -# <a href="http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2639.pdf">N2639</a>
 *  .
 *
 * \author Howard Hinnant (howard.hinnant@gmail.com)
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 *
 * <hr/>
 *
 *  (C) Copyright Howard Hinnant 2005-2011.
 *      [http://home.roadrunner.com/~hinnant/combinations.html]
 *  Use, modification and distribution are subject to the Boost Software License,
 *  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
 *  http://www.boost.org/LICENSE_1_0.txt).
 *
 *  See http://www.boost.org/libs/type_traits for most recent version including
 *  documentation.
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

#ifndef DCS_ALGORITHM_DETAIL_COMBPERM_HPP
#define DCS_ALGORITHM_DETAIL_COMBPERM_HPP


#include <algorithm>
#include <dcs/detail/macro_cx11.hpp>
#include <stdexcept>
#include <iterator>
#include <limits>


namespace dcs { namespace algorithm { namespace detail {

/**
 * \brief Rotates two discontinuous ranges to put *first2 where *first1 is.
 *
 * If last1 == first2 this would be equivalent to rotate(first1, first2, last2),
 * but instead the rotate "jumps" over the discontinuity [last1, first2) -
 * which need not be a valid range.
 * In order to make it faster, the length of [first1, last1) is passed in as d1,
 * and d2 must be the length of [first2, last2).
 * a perfect world the d1 > d2 case would have used swap_ranges and
 * reverse_iterator, but reverse_iterator is too inefficient.
 */
template <typename BidirIter>
void
rotate_discontinuous(BidirIter first1, BidirIter last1,
                     typename ::std::iterator_traits<BidirIter>::difference_type d1,
                     BidirIter first2, BidirIter last2,
                     typename ::std::iterator_traits<BidirIter>::difference_type d2)
{
    if (d1 <= d2)
	{
        ::std::rotate(first2, ::std::swap_ranges(first1, last1, first2), last2);
	}
    else
    {
        BidirIter i1 = last1;
        while (first2 != last2)
		{
            ::std::swap(*--i1, *--last2);
		}
        ::std::rotate(first1, i1, last1);
    }
}

/**
 * \brief Rotates the three discontinuous ranges to put *first2 where
 *  *first1 is.
 *
 * Just like rotate_discontinuous, except the second range is now represented by
 * two discontinuous ranges: [first2, last2) + [first3, last3).
 */
template <typename BidirIter>
void
rotate_discontinuous3(BidirIter first1, BidirIter last1,
                      typename ::std::iterator_traits<BidirIter>::difference_type d1,
                      BidirIter first2, BidirIter last2,
                      typename ::std::iterator_traits<BidirIter>::difference_type d2,
                      BidirIter first3, BidirIter last3,
                      typename ::std::iterator_traits<BidirIter>::difference_type d3)
{
    rotate_discontinuous(first1, last1, d1, first2, last2, d2);
    if (d1 <= d2)
	{
        rotate_discontinuous(DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(first2, d2 - d1), last2, d1, first3, last3, d3);
	}
    else
    {
        rotate_discontinuous(DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(first1, d2), last1, d1 - d2, first3, last3, d3);
        rotate_discontinuous(first2, last2, d2, first3, last3, d3);
    }
}

/**
 * \brief Call f() for each combination of the elements
 *  [first1, last1) + [first2, last2) swapped/rotated into the range
 *  [first1, last1).
 *
 *  As long as f() returns false, continue for every combination and then
 *  return [first1, last1) and [first2, last2) to their original state.
 *  If f() returns true, return immediately.
 *  Does the absolute mininum amount of swapping to accomplish its task.
 *  If f() always returns false it will be called (d1+d2)!/(d1!*d2!) times.
 */
template <typename BidirIter, typename Function>
bool
combine_discontinuous(BidirIter first1, BidirIter last1,
                      typename ::std::iterator_traits<BidirIter>::difference_type d1,
                      BidirIter first2, BidirIter last2,
                      typename ::std::iterator_traits<BidirIter>::difference_type d2,
                      Function& f,
                      typename ::std::iterator_traits<BidirIter>::difference_type d = 0)
{
    typedef typename ::std::iterator_traits<BidirIter>::difference_type D;

    if (d1 == 0 || d2 == 0)
	{
        return f();
	}

    if (d1 == 1)
    {
        for (BidirIter i2 = first2; i2 != last2; ++i2)
        {
            if (f())
			{
                return true;
			}
            ::std::swap(*first1, *i2);
        }
    }
    else
    {
        BidirIter f1p = DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(first1);
        BidirIter i2 = first2;
        for (D d22 = d2; i2 != last2; ++i2, --d22)
        {
            if (combine_discontinuous(f1p, last1, d1-1, i2, last2, d22, f, d+1))
			{
                return true;
			}
            ::std::swap(*first1, *i2);
        }
    }

    if (f())
	{
        return true;
	}
    if (d != 0)
	{
        rotate_discontinuous(first1, last1, d1, DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(first2), last2, d2-1);
	}
    else
	{
        rotate_discontinuous(first1, last1, d1, first2, last2, d2);
	}

    return false;
}

/**
 * \brief A binder for binding arguments to call combine_discontinuous.
 */
template <typename Function, typename BidirIter>
class call_combine_discontinuous
{
    private: typedef typename ::std::iterator_traits<BidirIter>::difference_type D;

	public: call_combine_discontinuous(
					  BidirIter first1, BidirIter last1,
					  D d1,
					  BidirIter first2, BidirIter last2,
					  D d2,
					  Function& f)
	: f_(f),
	  first1_(first1),
	  last1_(last1),
	  d1_(d1),
	  first2_(first2),
	  last2_(last2),
	  d2_(d2)
	{
	}

	public: bool operator()()
	{
		return combine_discontinuous(first1_, last1_, d1_, first2_, last2_, d2_, f_);
	}

    private: Function f_;
    private: BidirIter first1_;
    private: BidirIter last1_;
    private: D d1_;
    private: BidirIter first2_;
    private: BidirIter last2_;
    private: D d2_;
};

/**
 * \brief See combine_discontinuous3.
 */
template <typename BidirIter, typename Function>
bool combine_discontinuous3_(BidirIter first1, BidirIter last1,
                        typename ::std::iterator_traits<BidirIter>::difference_type d1,
                        BidirIter first2, BidirIter last2,
                        typename ::std::iterator_traits<BidirIter>::difference_type d2,
                        BidirIter first3, BidirIter last3,
                        typename ::std::iterator_traits<BidirIter>::difference_type d3,
                        Function& f,
                        typename ::std::iterator_traits<BidirIter>::difference_type d = 0)
{
    typedef typename ::std::iterator_traits<BidirIter>::difference_type D;
    if (d1 == 1)
    {
        for (BidirIter i2 = first2; i2 != last2; ++i2)
        {
            if (f())
			{
                return true;
			}
            ::std::swap(*first1, *i2);
        }
        if (f())
		{
            return true;
		}
        ::std::swap(*first1, *DCS_ALGORITHM_DETAIL_MACRO_STD_PREV_(last2));
        ::std::swap(*first1, *first3);
        for (BidirIter i2 = DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(first3); i2 != last3; ++i2)
        {
            if (f())
			{
                return true;
			}
            ::std::swap(*first1, *i2);
        }
    }
    else
    {
        BidirIter f1p = DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(first1);
        BidirIter i2 = first2;
        for (D d22 = d2; i2 != last2; ++i2, --d22)
        {
            if (combine_discontinuous3_(f1p, last1, d1-1, i2, last2, d22, first3, last3, d3, f, d+1))
			{
                return true;
			}
            ::std::swap(*first1, *i2);
        }
        i2 = first3;
        for (D d22 = d3; i2 != last3; ++i2, --d22)
        {
            if (combine_discontinuous(f1p, last1, d1-1, i2, last3, d22, f, d+1))
			{
                return true;
			}
            ::std::swap(*first1, *i2);
        }
    }
    if (f())
	{
        return true;
	}
    if (d1 == 1)
	{
        ::std::swap(*DCS_ALGORITHM_DETAIL_MACRO_STD_PREV_(last2), *first3);
	}
    if (d != 0)
    {
        if (d2 > 1)
		{
            rotate_discontinuous3(first1, last1, d1, DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(first2), last2, d2-1, first3, last3, d3);
		}
        else
		{
            rotate_discontinuous(first1, last1, d1, first3, last3, d3);
		}
    }
    else
	{
        rotate_discontinuous3(first1, last1, d1, first2, last2, d2, first3, last3, d3);
	}
    return false;
}

/**
 * \brief Like combine_discontinuous, but swaps/rotates each combination out of
 *  [first1, last1) + [first2, last2) + [first3, last3) into [first1, last1).
 *
 * If f() always returns false, it is called (d1+d2+d3)!/(d1!*(d2+d3)!) times.
 */
template <typename BidirIter, typename Function>
bool combine_discontinuous3(BidirIter first1,
							BidirIter last1,
						    typename ::std::iterator_traits<BidirIter>::difference_type d1,
						    BidirIter first2, BidirIter last2,
						    typename ::std::iterator_traits<BidirIter>::difference_type d2,
						    BidirIter first3, BidirIter last3,
						    typename ::std::iterator_traits<BidirIter>::difference_type d3,
						    Function& f)
{
    typedef call_combine_discontinuous<Function&, BidirIter> F;
    F fbc(first2, last2, d2, first3, last3, d3, f);  // BC
    return combine_discontinuous3_(first1, last1, d1, first2, last2, d2, first3, last3, d3, fbc);
}

/**
 * \brief See permute.
 */
template <typename BidirIter, typename Function>
bool permute_(BidirIter first1,
			  BidirIter last1,
			  typename ::std::iterator_traits<BidirIter>::difference_type d1,
			  Function& f)
{
    switch (d1)
    {
    case 0:
    case 1:
        return f();
    case 2:
        if (f())
		{
            return true;
		}
        ::std::swap(*first1, *DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(first1));
        return f();
    case 3:
        {
        if (f())
		{
            return true;
		}
        BidirIter f2 = DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(first1);
        BidirIter f3 = DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(f2);
        ::std::swap(*f2, *f3);
        if (f())
		{
            return true;
		}
        ::std::swap(*first1, *f3);
        ::std::swap(*f2, *f3);
        if (f())
		{
            return true;
		}
        ::std::swap(*f2, *f3);
        if (f())
		{
            return true;
		}
        ::std::swap(*first1, *f2);
        ::std::swap(*f2, *f3);
        if (f())
		{
            return true;
		}
        ::std::swap(*f2, *f3);
        return f();
        }
    }
    BidirIter fp1 = DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(first1);
    for (BidirIter p = fp1; p != last1; ++p)
    {
        if (permute_(fp1, last1, d1-1, f))
		{
            return true;
		}
        ::std::reverse(fp1, last1);
        ::std::swap(*first1, *p);
    }
    return permute_(fp1, last1, d1-1, f);
}

/**
 * \brief Calls f() for each permutation of [first1, last1).
 *
 * Divided into permute and permute_ in a (perhaps futile) attempt to
 * squeeze a little more performance out of it.
 */
template <typename BidirIter, typename Function>
bool permute(BidirIter first1,
			 BidirIter last1,
			 typename ::std::iterator_traits<BidirIter>::difference_type d1,
			 Function& f)
{
    switch (d1)
    {
    case 0:
    case 1:
        return f();
    case 2:
        {
			if (f())
			{
				return true;
			}
			BidirIter i = DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(first1);
			::std::swap(*first1, *i);
			if (f())
			{
				return true;
			}
			::std::swap(*first1, *i);
        }
        break;
    case 3:
        {
			if (f())
			{
				return true;
			}
			BidirIter f2 = DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(first1);
			BidirIter f3 = DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(f2);
			::std::swap(*f2, *f3);
			if (f())
			{
				return true;
			}
			::std::swap(*first1, *f3);
			::std::swap(*f2, *f3);
			if (f())
			{
				return true;
			}
			::std::swap(*f2, *f3);
			if (f())
			{
				return true;
			}
			::std::swap(*first1, *f2);
			::std::swap(*f2, *f3);
			if (f())
			{
				return true;
			}
			::std::swap(*f2, *f3);
			if (f())
			{
				return true;
			}
			::std::swap(*first1, *f3);
        }
        break;
    default:
        BidirIter fp1 = DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(first1);
        for (BidirIter p = fp1; p != last1; ++p)
        {
            if (permute_(fp1, last1, d1-1, f))
			{
                return true;
			}
            ::std::reverse(fp1, last1);
            ::std::swap(*first1, *p);
        }
        if (permute_(fp1, last1, d1-1, f))
		{
            return true;
		}
        ::std::reverse(first1, last1);
        break;
    }
    return false;
}

/**
 * \brief Creates a functor with no arguments which calls f_(first_, last_).
 *
 * Also has a variant that takes two It and ignores them.
 */
template <typename Function, typename It>
class bound_range
{
	public: bound_range(Function f, It first, It last)
	: f_(f),
	  first_(first),
	  last_(last)
	{
	}

	public: bool operator()()
	{
		return f_(first_, last_);
	}

	public: bool operator()(It, It)
	{
		return f_(first_, last_);
	}

    private: Function f_;
    private: It first_;
    private: It last_;
};

/**
 * \brief A binder for binding arguments to call permute.
 */
template <typename Function, typename It>
class call_permute
{
	private: typedef typename std::iterator_traits<It>::difference_type D;

	public: call_permute(Function f, It first, It last, D d)
	: f_(f),
	  first_(first),
	  last_(last),
	  d_(d)
	{
	}

	public: bool operator()()
	{
		return permute(first_, last_, d_, f_);
	}

	private: Function f_;
	private: It first_;
	private: It last_;
	private: D d_;
};


/**
 * \brief Adapt functor to permute over [first+1, last).
 *
 * A circular permutation of N items is done by holding the first item and
 * permuting [first+1, last).
 */
template <typename Function, typename BidirIter>
class circular_permutation
{
	private: typedef typename ::std::iterator_traits<BidirIter>::difference_type D;

	public: explicit circular_permutation(Function f, D s)
	: f_(f),
	  s_(s)
	{
	}

	public: bool operator()(BidirIter first, BidirIter last)
	{
		if (s_ <= 1)
		{
			return f_(first, last);
		}
		bound_range<Function, BidirIter> f(f_, first, last);
		return permute(DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(first), last, s_ - 1, f);
	}

	private: Function f_;
	private: D s_;
};

template <typename Function, typename Size>
class reversible_permutation
{
	public: reversible_permutation(Function f, Size s)
	: f_(f),
	  s_(s)
	{
	}

    public: template <typename BidirIter>
    		bool operator()(BidirIter first, BidirIter last);

	private: Function f_;
	private: Size s_;
};

/**
 * \note rev1 looks like call_permute
 */
template <typename Function, typename BidirIter>
class rev1
{
	private: typedef typename ::std::iterator_traits<BidirIter>::difference_type D;

    public: rev1(Function f,
				 BidirIter first,
				 BidirIter last,
				 D d)
	: f_(f),
	  first1_(first),
	  last1_(last),
	  d1_(d)
	{
	}

    public: bool operator()()
    {
        return permute(first1_, last1_, d1_, f_);
    }

	private: Function f_;
	private: BidirIter first1_;
	private: BidirIter last1_;
	private: D d1_;
};

/**
 * \brief For each permutation in [first1, last1), call f() for each
 *  permutation of [first2, last2).
 */
template <typename Function, typename BidirIter>
class rev2
{
	private: typedef typename std::iterator_traits<BidirIter>::difference_type D;

    public: rev2(Function f,
				 BidirIter first1,
				 BidirIter last1,
				 D d1,
				 BidirIter first2,
				 BidirIter last2,
				 D d2)
	: f_(f),
	  first1_(first1),
	  last1_(last1),
	  d1_(d1),
	  first2_(first2),
	  last2_(last2),
	  d2_(d2)
	{
	}

    public: bool operator()()
    {
        call_permute<Function, BidirIter> f(f_, first2_, last2_, d2_);
        return permute(first1_, last1_, d1_, f);
    }

	private: Function f_;
	private: BidirIter first1_;
	private: BidirIter last1_;
	private: D d1_;
	private: BidirIter first2_;
	private: BidirIter last2_;
	private: D d2_;
};

/**
 * \brief For each permutation in [first1, last1), and for each permutation of
 *  [first2, last2) call f() for each permutation of [first3, last3).
 */
template <typename Function, typename BidirIter>
class rev3
{
	private: typedef typename std::iterator_traits<BidirIter>::difference_type D;

	public: rev3(Function f,
				 BidirIter first1,
				 BidirIter last1,
				 D d1,
				 BidirIter first2,
				 BidirIter last2,
				 D d2,
				 BidirIter first3,
				 BidirIter last3,
				 D d3)
	: f_(f),
	  first1_(first1),
	  last1_(last1),
	  d1_(d1),
	  first2_(first2),
	  last2_(last2),
	  d2_(d2),
	  first3_(first3),
	  last3_(last3),
	  d3_(d3)
	{
	}

    public: bool operator()()
    {
        rev2<Function, BidirIter> f(f_, first2_, last2_, d2_, first3_, last3_, d3_);
        return permute(first1_, last1_, d1_, f);
    }

	private: Function f_;
	private: BidirIter first1_;
	private: BidirIter last1_;
	private: D d1_;
	private: BidirIter first2_;
	private: BidirIter last2_;
	private: D d2_;
	private: BidirIter first3_;
	private: BidirIter last3_;
	private: D d3_;
};

template <typename Function, typename Size>
template <typename BidirIter>
//template <typename Function, typename Size, typename BidirIter>
bool reversible_permutation<Function, Size>::operator()(BidirIter first,
														BidirIter last)
{
    //typedef typename ::std::iterator_traits<BidirIter>::difference_type difference_type;
    typedef rev2<bound_range<Function&, BidirIter>, BidirIter> F2;
    typedef rev3<bound_range<Function&, BidirIter>, BidirIter> F3;
    // When the range is 0 - 2, then this is just a combination of N out of N
    //   elements.
    if (s_ < 3)
	{
        return f_(first, last);
	}
    // Hold the first element steady and call f_(first, last) for each
    //    permutation in [first+1, last).
    BidirIter a = DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(first);
    bound_range<Function&, BidirIter> f(f_, first, last);
    if (permute(a, last, s_-1, f))
	{
        return true;
	}
    // Beginning with the first element, swap the previous element with the
    //    next element.  For each swap, call f_(first, last) for each
    //    permutation of the discontinuous range:
    //    [prior to the orignal element] + [after the original element].
    Size s2 = s_ / 2;
    BidirIter am1 = first;
    BidirIter ap1 = DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(a);
    for (Size i = 1; i < s2; ++i, ++am1, ++a, ++ap1)
    {
        ::std::swap(*am1, *a);
        F2 f2(f, first, a, i, ap1, last, s_ - i - 1);
        if (combine_discontinuous(first, a, i, ap1, last, s_ - i - 1, f2))
		{
            return true;
		}
    }
    // If [first, last) has an even number of elements, then fix it up to the
    //     original permutation.
    if (2*s2 == s_)
    {
        ::std::rotate(first, am1, a);
    }
    // else if the range has length 3, we need one more call and the fix is easy.
    else if (s_ == 3)
    {
        ::std::swap(*am1, *a);
        if (f_(first, last))
		{
            return true;
		}
        ::std::swap(*am1, *a);
    }
    // else the range is an odd number greater than 3.  We need to permute
    //     through exactly half of the permuations with the original element in
    //     the middle.
    else
    {
        // swap the original first element into the middle, and hold the current
        //   first element steady.  This creates a discontinuous range:
        //     [first+1, middle) + [middle+1, last).  Run through all permutations
        //     of that discontinuous range.
        ::std::swap(*am1, *a);
        BidirIter b = first;
        BidirIter bp1 = DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(b);
        F2 f2(f, bp1, a, s2-1, ap1, last, s_ - s2 - 1);
        if (combine_discontinuous(bp1, a, s2-1, ap1, last, s_ - s2 - 1, f2))
		{
            return true;
		}
        // Swap the current first element into every place from first+1 to middle-1.
        //   For each location, hold it steady to create the following discontinuous
        //   range (made of 3 ranges): [first, b-1) + [b+1, middle) + [middle+1, last).
        //   For each b in [first+1, middle-1), run through all permutations of
        //      the discontinuous ranges.
        b = bp1;
        ++bp1;
        BidirIter bm1 = first;
        for (Size i = 1; i < s2-1; ++i, ++bm1, ++b, ++bp1)
        {
            ::std::swap(*bm1, *b);
            F3 f3(f, first, b, i, bp1, a, s2-i-1, ap1, last, s_ - s2 - 1);
            if (combine_discontinuous3(first, b, i, bp1, a, s2-i-1, ap1, last, s_-s2-1, f3))
			{
                return true;
			}
        }
        // swap b into into middle-1, creates a discontinuous range:
        //     [first, middle-1) + [middle+1, last).  Run through all permutations
        //     of that discontinuous range.
        ::std::swap(*bm1, *b);
        F2 f21(f, first, b, s2-1, ap1, last, s_ - s2 - 1);
        if (combine_discontinuous(first, b, s2-1, ap1, last, s_ - s2 - 1, f21))
		{
            return true;
		}
        // Revert [first, last) to original order
        ::std::reverse(first, b);
        ::std::reverse(first, ap1);
    }

    return false;
}

/**
 * \brief Adapt functor to permute over [first+1, last).
 * A reversible circular permutation of N items is done by holding the first
 * item and reverse-permuting [first+1, last).
 */
template <typename Function, typename BidirIter>
class reverse_circular_permutation
{
    private: typedef typename ::std::iterator_traits<BidirIter>::difference_type D;

	public: explicit reverse_circular_permutation(Function f, D s)
	: f_(f),
	  s_(s)
	{
	}

    public: bool operator()(BidirIter first, BidirIter last)
    {
        if (s_ == 1)
		{
            return f_(first, last);
		}
        typedef typename ::std::iterator_traits<BidirIter>::difference_type D;
        typedef bound_range<Function, BidirIter> BoundFunc;
        BoundFunc f(f_, first, last);
        BidirIter n = DCS_ALGORITHM_DETAIL_MACRO_STD_NEXT_(first);
        return reversible_permutation<BoundFunc, D>(f, ::std::distance(n, last))(n, last);
    }

    private: Function f_;
    private: D s_;
};

template <typename BidirectionalIterator>
bool next_combination(BidirectionalIterator first1,
					  BidirectionalIterator last1,
					  BidirectionalIterator first2,
					  BidirectionalIterator last2)
{
	if ((first1 == last1) || (first2 == last2))
	{
		return false ;
	}

	BidirectionalIterator m1 = last1;
	BidirectionalIterator m2 = last2;

	--m2;
	while (--m1 != first1 && !(*m1 < *m2)) ;

	bool result = (m1 == first1) && !(*first1 < *m2);

	if (!result)
	{
		while (first2 != m2 && !(*m1 < *first2))
		{
			++first2;
		}

		first1 = m1 ;
		::std::iter_swap(first1, first2);
		++first1;
		++first2;
	}
	if ((first1 != last1) && (first2 != last2))
	{
		m1 = last1;
		m2 = first2;
		while ((m1 != first1) && (m2 != last2))
		{
			::std::iter_swap(--m1, m2);
			++m2;
		}
		::std::reverse(first1, m1);
		::std::reverse(first1, last1);
		::std::reverse(m2, last2);
		::std::reverse(first2, last2);
	}
	return !result;
}

template <typename BidirectionalIterator, typename Compare>
bool next_combination(BidirectionalIterator first1,
					  BidirectionalIterator last1,
					  BidirectionalIterator first2,
					  BidirectionalIterator last2,
					  Compare comp)
{
	if ((first1 == last1) || (first2 == last2))
	{
		return false ;
	}

	BidirectionalIterator m1 = last1;
	BidirectionalIterator m2 = last2;

	--m2;
	while (--m1 != first1 && !comp(*m1, *m2)) ;

	bool result = (m1 == first1) && !comp(*first1, *m2);

	if (!result)
	{
		while (first2 != m2 && !comp(*m1, *first2))
		{
			++first2;
		}

		first1 = m1 ;
		::std::iter_swap(first1, first2);
		++first1;
		++first2;
	}
	if ((first1 != last1) && (first2 != last2))
	{
		m1 = last1;
		m2 = first2;
		while ((m1 != first1) && (m2 != last2))
		{
			::std::iter_swap(--m1, m2);
			++m2;
		}
		::std::reverse(first1, m1);
		::std::reverse(first1, last1);
		::std::reverse(m2, last2);
		::std::reverse(first2, last2);
	}
	return !result;
}

template <typename UIntT>
inline
UIntT gcd(UIntT x, UIntT y)
{
    while (y != 0)
    {
        UIntT t = x % y;
        x = y;
        y = t;
    }

    return x;
}

template <typename T>
struct default_incrementor
{
	T& operator()(T& t) const
	{
		return ++t;
	}
};

template <typename T>
struct default_decrementor
{
	T& operator()(T& t) const
	{
		return --t;
	}
};

template <typename BidirectionalIterator, typename T, typename Incrementor>
bool next_mapping(BidirectionalIterator first,
				  BidirectionalIterator last,
				  T first_value,
				  T last_value,
				  Incrementor increment)
{
	if (last == first)
	{
		return false;
	}

	do
	{
		//if (++(*(--last)) != last_value)
		if (increment(*(--last)) != last_value)
		{
			return true ;
		}
		*last = first_value;
	}
	while (last != first);

	return false ;
}

template <typename BidirectionalIterator, typename T, typename Decrementor>
bool prev_mapping(BidirectionalIterator first,
				  BidirectionalIterator last,
				  T first_value,
				  T last_value,
				  Decrementor decrement)
{
	if (last == first)
	{
		return false ;
	}

	--last_value ;

	do
	{
		if (*(--last) != first_value)
		{
			decrement(*last);
			return true;
		}
		*last = last_value;
	}
	while (last != first);

	return true ;
}

}}} // Namespace dcs::algorithm::detail


#endif // DCS_ALGORITHM_DETAIL_COMBPERM_HPP
