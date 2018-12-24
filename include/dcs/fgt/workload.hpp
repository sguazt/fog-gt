/* vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */

/**
 * \file dcs/fgt/workload.hpp
 *
 * \brief Utility for workload generations.
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

#ifndef DCS_FGT_WORKLOAD_HPP
#define DCS_FGT_WORKLOAD_HPP


#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/macro.hpp>
#include <dcs/exception.hpp>
#include <initializer_list>
#include <stdexcept>
#include <tuple>
#include <vector>


namespace dcs { namespace fgt {


template <typename RealT>
struct workload_generator_t
{
    virtual ~workload_generator_t() { }

    virtual std::tuple<RealT,RealT> operator()(random_number_engine_t& rng) = 0;
}; // workload_generator_t


template <typename RealT>
class multistep_workload_generator_t: public workload_generator_t<RealT>
{
public:
    template <typename DurationIterT, typename ArrRateIterT>
    multistep_workload_generator_t(DurationIterT duration_first, DurationIterT duration_last,
                                   ArrRateIterT arr_rate_first, ArrRateIterT arr_rate_last)
    : durations_(duration_first, duration_last),
      arr_rates_(arr_rate_first, arr_rate_last),
      n_(durations_.size()),
      next_idx_(0)
    {
    }

    multistep_workload_generator_t(std::initializer_list<RealT> durations,
                                   std::initializer_list<RealT> arr_rates)
    : durations_(durations.begin(), durations.end()),
      arr_rates_(arr_rates.begin(), arr_rates.end()),
      n_(durations_.size()),
      next_idx_(0)
    {
        // check: size(durations_) == size(arr_rates_)
        DCS_ASSERT(durations_.size() == arr_rates_.size(),
                   DCS_EXCEPTION_THROW(std::invalid_argument, "Duration vector and arrival rate vector have different size"));
    }

    std::tuple<RealT,RealT> operator()(random_number_engine_t& rng)
    {
        DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( rng );

        RealT duration = durations_[next_idx_];
        RealT arr_rate = arr_rates_[next_idx_];

        next_idx_ = (next_idx_+1) % n_;

        return std::make_tuple(duration, arr_rate);
    }

private:
    std::vector<RealT> durations_;
    std::vector<RealT> arr_rates_;
    std::size_t n_;
    std::size_t next_idx_;
}; // multistep_workload_generator_t


}} // Namespace dcs::fgt

#endif // DCS_FGT_WORKLOAD_HPP
