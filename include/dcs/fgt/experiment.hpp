/* vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */

/**
 * \file dcs/fgt/experiment.hpp
 *
 * \brief Function and types for performing experiments.
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

#ifndef DCS_FGT_EXPERIMENT_HPP
#define DCS_FGT_EXPERIMENT_HPP


#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/smart_ptr.hpp>
#include <cctype>
#include <cstddef>
#include <ctime>
#include <dcs/algorithm/combinatorics.hpp>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/exception.hpp>
#include <dcs/fgt/coalition_formation.hpp>
#include <dcs/fgt/MMc.hpp>
#include <dcs/fgt/random.hpp>
#include <dcs/fgt/simulator.hpp>
#include <dcs/fgt/statistics.hpp>
#include <dcs/fgt/util.hpp>
#include <dcs/fgt/vm_allocation.hpp>
#include <dcs/fgt/vm_allocation_solvers.hpp>
#include <dcs/fgt/workload.hpp>
#include <dcs/logging.hpp>
#include <fstream>
#include <functional>
#include <gtpack/cooperative.hpp>
#include <initializer_list>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>


namespace dcs { namespace fgt {

/// Experimental scenario
template <typename RealT>
struct scenario_t
{
    std::size_t num_fps; ///< Number of fog providers (FPs)
    std::size_t num_fn_categories; ///< Number of fog node (FN) categories
    std::size_t num_svc_categories; ///< Number of service categories
    std::size_t num_vm_categories; ///< Number of virtual machine (VM) categories
    std::vector<RealT> svc_max_delays; ///< Max delays tolerated by services, by service category
    std::vector<std::size_t> svc_vm_categories; ///< Category of the VMs associated with each service, by service category (VMs of the same service belong to the same category).
    std::vector<RealT> svc_vm_service_rates; ///< Service rate of every VM associated with each service, by service category
    std::vector<std::vector<std::pair<RealT,RealT>>> svc_workloads; ///< Workload (i.e., a sequence of duration - arrival rate pairs denoting a multistep function) of each service, by service category
    std::vector<std::vector<std::size_t>> fp_num_svcs; ///< Number of services, by FP and per service category
    std::vector<std::vector<std::size_t>> fp_num_fns; ///< Number of FNs, by FP and per FN category
    //std::vector<std::tuple<RealT,RealT,RealT>> provider_electricity_costs; ///< FP electricity cost plans: <start-hour, stop-hour, $/kWh>
    std::vector<RealT> fp_electricity_costs; ///< FP electricity cost plans (in $/kWh)
    std::vector<RealT> fp_coalition_costs; ///< Cost due to form a coalition structure, by FP
    std::vector<std::vector<RealT>> fp_svc_revenues; ///< FP revenues (in $/service) for running services, by FP and service category
    std::vector<std::vector<RealT>> fp_svc_penalties; ///< FP penalties (in $/service) for violating service QoS (i.e., the max delay), by FP and service category
    std::vector<std::vector<RealT>> fp_fn_asleep_costs; ///< FP costs for powering off a powered-on FN, by FP and FN category
    std::vector<std::vector<RealT>> fp_fn_awake_costs; ///< FP costs for powering on a powered-off FN, by FP and FN category
    //std::vector<std::vector<RealT>> fp_svc_mixes; ///< Service classes mix, by FP and service category
    std::vector<RealT> fn_min_powers; ///< FN min power consumptions (in kW), by FN category
    std::vector<RealT> fn_max_powers; ///< FN max power consumptions (in kW), by FN category
    std::vector<std::vector<RealT>> vm_cpu_requirements; ///< VM CPU requirements, by VM category and FN category
    std::vector<std::vector<RealT>> vm_ram_requirements; ///< VM RAM requirements, by VM category and FN category
}; // scenario_t


/// Experimental options
template <typename RealT>
struct options_t
{
    options_t()
    : coalition_formation(fgt::nash_stable_coalition_formation),
      coalition_formation_interval(0),
      coalition_value_division(fgt::shapley_coalition_value_division),
      find_all_best_partitions(false),
      optim_relative_tolerance(0),
      optim_time_limit(-1),
      sim_ci_level(0.95),
      sim_ci_rel_precision(0.04),
      sim_max_num_replications(0),
      sim_max_replication_duration(0),
      service_delay_tolerance(0),
      verbosity(0)
    {
    }


    fgt::coalition_formation_category coalition_formation; ///< The strategy according which form coalitions
    RealT coalition_formation_interval; ///< The activating time of the coalition formation algorithm
    fgt::coalition_value_division_category coalition_value_division;
    bool find_all_best_partitions; ///< A \c true value means that all possible best partitions are computed
    RealT optim_relative_tolerance; ///< The relative tolerance option to set in the optimizer
    RealT optim_time_limit; ///< The time limit option to set in the optimizer
    std::string output_stats_data_file; ///< The path to the output stats data file
    std::string output_trace_data_file; ///< The path to the output trace data file
    RealT sim_ci_level; ///< Level for confidence intervals
    RealT sim_ci_rel_precision; ///< Relative precision for the half-width of the confidence intervals
    std::size_t sim_max_num_replications; ///< Maximum number of replications (use 0 for an unlimited number of replications)
    RealT sim_max_replication_duration; ///< Maximum length of each replication
    RealT service_delay_tolerance; ///< The relative tolerance to set in the service performance model
    int verbosity; ///< The verbosity level: 0 for 'minimum' and 9 for 'maximum' verbosity level
}; // options_t

template <typename CharT, typename CharTraitsT, typename RealT>
std::basic_ostream<CharT,CharTraitsT>& operator<<(std::basic_ostream<CharT,CharTraitsT>& os, const scenario_t<RealT>& s)
{
    os  << "num_fps=" << s.num_fps
        << ", " << "num_fn_categories=" << s.num_fn_categories
        << ", " << "num_svc_categories=" << s.num_svc_categories
        << ", " << "num_vm_categories=" << s.num_vm_categories;

    os << ", " << "svc.max_delays=[";
    for (std::size_t i = 0; i < s.svc_max_delays.size(); ++i)
    {
        if (i > 0)
        {
            os << ", ";
        }
        os << s.svc_max_delays[i];
    }
    os << "]";
    os << ", " << "svc.vm_categories=[";
    for (std::size_t i = 0; i < s.svc_vm_categories.size(); ++i)
    {
        if (i > 0)
        {
            os << ", ";
        }
        os << s.svc_vm_categories[i];
    }
    os << "]";
    os << ", " << "svc.vm_service_rates=[";
    for (std::size_t i = 0; i < s.svc_vm_service_rates.size(); ++i)
    {
        if (i > 0)
        {
            os << ", ";
        }
        os << s.svc_vm_service_rates[i];
    }
    os << "]";
    os << ", " << "svc.workloads=[";
    for (std::size_t i = 0; i < s.svc_workloads.size(); ++i)
    {
        if (i > 0)
        {
            os << ", ";
        }
        os << "[";
        for (std::size_t j = 0; j < s.svc_workloads[i].size(); ++j)
        {
            if (j > 0)
            {
                os << ", ";
            }
            os << "[" << s.svc_workloads[i][j].first << " " << s.svc_workloads[i][j].second << "]";
        }
        os << "]";
    }
    os << "]";
    os << ", " << "fp.num_svcs=[";
    for (std::size_t i = 0; i < s.fp_num_svcs.size(); ++i)
    {
        if (i > 0)
        {
            os << ", ";
        }
        os << "[";
        for (std::size_t j = 0; j < s.fp_num_svcs[i].size(); ++j)
        {
            if (j > 0)
            {
                os << ", ";
            }
            os << s.fp_num_svcs[i][j];
        }
        os << "]";
    }
    os << "]";
    os << ", " << "fp.num_fns=[";
    for (std::size_t i = 0; i < s.fp_num_fns.size(); ++i)
    {
        if (i > 0)
        {
            os << ", ";
        }
        os << "[";
        for (std::size_t j = 0; j < s.fp_num_fns[i].size(); ++j)
        {
            if (j > 0)
            {
                os << ", ";
            }
            os << s.fp_num_fns[i][j];
        }
        os << "]";
    }
    os << "]";
    os << ", " << "fp.electricity_costs=[";
    for (std::size_t i = 0; i < s.fp_electricity_costs.size(); ++i)
    {
        if (i > 0)
        {
            os << ", ";
        }
        os << s.fp_electricity_costs[i];
    }
    os << "]";
    os << ", " << "fp.fn_asleep_costs=[";
    for (std::size_t i = 0; i < s.fp_fn_asleep_costs.size(); ++i)
    {
        if (i > 0)
        {   
            os << "  ";
        }

        os << "[";
        for (std::size_t j = 0; j < s.fp_fn_asleep_costs[i].size(); ++j)
        {
            if (j > 0)
            {   
                os << ", ";
            }
            os << s.fp_fn_asleep_costs[i][j];
        }
        os << "]";
    }
    os << "]";
    os << ", " << "fp.fn_awake_costs=[";
    for (std::size_t i = 0; i < s.fp_fn_awake_costs.size(); ++i)
    {
        if (i > 0)
        {   
            os << "  ";
        }

        os << "[";
        for (std::size_t j = 0; j < s.fp_fn_awake_costs[i].size(); ++j)
        {
            if (j > 0)
            {   
                os << ", ";
            }
            os << s.fp_fn_awake_costs[i][j];
        }
        os << "]";
    }
    os << "]";
    os << ", " << "fp.coalition_costs=[";
    for (std::size_t i = 0; i < s.fp_coalition_costs.size(); ++i)
    {
        if (i > 0)
        {
            os << ", ";
        }
        os << s.fp_coalition_costs[i];
    }
    os << "]";
    os << ", " << "fp.svc_revenues=[";
    for (std::size_t i = 0; i < s.fp_svc_revenues.size(); ++i)
    {
        if (i > 0)
        {
            os << ", ";
        }
        os << "[";
        for (std::size_t j = 0; j < s.fp_svc_revenues[i].size(); ++j)
        {
            if (j > 0)
            {
                os << ", ";
            }
            os << s.fp_svc_revenues[i][j];
        }
        os << "]";
    }
    os << "]";
    os << ", " << "fp.svc_penalties=[";
    for (std::size_t i = 0; i < s.fp_svc_penalties.size(); ++i)
    {
        if (i > 0)
        {
            os << ", ";
        }
        os << "[";
        for (std::size_t j = 0; j < s.fp_svc_penalties[i].size(); ++j)
        {
            if (j > 0)
            {
                os << ", ";
            }
            os << s.fp_svc_penalties[i][j];
        }
        os << "]";
    }
    os << "]";
    os << ", " << "fn.min_powers=[";
    for (std::size_t i = 0; i < s.fn_min_powers.size(); ++i)
    {
        if (i > 0)
        {
            os << ", ";
        }
        os << s.fn_min_powers[i];
    }
    os << "]";
    os << ", " << "fn.max_powers=[";
    for (std::size_t i = 0; i < s.fn_max_powers.size(); ++i)
    {
        if (i > 0)
        {
            os << ", ";
        }
        os << s.fn_max_powers[i];
    }
    os << "]";
    os << ", " << "vm_cpu_requirements=[";
    for (std::size_t i = 0; i < s.vm_cpu_requirements.size(); ++i)
    {
        if (i > 0)
        {
            os << " ";
        }

        os << "[";
        for (std::size_t j = 0; j < s.vm_cpu_requirements[i].size(); ++j)
        {
            if (j > 0)
            {
                os << ", ";
            }
            os << s.vm_cpu_requirements[i][j];
        }
        os << "]";
    }
    os << "]";
    os << ", " << "vm_ram_requirements=[";
    for (std::size_t i = 0; i < s.vm_ram_requirements.size(); ++i)
    {
        if (i > 0)
        {
            os << " ";
        }

        os << "[";
        for (std::size_t j = 0; j < s.vm_ram_requirements[i].size(); ++j)
        {
            if (j > 0)
            {
                os << ", ";
            }
            os << s.vm_ram_requirements[i][j];
        }
        os << "]";
    }
    os << "]";

    return os;
}

template <typename RealT>
scenario_t<RealT> make_scenario(const std::string& fname)
{
    DCS_ASSERT(!fname.empty(),
               DCS_EXCEPTION_THROW(std::invalid_argument, "Invalid scenario file name"));

    scenario_t<RealT> s;

    std::ifstream ifs(fname.c_str());

    DCS_ASSERT(ifs,
               DCS_EXCEPTION_THROW(std::runtime_error, "Cannot open scenario file"));

    std::ostringstream oss;
    std::size_t lineno = 0;
    for (std::string line; std::getline(ifs, line); )
    {
        ++lineno;

        std::size_t pos = 0;
        for (; pos < line.length() && std::isspace(line[pos]); ++pos)
        {
            ; // empty
        }
        if (pos > 0)
        {
            line = line.substr(pos);
        }
        if (line.empty() || line.at(0) == '#')
        {
            // Skip either empty or comment lines
            continue;
        }

        boost::to_lower(line);
        if (boost::istarts_with(line, "num_fps"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            iss >> s.num_fps;
        }
        else if (boost::istarts_with(line, "num_fn_categories"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            iss >> s.num_fn_categories;
        }
        else if (boost::istarts_with(line, "num_svc_categories"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            iss >> s.num_svc_categories;
        }
        else if (boost::istarts_with(line, "num_vm_categories"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            iss >> s.num_vm_categories;
        }
        else if (boost::istarts_with(line, "svc.max_delays"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            // Move to '['
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            s.svc_max_delays.resize(s.num_svc_categories);
            for (std::size_t i = 0; i < s.num_svc_categories; ++i)
            {
                iss >> s.svc_max_delays[i];
            }
        }
        else if (boost::istarts_with(line, "svc.vm_categories"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            // Move to '['
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            s.svc_vm_categories.resize(s.num_svc_categories);
            for (std::size_t i = 0; i < s.num_svc_categories; ++i)
            {
                iss >> s.svc_vm_categories[i];
            }
        }
        else if (boost::istarts_with(line, "svc.vm_service_rates"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            // Move to '['
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            s.svc_vm_service_rates.resize(s.num_svc_categories);
            for (std::size_t i = 0; i < s.num_svc_categories; ++i)
            {
                iss >> s.svc_vm_service_rates[i];
            }
        }
        else if (boost::istarts_with(line, "svc.workloads"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            // Move to '['
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            s.svc_workloads.resize(s.num_svc_categories);
            for (std::size_t i = 0; i < s.num_svc_categories; ++i)
            {
                // Move to '['
                iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
                DCS_ASSERT(iss.good(),
                           DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

                // Skip spaces
                while (std::isspace(iss.peek()))
                {
                    iss.ignore();
                }

                while (iss.peek() != ']')
                {
                    // Move to '['
                    iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
                    DCS_ASSERT(iss.good(),
                               DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

                    RealT duration = 0;
                    RealT arr_rate = 0;

                    iss >> duration;
                    iss >> arr_rate;

                    s.svc_workloads[i].push_back(std::make_pair(duration, arr_rate));

                    // Move to ']'
                    iss.ignore(std::numeric_limits<std::streamsize>::max(), ']');
                    DCS_ASSERT(iss.good(),
                               DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file (']' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

                    // Skip spaces
                    while (std::isspace(iss.peek()))
                    {
                        iss.ignore();
                    }
                }
            }
        }
        else if (boost::istarts_with(line, "fp.num_svcs"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            // Move to '['
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            s.fp_num_svcs.resize(s.num_fps);
            for (std::size_t i = 0; i < s.num_fps; ++i)
            {
                // Move to '['
                iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
                DCS_ASSERT(iss.good(),
                           DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

                s.fp_num_svcs[i].resize(s.num_svc_categories);
                for (std::size_t j = 0; j < s.num_svc_categories; ++j)
                {
                    iss >> s.fp_num_svcs[i][j];
                }

                // Move to ']'
                iss.ignore(std::numeric_limits<std::streamsize>::max(), ']');
                DCS_ASSERT(iss.good(),
                           DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file (']' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));
            }
        }
        else if (boost::istarts_with(line, "fp.num_fns"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            // Move to '['
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            s.fp_num_fns.resize(s.num_fps);
            for (std::size_t i = 0; i < s.num_fps; ++i)
            {
                // Move to '['
                iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
                DCS_ASSERT(iss.good(),
                           DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

                s.fp_num_fns[i].resize(s.num_fn_categories);
                for (std::size_t j = 0; j < s.num_fn_categories; ++j)
                {
                    iss >> s.fp_num_fns[i][j];
                }

                // Move to ']'
                iss.ignore(std::numeric_limits<std::streamsize>::max(), ']');
                DCS_ASSERT(iss.good(),
                           DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file (']' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));
            }
        }
        else if (boost::istarts_with(line, "fp.electricity_costs"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            // Move to '['
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            s.fp_electricity_costs.resize(s.num_fps);
            for (std::size_t i = 0; i < s.num_fps; ++i)
            {
                iss >> s.fp_electricity_costs[i];
            }
        }
        else if (boost::istarts_with(line, "fp.fn_asleep_costs"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            // Move to '['
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            s.fp_fn_asleep_costs.resize(s.num_fps);
            for (std::size_t i = 0; i < s.num_fps; ++i)
            {
                // Move to '['
                iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
                DCS_ASSERT(iss.good(),
                           DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

                s.fp_fn_asleep_costs[i].resize(s.num_fn_categories);
                for (std::size_t j = 0; j < s.num_fn_categories; ++j)
                {
                    iss >> s.fp_fn_asleep_costs[i][j];
                }
            }
        }
        else if (boost::istarts_with(line, "fp.fn_awake_costs"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            // Move to '['
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            s.fp_fn_awake_costs.resize(s.num_fps);
            for (std::size_t i = 0; i < s.num_fps; ++i)
            {
                // Move to '['
                iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
                DCS_ASSERT(iss.good(),
                           DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

                s.fp_fn_awake_costs[i].resize(s.num_fn_categories);
                for (std::size_t j = 0; j < s.num_fn_categories; ++j)
                {
                    iss >> s.fp_fn_awake_costs[i][j];
                }
            }
        }
        else if (boost::istarts_with(line, "fp.coalition_costs"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            // Move to '['
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            s.fp_coalition_costs.resize(s.num_fps);
            for (std::size_t i = 0; i < s.num_fps; ++i)
            {
                iss >> s.fp_coalition_costs[i];
            }
        }
        else if (boost::istarts_with(line, "fp.svc_revenues"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            // Move to '['
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            s.fp_svc_revenues.resize(s.num_fps);
            for (std::size_t i = 0; i < s.num_fps; ++i)
            {
                // Move to '['
                iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
                DCS_ASSERT(iss.good(),
                           DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

                s.fp_svc_revenues[i].resize(s.num_svc_categories);
                for (std::size_t j = 0; j < s.num_svc_categories; ++j)
                {
                    iss >> s.fp_svc_revenues[i][j];
                }

                // Move to ']'
                iss.ignore(std::numeric_limits<std::streamsize>::max(), ']');
                DCS_ASSERT(iss.good(),
                           DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file (']' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));
            }
        }
        else if (boost::istarts_with(line, "fp.svc_penalties"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            // Move to '['
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            s.fp_svc_penalties.resize(s.num_fps);
            for (std::size_t i = 0; i < s.num_fps; ++i)
            {
                // Move to '['
                iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
                DCS_ASSERT(iss.good(),
                           DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

                s.fp_svc_penalties[i].resize(s.num_svc_categories);
                for (std::size_t j = 0; j < s.num_svc_categories; ++j)
                {
                    iss >> s.fp_svc_penalties[i][j];
                }

                // Move to ']'
                iss.ignore(std::numeric_limits<std::streamsize>::max(), ']');
                DCS_ASSERT(iss.good(),
                           DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file (']' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));
            }
        }
        else if (boost::istarts_with(line, "fn.min_powers"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            // Move to '['
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            s.fn_min_powers.resize(s.num_fn_categories);
            for (std::size_t i = 0; i < s.num_fn_categories; ++i)
            {
                iss >> s.fn_min_powers[i];
            }

            //// Move to ']'
            //iss.ignore(std::numeric_limits<std::streamsize>::max(), ']');
            //DCS_ASSERT(iss.good(),
            //           DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file (']' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));
        }
        else if (boost::istarts_with(line, "fn.max_powers"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            // Move to '['
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            s.fn_max_powers.resize(s.num_fn_categories);
            for (std::size_t i = 0; i < s.num_fn_categories; ++i)
            {
                iss >> s.fn_max_powers[i];
            }

            //// Move to ']'
            //iss.ignore(std::numeric_limits<std::streamsize>::max(), ']');
            //DCS_ASSERT(iss.good(),
            //         DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file (']' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));
        }
        else if (boost::istarts_with(line, "vm.cpu_requirements"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            // Move to '['
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            s.vm_cpu_requirements.resize(s.num_fps);
            for (std::size_t i = 0; i < s.num_vm_categories; ++i)
            {
                // Move to '['
                iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
                DCS_ASSERT(iss.good(),
                           DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

                s.vm_cpu_requirements[i].resize(s.num_fn_categories);
                for (std::size_t j = 0; j < s.num_fn_categories; ++j)
                {
                    iss >> s.vm_cpu_requirements[i][j];
                }

                // Move to ']'
                iss.ignore(std::numeric_limits<std::streamsize>::max(), ']');
                DCS_ASSERT(iss.good(),
                           DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file (']' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));
            }
        }
        else if (boost::istarts_with(line, "vm.ram_requirements"))
        {
            std::istringstream iss(line);

            // Move to '='
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '=');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('=' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            // Move to '['
            iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
            DCS_ASSERT(iss.good(),
                       DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

            s.vm_ram_requirements.resize(s.num_fps);
            for (std::size_t i = 0; i < s.num_vm_categories; ++i)
            {
                // Move to '['
                iss.ignore(std::numeric_limits<std::streamsize>::max(), '[');
                DCS_ASSERT(iss.good(),
                           DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file ('[' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));

                s.vm_ram_requirements[i].resize(s.num_fn_categories);
                for (std::size_t j = 0; j < s.num_fn_categories; ++j)
                {
                    iss >> s.vm_ram_requirements[i][j];
                }

                // Move to ']'
                iss.ignore(std::numeric_limits<std::streamsize>::max(), ']');
                DCS_ASSERT(iss.good(),
                           DCS_EXCEPTION_THROW(std::runtime_error, "Malformed scenario file (']' is missing at line " + stringify(lineno) + " and column " + stringify(iss.tellg()) + ")"));
            }
        }
    }

    // Assign default values
/*TODO
    if (s.svc_max_delays.size() == 0)
    {
        s.svc_max_delays.resize(s.num_svc_categories);
        std::fill(s.svc_max_delays.begin(), s.svc_max_delays.end(), default_svc_max_delay);
    }
    if (s.svc_vm_categories.size() == 0)
    {
        s.svc_vm_categories.resize(s.num_svc_categories);
        std::fill(s.svc_vm_categories.begin(), s.svc_vm_categories.end(), default_svc_vm_category);
    }
    if (s.fp_num_fns.size() == 0)
    {
        s.fp_num_fns.resize(s.num_fps);
        for (std::size_t i = 0; i < s.num_fps; ++i)
        {
            s.fp_num_fns[i].resize(s.num_fn_categories);
            std::fill(s.fp_num_fns[i].begin(), s.fp_num_fns[i].end(), default_fp_num_fns);
        }
    }
    if (s.fp_electricity_costs.size() == 0)
    {
        s.fp_electricity_costs.resize(s.num_fps);
        std::fill(s.fp_electricity_costs.begin(), s.fp_electricity_costs.end(), default_fp_electricity_cost);
    }
    if (s.fp_coalition_costs.size() == 0)
    {
        s.fp_coalition_costs.resize(s.num_fps);
        std::fill(s.fp_coalition_costs.begin(), s.fp_coalition_costs.end(), default_fp_coalition_cost);
    }
    if (s.fp_svc_revenues.size() == 0)
    {
        s.fp_svc_revenues.resize(s.num_fps);
        for (std::size_t i = 0; i < s.num_fps; ++i)
        {
            s.fp_svc_revenues[i].resize(s.num_svc_categories);
            std::fill(s.fp_svc_revenues[i].begin(), s.fp_svc_revenues[i].end(), default_fp_svc_revenue);
        }
    }
    if (s.fp_svc_penalties.size() == 0)
    {
        s.fp_svc_penalties.resize(s.num_fps);
        for (std::size_t i = 0; i < s.num_fps; ++i)
        {
            s.fp_svc_penalties[i].resize(s.num_svc_categories);
            std::fill(s.fp_svc_penalties[i].begin(), s.fp_svc_penalties[i].end(), default_fp_svc_penalty);
        }
    }
    if (s.fn_min_powers.size() == 0)
    {
        s.fn_min_powers.resize(s.num_fn_categories);
        std::fill(s.fn_min_powers.begin(), s.fn_min_powers.end(), default_fn_min_power);
    }
    if (s.fn_max_powers.size() == 0)
    {
        s.fn_max_powers.resize(s.num_fn_types);
        std::fill(s.fn_max_powers.begin(), s.fn_max_powers.end(), default_fn_max_power);
    }
*/

    // Post-parsing consistency checks
    DCS_ASSERT(s.num_fps > 0,
               DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of FPs"));
    DCS_ASSERT(s.num_fn_categories > 0,
               DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of FN categories"));
    DCS_ASSERT(s.num_svc_categories > 0,
               DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of service categories"));
    DCS_ASSERT(s.num_vm_categories > 0,
               DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of VM categories"));
    DCS_ASSERT(s.svc_max_delays.size() == s.num_svc_categories,
               DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of service categories in service maximum delays by service categoriy"));
    DCS_ASSERT(s.svc_vm_categories.size() == s.num_svc_categories,
               DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of service categories in service VM categories by service categoriy"));
    DCS_ASSERT(s.fp_num_svcs.size() == s.num_fps,
               DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of FPs in number of services by FPs"));
    for (std::size_t i = 0; i < s.num_fps; ++i)
    {
        DCS_ASSERT(s.fp_num_svcs[i].size() == s.num_svc_categories,
                   DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of service categories for FP " + stringify(i) + " in number of services by FP"));
    }
    DCS_ASSERT(s.fp_num_fns.size() == s.num_fps,
               DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of FPs in number of FNs by FP"));
    for (std::size_t i = 0; i < s.num_fps; ++i)
    {
        DCS_ASSERT(s.fp_num_fns[i].size() == s.num_fn_categories,
                   DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of FN categories for FP " + stringify(i) + " in number of FNs by FP"));
    }
    DCS_ASSERT(s.fp_electricity_costs.size() == s.num_fps,
               DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of FPs in electricity costs by FP"));
    DCS_ASSERT(s.fp_fn_asleep_costs.size() == s.num_fps,
               DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of FPs in FN asleep costs by FP"));
    for (std::size_t i = 0; i < s.num_fps; ++i)
    {
        DCS_ASSERT(s.fp_fn_asleep_costs[i].size() == s.num_fn_categories,
                   DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of FN categories for FP " + stringify(i) + " in FN asleep costs by FP"));
    }
    DCS_ASSERT(s.fp_fn_awake_costs.size() == s.num_fps,
               DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of FPs in FN asleep costs by FP"));
    for (std::size_t i = 0; i < s.num_fps; ++i)
    {
        DCS_ASSERT(s.fp_fn_awake_costs[i].size() == s.num_fn_categories,
                   DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of FN categories for FP " + stringify(i) + " in FN asleep costs by FP"));
    }
    DCS_ASSERT(s.fp_coalition_costs.size() == s.num_fps,
               DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of FPs in coalition costs by FP"));
    DCS_ASSERT(s.fp_svc_revenues.size() == s.num_fps,
               DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of FPs in service revenues by FP"));
    for (std::size_t i = 0; i < s.num_fps; ++i)
    {
        DCS_ASSERT(s.fp_svc_revenues[i].size() == s.num_svc_categories,
                   DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of service categories for FP " + stringify(i) + " in service revenues by FP"));
    }
    DCS_ASSERT(s.fp_svc_penalties.size() == s.num_fps,
               DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of FPs in service penalties by FP"));
    for (std::size_t i = 0; i < s.num_fps; ++i)
    {
        DCS_ASSERT(s.fp_svc_penalties[i].size() == s.num_svc_categories,
                   DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of service categories for FP " + stringify(i) + " in service penalties by FP"));
    }
    DCS_ASSERT(s.fn_min_powers.size() == s.num_fn_categories,
               DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of FN categories in FN min power consumptions by FN category"));
    DCS_ASSERT(s.fn_max_powers.size() == s.num_fn_categories,
               DCS_EXCEPTION_THROW(std::runtime_error, "Unexpected number of FN categories in FN max power consumptions by FN category"));

    return s;
}

template <typename CharT, typename CharTraitsT, typename RealT>
std::basic_ostream<CharT,CharTraitsT>& operator<<(std::basic_ostream<CharT,CharTraitsT>& os, const options_t<RealT>& opts)
{
    os  << "optim-relative-tolerance: " << opts.optim_relative_tolerance
        << ", optim-time-limit: " << opts.optim_time_limit
        << ", coalition-formation: " << opts.coalition_formation
        << ", coalition-value-division: " << opts.coalition_value_division
        << ", output-stats-data-file: " << opts.output_stats_data_file
        << ", output-trace-data-file: " << opts.output_trace_data_file
        << ", sim-ci-level: " << opts.sim_ci_level
        << ", sim-ci-relative-precision: " << opts.sim_ci_rel_precision
        << ", sim-max-num-replications: " << opts.sim_max_num_replications
        << ", sim-max-replication-duration: " << opts.sim_max_replication_duration
        << ", service-delay-tolerance: " << opts.service_delay_tolerance
        << ", verbosity: " << opts.verbosity;
        //<< ", simulation-mode: " << opts.simulation_mode;

    return os;
}


template <typename RealT>
class experiment_t: public simulator_t<RealT>
{
private:
    typedef simulator_t<RealT> base_type;

    enum verbosity_level_t
    {
        none = 0,
        low = 1,
        low_medium = 2,
        medium = 5,
        high = 9
    }; // verbosity_level_t

    enum event_tag_t
    {
        arrival_burst_start_event,
        arrival_burst_stop_event,
        coalition_formation_trigger_event
    }; // event_tag_t

    struct arrival_burst_event_state_t: public event_state_t
    {
        std::size_t service;
        RealT duration;
        RealT arrival_rate;
    }; // arrival_burst_event_state_t

    struct coalition_formation_trigger_event_state_t: public event_state_t
    {
        RealT start_time = -1;
        RealT stop_time = -1;
    }; // coalition_formation_trigger_event_state_t

    static const char field_quote_ch = '"';
    static const char field_sep_ch = ',';


public:
    experiment_t()
    : num_fns_(0),
      num_svcs_(0)
    {
    }

    void setup(const scenario_t<RealT>& scenario, const options_t<RealT>& options, random_number_engine_t& rng)
    {
        this->reset();

        scen_ = scenario;
        opts_ = options;
        rng_ = rng;

        fps_.resize(scen_.num_fps);
        std::iota(fps_.begin(), fps_.end(), 0);

        // Fills FN data structures and compute the total number of FNs
        for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
        {
            for (std::size_t fnc = 0; fnc < scen_.num_fn_categories; ++fnc)
            {
                const std::size_t nfns = scen_.fp_num_fns[fp][fnc];

                for (std::size_t i = 0; i < nfns; ++i)
                {
                    fn_fps_.push_back(fp);
                    fn_categories_.push_back(fnc);
                }

                num_fns_ += nfns;
            }
        }

        // Fills service data structures and computes the total number of services
        for (std::size_t p = 0; p < scen_.num_fps; ++p)
        {
            for (std::size_t s = 0; s < scen_.num_svc_categories; ++s)
            {
                const std::size_t nsvcs = scen_.fp_num_svcs[p][s];

                for (std::size_t i = 0; i < nsvcs; ++i)
                {
                    svc_fps_.push_back(p);
                    svc_categories_.push_back(s);
                }

                num_svcs_ += nsvcs;
            }
        }

        // Initialize workload generators
        wkl_gens_.resize(scen_.num_svc_categories);
        for (std::size_t i = 0; i < scen_.num_svc_categories; ++i)
        {
            std::vector<RealT> durations;
            std::vector<RealT> arr_rates;

            for (std::size_t j = 0; j < scen_.svc_workloads[i].size(); ++j)
            {
                durations.push_back(scen_.svc_workloads[i][j].first);
                arr_rates.push_back(scen_.svc_workloads[i][j].second);
            }

            wkl_gens_[i] = std::make_shared<multistep_workload_generator_t<RealT>>(durations.begin(), durations.end(), arr_rates.begin(), arr_rates.end());
        }
    }

    void reset()
    {
        //this->base_type::reset();

        num_fns_ = 0;
        num_svcs_ = 0;
        fps_.clear();
        fn_fps_.clear();
        fn_categories_.clear();
        svc_fps_.clear();
        svc_categories_.clear();
        wkl_gens_.clear();
        fp_coal_profit_ci_stats_.clear();
        fp_alone_profit_ci_stats_.clear();
        rep_fp_coal_profit_stats_.clear();
        rep_fp_alone_profit_stats_.clear();
    }

private:
    template <typename IterT>
    static bool check_stats(IterT first, IterT last)
    {
        while (first != last)
        {
            if (!(*first)->done() && !(*first)->unstable())
            {
                return false;
            }

            ++first;
        }

        return true;
    }

    void do_initialize_simulation()
    {
        // Initialize the simulator
        if (opts_.sim_max_num_replications > 0)
        {
            this->max_num_replications(opts_.sim_max_num_replications);
        }
        if (opts_.sim_max_replication_duration >= 0)
        {
            this->max_replication_duration(opts_.sim_max_replication_duration);
        }

        // Initialize confidence interval variables
        fp_coal_profit_ci_stats_.resize(scen_.num_fps);
        fp_alone_profit_ci_stats_.resize(scen_.num_fps);
        rep_fp_coal_profit_stats_.resize(scen_.num_fps);
        rep_fp_alone_profit_stats_.resize(scen_.num_fps);
        for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
        {
            std::ostringstream oss;

            oss << "CoalitionProfit_{" << fp << "}";
            fp_coal_profit_ci_stats_[fp] = std::make_shared<ci_mean_estimator_t<RealT>>(opts_.sim_ci_level, opts_.sim_ci_rel_precision);
            fp_coal_profit_ci_stats_[fp]->name(oss.str());
            rep_fp_coal_profit_stats_[fp] = std::make_shared<mean_estimator_t<RealT>>();
            rep_fp_coal_profit_stats_[fp]->name(oss.str());

            oss.str("");
            oss << "AloneProfit_{" << fp << "}";
            fp_alone_profit_ci_stats_[fp] = std::make_shared<ci_mean_estimator_t<RealT>>(opts_.sim_ci_level, opts_.sim_ci_rel_precision);
            fp_alone_profit_ci_stats_[fp]->name(oss.str());
            rep_fp_alone_profit_stats_[fp] = std::make_shared<mean_estimator_t<RealT>>();
            rep_fp_alone_profit_stats_[fp]->name(oss.str());
        }

        // Initialize output files

        if (!opts_.output_stats_data_file.empty())
        {
            stats_dat_ofs_.open(opts_.output_stats_data_file.c_str());

            DCS_ASSERT(stats_dat_ofs_,
                       DCS_EXCEPTION_THROW(std::runtime_error, "Unable to open output stats data file"));

            stats_dat_ofs_  << field_quote_ch  << "Timestamp" << field_quote_ch
                            << field_sep_ch << field_quote_ch  << "Coalition Formation Start Time" << field_quote_ch
                            << field_sep_ch << field_quote_ch << "Coalition Formation Duration" << field_quote_ch;
            for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
            {
                stats_dat_ofs_  << field_sep_ch << field_quote_ch << "FP " << fp << " - Coalition Profit" << field_quote_ch
                                << field_sep_ch << field_quote_ch << "FP " << fp << " - Alone Profit" << field_quote_ch
                                << field_sep_ch << field_quote_ch << "FP " << fp << " - Coalition Profit vs. Alone Profit" << field_quote_ch;
            }
            stats_dat_ofs_ << std::endl;
        }

        if (!opts_.output_trace_data_file.empty())
        {
            trace_dat_ofs_.open(opts_.output_trace_data_file.c_str());

            DCS_ASSERT(trace_dat_ofs_,
                       DCS_EXCEPTION_THROW(std::runtime_error, "Unable to open output trace data file"));

            trace_dat_ofs_  << field_quote_ch  << "Timestamp" << field_quote_ch
                            << field_sep_ch << field_quote_ch << "Coalition Formation Start Time" << field_quote_ch
                            << field_sep_ch << field_quote_ch << "Coalition Formation Duration" << field_quote_ch
                            << field_sep_ch << field_quote_ch << "Coalition Structure" << field_quote_ch;
            for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
            {
                trace_dat_ofs_  << field_sep_ch << field_quote_ch << "FP " << fp << " - Alone Profit" << field_quote_ch
                                << field_sep_ch << field_quote_ch << "FP " << fp << " - Coalition Profit" << field_quote_ch;
            }
            trace_dat_ofs_ << std::endl;
         }
    }

    void do_finalize_simulation()
    {
        if (stats_dat_ofs_.is_open())
        {
            stats_dat_ofs_.close();
        }
        if (trace_dat_ofs_.is_open())
        {
            trace_dat_ofs_.close();
        }

        if (opts_.verbosity > none)
        {
            DCS_LOGGING_STREAM << "-- CONFIDENCE INTERVALS OUTPUTS:" << std::endl;
            for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
            {
                DCS_LOGGING_STREAM << "  * FP " << fp << std::endl;
                DCS_LOGGING_STREAM << "   - Coalition profit statistics: " << fp_coal_profit_ci_stats_[fp]->estimate() << " (s.d. " << fp_coal_profit_ci_stats_[fp]->standard_deviation() << ") [" << fp_coal_profit_ci_stats_[fp]->lower() << ", " << fp_coal_profit_ci_stats_[fp]->upper() << "] (rel. prec.: " << fp_coal_profit_ci_stats_[fp]->relative_precision() << ", size: " << fp_coal_profit_ci_stats_[fp]->size() << ")" << std::endl;
                DCS_LOGGING_STREAM << "   - Alone profit statistics: " << fp_alone_profit_ci_stats_[fp]->estimate() << " (s.d. " << fp_alone_profit_ci_stats_[fp]->standard_deviation() << ") [" << fp_alone_profit_ci_stats_[fp]->lower() << ", " << fp_alone_profit_ci_stats_[fp]->upper() << "] (rel. prec.: " << fp_alone_profit_ci_stats_[fp]->relative_precision() << ", size: " << fp_alone_profit_ci_stats_[fp]->size() << ")" << std::endl;
            }
        }
    }

    void do_initialize_replication()
    {
        rep_fn_power_states_.resize(num_fns_);
        std::fill(rep_fn_power_states_.begin(), rep_fn_power_states_.end(), true);

        rep_fp_coal_profit_stats_.resize(scen_.num_fps);
        rep_fp_alone_profit_stats_.resize(scen_.num_fps);
        for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
        {
            // Reset replication stats

            rep_fp_coal_profit_stats_[fp]->reset();
            rep_fp_alone_profit_stats_[fp]->reset();
        }

        // Schedule initial events

        rep_svc_wkl_bursts_.resize(num_svcs_);
        for (std::size_t svc = 0; svc < num_svcs_; ++svc)
        {
            // Clear workload bursts

            rep_svc_wkl_bursts_[svc].clear();

            // Schedule new burst arrival

            assert( svc < svc_categories_.size() );

            auto const svc_cat = svc_categories_[svc];

            auto p_state = std::make_shared<arrival_burst_event_state_t>();
            p_state->service = svc;
            std::tie(p_state->duration, p_state->arrival_rate) = (*wkl_gens_[svc_cat])(rng_);
            this->schedule_event(this->simulated_time(), arrival_burst_start_event, p_state);
        }

        auto p_state = std::make_shared<coalition_formation_trigger_event_state_t>();
        p_state->start_time = this->simulated_time();
        p_state->stop_time = this->simulated_time() + opts_.coalition_formation_interval;
        this->schedule_event(p_state->stop_time, coalition_formation_trigger_event, p_state);
    }

    void do_finalize_replication()
    {
        // Collect stats

        for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
        {
            fp_coal_profit_ci_stats_[fp]->collect(rep_fp_coal_profit_stats_[fp]->estimate());
            fp_alone_profit_ci_stats_[fp]->collect(rep_fp_alone_profit_stats_[fp]->estimate());
        }

        if (opts_.verbosity >= low)
        {
            DCS_LOGGING_STREAM << "-- REPLICATION #" << this->num_replications() << std::endl;

            if (opts_.verbosity >= low_medium)
            {
                DCS_LOGGING_STREAM << " - SUMMARY OUTPUTS:" << std::endl;
                DCS_LOGGING_STREAM << "  - Total Coalition Profits: [" << scen_.num_fps << "]{";
                for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
                {
                    if (fp > 0)
                    {
                        DCS_LOGGING_STREAM << ",";
                    }

                    DCS_LOGGING_STREAM << rep_fp_coal_profit_stats_[fp]->estimate();
                }
                DCS_LOGGING_STREAM << "}" << std::endl;
                DCS_LOGGING_STREAM << "  - Total Alone Profits: [" << scen_.num_fps << "]{";
                for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
                {
                    if (fp > 0)
                    {
                        DCS_LOGGING_STREAM << ",";
                    }

                    DCS_LOGGING_STREAM << rep_fp_alone_profit_stats_[fp]->estimate();
                }
                DCS_LOGGING_STREAM << "}" << std::endl;
            }

            DCS_LOGGING_STREAM << " - CONFIDENCE INTERVALS OUTPUTS:" << std::endl;
            for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
            {
                DCS_LOGGING_STREAM << "  * FP " << fp << std::endl;
                DCS_LOGGING_STREAM << "   - Coalition profit statistics: " << fp_coal_profit_ci_stats_[fp]->estimate() << " (s.d. " << fp_coal_profit_ci_stats_[fp]->standard_deviation() << ") [" << fp_coal_profit_ci_stats_[fp]->lower() << ", " << fp_coal_profit_ci_stats_[fp]->upper() << "] (rel. prec.: " << fp_coal_profit_ci_stats_[fp]->relative_precision() << ", size: " << fp_coal_profit_ci_stats_[fp]->size() << ")" << std::endl;
                DCS_LOGGING_STREAM << "   - Alone profit statistics: " << fp_alone_profit_ci_stats_[fp]->estimate() << " (s.d. " << fp_alone_profit_ci_stats_[fp]->standard_deviation() << ") [" << fp_alone_profit_ci_stats_[fp]->lower() << ", " << fp_alone_profit_ci_stats_[fp]->upper() << "] (rel. prec.: " << fp_alone_profit_ci_stats_[fp]->relative_precision() << ", size: " << fp_alone_profit_ci_stats_[fp]->size() << ")" << std::endl;
            }
        }
    }

    bool do_check_end_of_replication() const
    {
        return false;
    }

    bool do_check_end_of_simulation() const
    {
        return  check_stats(fp_coal_profit_ci_stats_.cbegin(), fp_coal_profit_ci_stats_.cend());
            //||  check_stats(fp_alone_profit_ci_stats_.cbegin(), fp_alone_profit_ci_stats_.cend());
    }


    void do_process_event(const std::shared_ptr<event_t<RealT>>& p_event)
    {
        switch (p_event->tag)
        {
            case arrival_burst_start_event:
                this->process_arrival_burst_start_event(p_event);
                break;
            case arrival_burst_stop_event:
                this->process_arrival_burst_stop_event(p_event);
                break;
            case coalition_formation_trigger_event:
                this->process_coalition_formation_trigger_event(p_event);
                break;
            default:
                dcs::log_warn(DCS_LOGGING_AT, "Unable to process events with tag " + stringify(p_event->tag));
                break;
        }
    }

    void process_arrival_burst_start_event(const std::shared_ptr<event_t<RealT>>& p_event)
    {
        // check: event is available
        DCS_ASSERT(p_event,
                   DCS_EXCEPTION_THROW(std::invalid_argument, "Event is not available"));

        // check: event state is available
        DCS_ASSERT(p_event->p_state,
                   DCS_EXCEPTION_THROW(std::logic_error, "Event state is not available"));

        auto p_state = std::dynamic_pointer_cast<arrival_burst_event_state_t>(p_event->p_state);
        //auto p_state = dynamic_cast<arrival_burst_event_state_t*>(p_event->p_state.get());

        // check: down-cast OK
        DCS_DEBUG_ASSERT( p_state );

        auto const svc = p_state->service;
        //auto const svc_cat = svc_categories_[svc];

        DCS_DEBUG_TRACE("Processing 'ARRIVAL_BURST_START' event for service: " << svc << " (time: " << this->simulated_time() << ")");

        DCS_DEBUG_ASSERT( svc < rep_svc_wkl_bursts_.size() );

        auto burst_stop_time = this->simulated_time() + p_state->duration;

        rep_svc_wkl_bursts_[svc].push_back(std::make_tuple(this->simulated_time(), burst_stop_time, p_state->arrival_rate));

        this->schedule_event(burst_stop_time, arrival_burst_stop_event, p_state);
    }

    void process_arrival_burst_stop_event(const std::shared_ptr<event_t<RealT>>& p_event)
    {
        // check: event is available
        DCS_ASSERT(p_event,
                   DCS_EXCEPTION_THROW(std::invalid_argument, "Event is not available"));

        // check: event state is available
        DCS_ASSERT(p_event->p_state,
                   DCS_EXCEPTION_THROW(std::logic_error, "Event state is not available"));

        auto p_state = std::dynamic_pointer_cast<arrival_burst_event_state_t>(p_event->p_state);
        //auto p_state = dynamic_cast<arrival_burst_event_state_t*>(p_event->p_state.get());

        // check: down-cast OK
        DCS_DEBUG_ASSERT( p_state );

        auto const svc = p_state->service;
        auto const svc_cat = svc_categories_[svc];

        DCS_DEBUG_TRACE("Processing 'ARRIVAL_BURST_STOP' event for service: " << svc << " (time: " << this->simulated_time() << ")");

        p_state = std::make_shared<arrival_burst_event_state_t>();
        p_state->service = svc;
        std::tie(p_state->duration, p_state->arrival_rate) = (*wkl_gens_[svc_cat])(rng_);

        this->schedule_event(this->simulated_time(), arrival_burst_start_event, p_state);
    }

    void process_coalition_formation_trigger_event(const std::shared_ptr<event_t<RealT>>& p_event)
    {
        // check: event is available
        DCS_ASSERT(p_event,
                   DCS_EXCEPTION_THROW(std::invalid_argument, "Event is not available"));

        auto p_state = std::dynamic_pointer_cast<coalition_formation_trigger_event_state_t>(p_event->p_state);
        //auto p_state = dynamic_cast<coalition_formation_trigger_event_state_t*>(p_event->p_state.get());
        // check: down-cast OK
        DCS_DEBUG_ASSERT( p_state );

        DCS_DEBUG_TRACE("Processing 'COALITION_FORMATION_TRIGGER' event - start: " << p_state->start_time << ", stop: " << p_state->stop_time << " (time: " << this->simulated_time() << ")");

        this->analyze_coalitions(*p_state);

        // Schedule a new coalition formation trigger event

        p_state = std::make_shared<coalition_formation_trigger_event_state_t>();
        p_state->start_time = this->simulated_time();
        p_state->stop_time = this->simulated_time() + opts_.coalition_formation_interval;
        this->schedule_event(p_state->stop_time, coalition_formation_trigger_event, p_state);
    }

    void analyze_coalitions(const coalition_formation_trigger_event_state_t& coal_form_state)
    {
        namespace gt = gtpack;
        namespace alg = dcs::algorithm;

        // Analyze the best coalitions for the just finished trigger interval
        // NOTE: We use a backward evaluation: analyze the workload arrived to
        //       date and then analyze the best FP coalitions that should form
        //       to serve such workload.
        //       To use a forward evaluation (i.e., form coalitions to handle
        //       future workloads) we need to store somewhere the future
        //       incoming workload bursts (at least until the next activation
        //       of the coalition formation algorithm).

        auto const cur_timestamp = std::time(nullptr);
        auto const coal_form_start_time = coal_form_state.start_time;
        auto const coal_form_stop_time = coal_form_state.stop_time;
        auto const coalition_duration = coal_form_stop_time - coal_form_start_time;

        // Determines the maximum arrival rate of the workload bursts arrived in the last coalition formation interval

        std::vector<std::vector<RealT>> svc_predicted_delays(num_svcs_);
        std::vector<std::size_t> vm_svcs;
        for (std::size_t svc = 0; svc < num_svcs_; ++svc)
        {
            auto const svc_cat = svc_categories_[svc];

            RealT max_rate = 0;
            std::size_t b = 0;
            while (b < rep_svc_wkl_bursts_[svc].size())
            {
                RealT burst_start = 0;
                RealT burst_stop = 0;
                RealT arr_rate = 0;

                std::tie(burst_start, burst_stop, arr_rate) = rep_svc_wkl_bursts_[svc][b];

DCS_DEBUG_TRACE("Service: " << svc << ", Burst #" << b << ", start: " << burst_start << ", stop: " << burst_stop << ", rate: " << arr_rate);
                if (burst_stop <= coal_form_start_time)
                {
                    // This burst is arrived before this coalition formation interval -> erase
                    rep_svc_wkl_bursts_[svc].erase(rep_svc_wkl_bursts_[svc].cbegin()+b);
                }
                else if (burst_start < coal_form_stop_time)
                {
                    // This burst is inside this coalition formation interval -> analyze

                    if (max_rate < arr_rate)
                    {
                        max_rate = arr_rate;
                    }

                    if (burst_stop < coal_form_stop_time)
                    {
                        // This burst is already done -> erase
                        rep_svc_wkl_bursts_[svc].erase(rep_svc_wkl_bursts_[svc].cbegin()+b);
                    }
                    else
                    {
                        ++b;
                    }
                }
                else
                {
                    // This burst starts in the next coalition formation interval and thus will be considered later
                    break;
                }
            }

            // Predict delays for this service
            MMc<double> svc_perf_model(max_rate, scen_.svc_vm_service_rates[svc_cat], scen_.svc_max_delays[svc_cat], opts_.service_delay_tolerance);
            auto min_num_vms = svc_perf_model.computeQueueParameters(true);
            svc_perf_model.getDelays(&svc_predicted_delays[svc]);

            DCS_DEBUG_TRACE("Service: " << svc << ", arrival rate: " << max_rate << ", service rate: " << scen_.svc_vm_service_rates[svc_cat] << ", max delay: " << scen_.svc_max_delays[svc_cat] << " -> Min number of VMs: " << min_num_vms << ", Predicted delay: " << svc_predicted_delays[svc].back());

            vm_svcs.insert(vm_svcs.end(), min_num_vms, svc);
        }

        std::vector<RealT> fp_interval_alone_profits(scen_.num_fps, std::numeric_limits<RealT>::quiet_NaN());
        std::vector<RealT> fp_interval_coal_profits(scen_.num_fps, std::numeric_limits<RealT>::quiet_NaN());

        // Solve the coalition formation problem

        gt::cooperative_game<RealT> game(scen_.num_fps, boost::make_shared<gt::enumerated_characteristic_function<RealT>>());

        std::map<gt::cid_type,coalition_info_t<RealT>> visited_coalitions;
        std::map<gt::pid_type, std::vector<partition_info_t<RealT>>> best_partitions;
        //bool found_same_struc(false);

        alg::lexicographic_subset subset(scen_.num_fps, false);

        while (subset.has_next())
        {
            //typedef typename alg::subset_traits<std::size_t>::element_container element_container;

            DCS_DEBUG_TRACE("--- SUBSET: " << subset);//XXX

            auto coal_fps = alg::next_subset(fps_.begin(), fps_.end(), subset);

            auto cid = gt::make_coalition_id(coal_fps.begin(), coal_fps.end());

            DCS_DEBUG_TRACE("--- COALITION: " << game.coalition(cid) << " (CID=" << cid << ")");//XXX

            auto const coal_num_fps = coal_fps.size();

            std::vector<std::size_t> coal_fns;
            std::vector<std::size_t> coal_svcs;
            std::vector<std::size_t> coal_vms;
            //TODO: could be optimized by using auxiliary data structures (e.g., for each FP, stores the set of its service)
            for (std::size_t i = 0; i < coal_num_fps; ++i)
            {
                auto const fp = coal_fps[i];

                for (std::size_t fn = 0; fn < num_fns_; ++fn)
                {
                    if (fn_fps_[fn] == fp)
                    {
                        coal_fns.push_back(fn);
                    }
                }

                for (std::size_t svc = 0; svc < num_svcs_; ++svc)
                {
                    if (svc_fps_[svc] == fp)
                    {
                        coal_svcs.push_back(svc);

                        auto num_vms = vm_svcs.size();
                        for (std::size_t vm = 0; vm < num_vms; ++vm)
                        {
                            if (vm_svcs[vm] == svc)
                            {
                                coal_vms.push_back(vm);
                            }
                        }
                    }
                }
            }

            fgt::optimal_vm_allocation_solver_t<RealT> opt_solver(opts_.optim_relative_tolerance, opts_.optim_time_limit);
            fgt::vm_allocation_t<RealT> vm_alloc;

            vm_alloc = opt_solver(coal_fns,
                                  coal_vms,
                                  fn_fps_,
                                  fn_categories_,
                                  rep_fn_power_states_,
                                  scen_.fn_min_powers,
                                  scen_.fn_max_powers,
                                  vm_svcs,
                                  scen_.svc_vm_categories,
                                  scen_.vm_cpu_requirements,
                                  scen_.vm_ram_requirements,
                                  svc_fps_,
                                  svc_categories_,
                                  scen_.svc_max_delays,
                                  svc_predicted_delays,
                                  scen_.fp_svc_penalties,
                                  scen_.fp_electricity_costs,
                                  scen_.fp_fn_asleep_costs,
                                  scen_.fp_fn_awake_costs);

            visited_coalitions[cid].vm_allocation = vm_alloc;

            if (vm_alloc.solved)
            {
                // Compute game values (i.e., coalition profits)

                RealT revenue = 0;
                for (auto svc : coal_svcs)
                {
                    auto const fp = svc_fps_[svc];
                    auto const svc_cat = svc_categories_[svc];

                    revenue += scen_.fp_svc_revenues[fp][svc_cat];
                }

                RealT cost = vm_alloc.objective_value;

                if (coal_num_fps > 1)
                {
                    for (std::size_t i = 0; i < coal_num_fps; ++i)
                    {
                        auto const fp = coal_fps[i];

                        cost -= scen_.fp_coalition_costs[fp];
                    }
                }

                auto const profit = (revenue-cost)*coalition_duration;

                game.value(cid, profit);
                visited_coalitions[cid].value = profit;

                // Collect stats for singleton (alone) coalitions

                if (coal_num_fps == 1)
                {
                    auto const fp = coal_fps[0];

                    fp_interval_alone_profits[fp] = profit;
                }

                DCS_DEBUG_TRACE( "CID: " << cid << " - VM allocation objective value: " << vm_alloc.objective_value << " => v(CID)=" << game.value(cid) );

                gt::cooperative_game<RealT> subgame = game.subgame(coal_fps.begin(), coal_fps.end());
                gt::core<RealT> core = gt::find_core(subgame);
                if (core.empty())
                {
                    DCS_DEBUG_TRACE( "CID: " << cid << " - The core is empty" );

                    visited_coalitions[cid].core_empty = true;
                    visited_coalitions[cid].payoffs_in_core = false;

                    if (subgame.num_players() == scen_.num_fps)
                    {
                        DCS_DEBUG_TRACE( "CID: " << cid << " - The grand-coalition has an empty core" );
                    }
                }
                else
                {
                    DCS_DEBUG_TRACE( "CID: " << cid << " - The core is not empty" );

                    visited_coalitions[cid].core_empty = false;
                }

                // Compute the coalition payoffs (i.e., FP profits)

                std::map<gt::pid_type,RealT> coal_payoffs = gt::shapley_value(subgame);

#ifdef DCS_DEBUG
                for (auto fp : coal_fps)
                {
                    DCS_DEBUG_TRACE( "CID: " << cid << " - FP: " << fp << " - Coalition payoff: " << coal_payoffs[fp] );
                }
#endif // DCS_DEBUG

                visited_coalitions[cid].payoffs = coal_payoffs;

                // Check if the value is in the core (if the core != empty)

                if (!visited_coalitions.at(cid).core_empty)
                {
                    if (gtpack::belongs_to_core(subgame, coal_payoffs.begin(), coal_payoffs.end()))
                    {
                        DCS_DEBUG_TRACE( "CID: " << cid << " - The coalition value belongs to the core" );

                        visited_coalitions[cid].payoffs_in_core = true;
                    }
                    else
                    {
                        DCS_DEBUG_TRACE( "CID: " << cid << " - The coaition value does not belong to the core" );

                        visited_coalitions[cid].payoffs_in_core = false;
                    }
                }
            }
            else
            {
                DCS_DEBUG_TRACE( "CID: " << cid << " - The VM assignment problem is infeasible" );

                visited_coalitions[cid].core_empty = true;
                visited_coalitions[cid].payoffs_in_core = false;

                game.value(cid, -std::numeric_limits<RealT>::min());

                if (game.coalition(cid).num_players() == scen_.num_fps)
                {
                    // This is the grand coalition

                    DCS_DEBUG_TRACE( "CID: " << cid << " - The grand-coalition has an infeasible solution and thus an empty core" );
                }
            }
        }

        // Form stable coalitions

        coalition_formation_info_t<RealT> formed_coalitions;

        formed_coalitions.coalitions = visited_coalitions;
        switch (opts_.coalition_formation)
        {
            case nash_stable_coalition_formation:
                formed_coalitions.best_partitions = nash_stable_partition_selector_t<RealT>()(game, visited_coalitions);
                break;
            default:
                DCS_EXCEPTION_THROW(std::runtime_error, "Unknown coalition formation stability selector");
        }

#ifdef DCS_DEBUG
        DCS_DEBUG_STREAM << "FORMED PARTITIONS: " << std::endl;
        for (auto const& part : formed_coalitions.best_partitions)
        {
            typedef typename std::set<gt::cid_type>::const_iterator coalition_iterator;
            coalition_iterator coal_end_it(part.coalitions.end());
            DCS_DEBUG_STREAM << "  [";
            for (auto cid : part.coalitions)
            {
                DCS_DEBUG_STREAM << cid << ",";
            }
            DCS_DEBUG_STREAM << "]" << std::endl;
        }
#endif // DCS_DEBUG

        // Collects statistics and outputs some information

        if (trace_dat_ofs_.is_open())
        {
            trace_dat_ofs_ << cur_timestamp << field_sep_ch << coal_form_start_time << field_sep_ch << coalition_duration;
        }

        if (opts_.find_all_best_partitions)
        {
            // Auxiliary statistics for collecting FP profits of all best partitions and then taking their averages
            std::vector<std::shared_ptr<mean_estimator_t<RealT>>> fp_interval_coal_profit_stats(scen_.num_fps);
            for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
            {
                fp_interval_coal_profit_stats[fp] = std::make_shared<mean_estimator_t<RealT>>();
            }

            for (auto const& best_partition : formed_coalitions.best_partitions)
            {
                std::vector<RealT> part_fp_interval_coal_profits(scen_.num_fps, std::numeric_limits<RealT>::quiet_NaN());

                for (auto const& cid : best_partition.coalitions)
                {
                    // Compute the interval and the total payoffs, and the total payoff errors
                    for (auto const& payoff_info : formed_coalitions.coalitions.at(cid).payoffs)
                    {
                        auto const fp = payoff_info.first;
                        auto const payoff = payoff_info.second;

                        part_fp_interval_coal_profits[fp] = payoff;
                        fp_interval_coal_profit_stats[fp]->collect(payoff);
                    }
                }

                if (opts_.verbosity >= medium)
                {
                    auto const num_coalitions = best_partition.coalitions.size();

                    DCS_LOGGING_STREAM << "-- INTERVAL OUTPUTS FOR BEST PARTITION: (" << num_coalitions << "]{";
                    for (auto const& cid : best_partition.coalitions)
                    {
                        DCS_LOGGING_STREAM << "{";
                        for (auto const& payoff_info : formed_coalitions.coalitions.at(cid).payoffs)
                        {
                            auto const fp = payoff_info.first;

                            DCS_LOGGING_STREAM << ",";
                            DCS_LOGGING_STREAM << fp;
                        }
                        DCS_LOGGING_STREAM << "},";
                    }
                    DCS_LOGGING_STREAM << "}" << std::endl;
                    DCS_LOGGING_STREAM << "- Interval Coalition Profits (in $): [" << num_coalitions << "]{";
                    for (auto const& cid : best_partition.coalitions)
                    {
                        DCS_LOGGING_STREAM << "{";
                        for (auto const& payoff_info : formed_coalitions.coalitions.at(cid).payoffs)
                        {
                            auto const fp = payoff_info.first;

                            DCS_LOGGING_STREAM << ",";
                            DCS_LOGGING_STREAM << "(" << fp << "=>" << part_fp_interval_coal_profits[fp] << ")";
                        }
                        DCS_LOGGING_STREAM << "},";
                    }
                    DCS_LOGGING_STREAM << "}" << std::endl;
                    DCS_LOGGING_STREAM << "- Interval Alone Profits (in $): [" << num_coalitions << "]{";
                    for (auto const& cid : best_partition.coalitions)
                    {
                        DCS_LOGGING_STREAM << "{";
                        for (auto const& payoff_info : formed_coalitions.coalitions.at(cid).payoffs)
                        {
                            auto const fp = payoff_info.first;

                            DCS_LOGGING_STREAM << ",";
                            DCS_LOGGING_STREAM << "(" << fp << "=>" << fp_interval_alone_profits[fp] << ")";
                        }
                        DCS_LOGGING_STREAM << "}";
                    }

                    DCS_LOGGING_STREAM << "}" << std::endl;
                    DCS_LOGGING_STREAM << "-----------------------------------------------------------------------" << std::endl;
                }

                // Outputs to trace file
                if (trace_dat_ofs_.is_open())
                {
                    trace_dat_ofs_ << field_sep_ch << field_quote_ch << "{";
                    for (auto const& cid : best_partition.coalitions)
                    {
                        trace_dat_ofs_ << "{";
                        for (auto const& payoff_info : formed_coalitions.coalitions.at(cid).payoffs)
                        {
                            auto const fp = payoff_info.first;

                            trace_dat_ofs_ << ",";
                            trace_dat_ofs_ << fp;
                        }
                        trace_dat_ofs_ << "}";
                    }
                    trace_dat_ofs_ << "}" << field_quote_ch;

                    for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
                    {
                        trace_dat_ofs_  << field_sep_ch << fp_interval_alone_profits[fp]
                                        << field_sep_ch << part_fp_interval_coal_profits[fp];
                    }

                    trace_dat_ofs_ << std::endl;
                }

            }

            for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
            {
                fp_interval_coal_profits[fp] = fp_interval_coal_profit_stats[fp]->estimate();
            }
        }
        else
        {
            partition_info_t<RealT> max_best_partition;

            for (auto const& best_partition : formed_coalitions.best_partitions)
            {
                if (best_partition.value > max_best_partition.value)
                {
                    max_best_partition = best_partition;
                }
            }

            for (auto const cid : max_best_partition.coalitions)
            {
                // Compute the interval and the total payoffs, and the total payoff errors
                for (auto const& payoff_info : formed_coalitions.coalitions.at(cid).payoffs)
                {
                    auto const fp = payoff_info.first;
                    auto const payoff = payoff_info.second;

                    fp_interval_coal_profits[fp] = payoff;
                }
            }

            // Output results

            if (opts_.verbosity >= medium)
            {
                auto const num_coalitions = max_best_partition.coalitions.size();

                DCS_LOGGING_STREAM << "-- INTERVAL OUTPUTS FOR BEST PARTITION: (" << num_coalitions << "]{";
                for (auto const cid : max_best_partition.coalitions)
                {
                    DCS_LOGGING_STREAM << "{";
                    for (auto const& payoff_info : formed_coalitions.coalitions.at(cid).payoffs)
                    {
                        auto const fp = payoff_info.first;

                        DCS_LOGGING_STREAM << ",";
                        DCS_LOGGING_STREAM << fp;
                    }
                    DCS_LOGGING_STREAM << "}";
                }
                DCS_LOGGING_STREAM << "}" << std::endl;
                DCS_LOGGING_STREAM << "- Interval Coalition Profits (in $): [" << num_coalitions << "]{";
                for (auto const cid : max_best_partition.coalitions)
                {
                    DCS_LOGGING_STREAM << "{";
                    for (auto const& payoff_info : formed_coalitions.coalitions.at(cid).payoffs)
                    {
                        auto const fp = payoff_info.first;

                        DCS_LOGGING_STREAM << ",";
                        DCS_LOGGING_STREAM << "(" << fp << "=>" << fp_interval_coal_profits[fp] << ")";
                    }
                    DCS_LOGGING_STREAM << "}";
                }
                DCS_LOGGING_STREAM << "}" << std::endl;
                DCS_LOGGING_STREAM << "- Interval Alone Profits (in $): [" << num_coalitions << "]{";
                for (auto const cid : max_best_partition.coalitions)
                {
                    DCS_LOGGING_STREAM << "{";
                    for (auto const& payoff_info : formed_coalitions.coalitions.at(cid).payoffs)
                    {
                        auto const fp = payoff_info.first;

                        DCS_LOGGING_STREAM << ",";
                        DCS_LOGGING_STREAM << "(" << fp << "=>" << fp_interval_alone_profits[fp] << ")";
                    }
                    DCS_LOGGING_STREAM << "}";
                }
                DCS_LOGGING_STREAM << "}" << std::endl;
                DCS_LOGGING_STREAM << "-----------------------------------------------------------------------" << std::endl;
            }

            // Outputs to trace file

            if (trace_dat_ofs_.is_open())
            {
                trace_dat_ofs_ << field_sep_ch << field_quote_ch << "{";
                for (auto const cid : max_best_partition.coalitions)
                {
                    trace_dat_ofs_ << "{";
                    for (auto const& payoff_info : formed_coalitions.coalitions.at(cid).payoffs)
                    {
                        auto const fp = payoff_info.first;

                        trace_dat_ofs_ << ",";
                        trace_dat_ofs_ << fp;
                    }
                    trace_dat_ofs_ << "}";
                }
                trace_dat_ofs_ << "}" << field_quote_ch;

                for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
                {
                    trace_dat_ofs_  << field_sep_ch << fp_interval_alone_profits[fp]
                                    << field_sep_ch << fp_interval_coal_profits[fp];
                }

                trace_dat_ofs_ << std::endl;
            }

        }

        // Collect replication stats

        for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
        {
            rep_fp_coal_profit_stats_[fp]->collect(fp_interval_coal_profits[fp]);
            rep_fp_alone_profit_stats_[fp]->collect(fp_interval_alone_profits[fp]);
        }

        // Outputs some information

        if (opts_.verbosity >= medium)
        {
            DCS_LOGGING_STREAM << "-- AVERAGED INTERVAL OUTPUTS:" << std::endl;
            DCS_LOGGING_STREAM << "- Average Coalition Profits: [" << scen_.num_fps << "]{";
            for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
            {
                if (fp > 0)
                {
                    DCS_LOGGING_STREAM << ",";
                }

                DCS_LOGGING_STREAM << fp_interval_coal_profits[fp];
            }
            DCS_LOGGING_STREAM << "}" << std::endl;
            DCS_LOGGING_STREAM << "- Alone Profits: [" << scen_.num_fps << "]{";
            for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
            {
                if (fp > 0)
                {
                    DCS_LOGGING_STREAM << ",";
                }

                DCS_LOGGING_STREAM << fp_interval_alone_profits[fp];
            }
            DCS_LOGGING_STREAM << "}" << std::endl;

            if (opts_.verbosity >= high)
            {
                std::cout << "-- INCREMENTAL AVERAGED INTERVAL OUTPUTS:" << std::endl;
                std::cout << "- Incremental Average Coalition Profits: [" << scen_.num_fps << "]{";
                for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
                {
                    if (fp > 0)
                    {
                        DCS_LOGGING_STREAM << ",";
                    }

                    DCS_LOGGING_STREAM << rep_fp_coal_profit_stats_[fp]->estimate();
                }
                std::cout << "}" << std::endl;
                std::cout << "- Incremental Alone Profits: [" << scen_.num_fps << "]{";
                for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
                {
                    if (fp > 0)
                    {
                        DCS_LOGGING_STREAM << ",";
                    }

                    DCS_LOGGING_STREAM << rep_fp_alone_profit_stats_[fp]->estimate();
                }
                DCS_LOGGING_STREAM << "}" << std::endl;
            }
        }

        // Output to file
        if (stats_dat_ofs_.is_open())
        {
            stats_dat_ofs_ << cur_timestamp << field_sep_ch << coal_form_start_time << field_sep_ch << coalition_duration;
            for (std::size_t fp = 0; fp < scen_.num_fps; ++fp)
            {
                stats_dat_ofs_  << field_sep_ch << fp_interval_coal_profits[fp]
                                << field_sep_ch << fp_interval_alone_profits[fp]
                                << field_sep_ch << relative_increment(fp_interval_coal_profits[fp], fp_interval_alone_profits[fp]);
            }
            stats_dat_ofs_ << std::endl;
        }
    }


private:
    scenario_t<RealT> scen_;
    options_t<RealT> opts_;
    random_number_engine_t rng_;
    std::size_t num_fns_;
    std::size_t num_svcs_;
    std::vector<std::size_t> fps_; // FP identities
    std::vector<std::size_t> fn_fps_; // Map an FN to the FP that owns it
    std::vector<std::size_t> fn_categories_; // Map an FN to its category
    std::vector<std::size_t> svc_fps_; // Map a service to the FP that runs it
    std::vector<std::size_t> svc_categories_; // Map a service to its category
    std::vector<std::shared_ptr<workload_generator_t<RealT>>> wkl_gens_;
    std::vector<std::vector<std::tuple<RealT,RealT,RealT>>> rep_svc_wkl_bursts_; ///< Arrival burst profiles (a sequence of <start-time,stop-time,arrival-rate> triples) in a single replication, by service
    std::vector<std::shared_ptr<mean_estimator_t<RealT>>> rep_fp_coal_profit_stats_; ///< FP coalition profits in a single replication, by FP
    std::vector<std::shared_ptr<mean_estimator_t<RealT>>> rep_fp_alone_profit_stats_; ///< FP alone profits in a single replication, by FP
    std::vector<bool> rep_fn_power_states_;
    std::vector<std::shared_ptr<ci_mean_estimator_t<RealT>>> fp_coal_profit_ci_stats_; // FP coalition profits along all the simulation, by FP
    std::vector<std::shared_ptr<ci_mean_estimator_t<RealT>>> fp_alone_profit_ci_stats_; // FP alone profits along all the simulation, by FP
    std::ofstream stats_dat_ofs_;
    std::ofstream trace_dat_ofs_;
}; // experiment_t


}} // Namespace dcs::fgt

#endif // DCS_FGT_EXPERIMENT_HPP
