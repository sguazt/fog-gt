/* vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */

/**
 * \file dcs/fgt/random.hpp
 *
 * \brief Utility for random number generation.
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

#ifndef DCS_FGT_RANDOM_HPP
#define DCS_FGT_RANDOM_HPP


#include <random>


namespace dcs { namespace fgt {

class random_number_engine_t
{
private:
    //typedef std::defatul_random_engine eng_impl_type;
    typedef std::mt19937 eng_impl_type;


public:
    typedef typename eng_impl_type::result_type result_type;

    constexpr static result_type default_seed = eng_impl_type::default_seed;


public:
    explicit random_number_engine_t(result_type seed = default_seed)
    : eng_(seed)
    {
    }

    virtual ~random_number_engine_t()
    {
        // Empty
    }

    /// Initialize the random engine with the given seed
    void seed(result_type value = default_seed)
    {
        eng_.seed(value);
    }

    /// Generate a new random number
    result_type operator()()
    {
        return eng_();
    }

private:
    eng_impl_type eng_;
}; // random_number_engine_t


template <typename T>
class degenerate_distribution
{
public:
	typedef T result_type;

	struct param_type
	{
		public:
			typedef degenerate_distribution<result_type> distribution_type;
			explicit param_type(result_type v = result_type())
			: v_(v)
			{
			}
			result_type value() const
			{
				return v_;
			}

			friend bool operator==(const param_type& p1, const param_type& p2)
			{
				return p1.v_ == p2.v_;
			}

		private:
			T v_;
	};

public:
	explicit degenerate_distribution(T v = T())
	: p_(v)
	{
	}

	explicit degenerate_distribution(const param_type& p)
	: p_(p)
	{
	}

	void reset() { }

	result_type value() const { return p_.value(); }

	param_type param() const { return p_; }
	void param(const param_type& p) { p_ = p; }

	result_type min() const { return p_.value(); }

	result_type max() const { return p_.value(); }

	template <typename URNGT>
	result_type operator()(URNGT& rng)
	{
		(void) rng;
		return p_.value();
	}

	friend bool operator==(const degenerate_distribution& d1, const degenerate_distribution& d2)
	{
		return d1.p_ == d2.p_;
	}

private:
	param_type p_;
}; // degenerate_distribution

}} // Namespace dcs::fgt


#endif // DCS_FGT_RANDOM_HPP
