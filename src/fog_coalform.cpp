/* vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */

/**
 * \file src/fgt_coalform.cpp
 *
 * \brief Form stable coalition among a set of fog nodes.
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


#include <boost/timer.hpp>
#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/cli.hpp>
#include <dcs/debug.hpp>
#include <dcs/exception.hpp>
#include <dcs/fgt/coalition_formation.hpp>
#include <dcs/fgt/experiment.hpp>
#include <dcs/logging.hpp>
//#include <dcs/macro.hpp>
#include <exception>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>


namespace cli = dcs::cli;
namespace fgt = dcs::fgt;


namespace /*<unnamed>*/ { namespace detail {

class cli_options_t;

template <typename CharT, typename CharTraitsT>
std::basic_ostream<CharT,CharTraitsT>& operator<<(std::basic_ostream<CharT,CharTraitsT>& os, const cli_options_t& opts);

cli_options_t parse_cli_options(int argc, char* argv[]);

void usage(char const* progname);


struct cli_options_t
{
    cli_options_t()
    : help(false),
      coalition_formation(fgt::nash_stable_coalition_formation),
      coalition_formation_interval(0),
      coalition_value_division(fgt::shapley_coalition_value_division),
      find_all_best_partitions(false),
      optim_relative_tolerance(0),
      optim_time_limit(-1),
      rng_seed(5489),
      service_delay_tolerance(1e-5),
      sim_ci_level(0.95),
      sim_ci_rel_precision(0.04),
      sim_max_num_replications(0),
      sim_max_replication_duration(0),
      verbosity(0)
    {
    }


    bool help;
    fgt::coalition_formation_category coalition_formation; ///< The strategy according which form coalitions
    double coalition_formation_interval; ///< The time interval at which the coalition formation algorithm activates (in terms of simulated time)
    fgt::coalition_value_division_category coalition_value_division;
    bool find_all_best_partitions; ///< A \c true value means that all possible best partitions are computed
    double optim_relative_tolerance; ///< The relative tolerance option to set to the optimizer
    double optim_time_limit; ///< The time limit option to set to the optimizer
    std::string output_stats_data_file; ///< The path to the output stats data file
    std::string output_trace_data_file; ///< The path to the output trace data file
    unsigned long rng_seed; ///< The seed used for random number generation
    std::string scenario_file; ///< The path to the input scenario file
    double service_delay_tolerance; ///< The relative tolerance to set in the service performance model
    double sim_ci_level; ///< Level for confidence intervals
    double sim_ci_rel_precision; ///< Relative precision for the half-width of the confidence intervals
    std::size_t sim_max_num_replications; ///< Maximum number of replications (0 means 'unlimited')
    double sim_max_replication_duration; ///< Length of each replication (in terms of simulated time)
    int verbosity; ///< The verbosity level: 0 for 'minimum' and 9 for 'maximum' verbosity level
}; // cli_options_t


cli_options_t parse_cli_options(int argc, char* argv[])
{
    std::string opt_str;
    cli_options_t opt;

    DCS_DEBUG_TRACE("Parse CLI options...");//XXX

    opt.help = cli::simple::get_option(argv, argv+argc, "--help");
    opt_str = cli::simple::get_option<std::string>(argv, argv+argc, "--formation", "nash");
    if (opt_str == "nash")
    {
        opt.coalition_formation = fgt::nash_stable_coalition_formation;
    }
    else
    {
        DCS_EXCEPTION_THROW(std::invalid_argument, "Unknown coalition formation category");
    }
    opt.coalition_formation_interval = cli::simple::get_option<double>(argv, argv+argc, "--formation-interval", 0);
    opt_str = cli::simple::get_option<std::string>(argv, argv+argc, "--payoff", "shapley");
    if (opt_str == "shapley")
    {
        opt.coalition_value_division = fgt::shapley_coalition_value_division;
    }
    else
    {
        DCS_EXCEPTION_THROW(std::invalid_argument, "Unknown coalition value division category.");
    }
    opt.find_all_best_partitions = cli::simple::get_option(argv, argv+argc, "--find-all-parts");
    opt.optim_relative_tolerance = cli::simple::get_option<double>(argv, argv+argc, "--optim-reltol", 0);
    opt.optim_time_limit = cli::simple::get_option<double>(argv, argv+argc, "--optim-tilim", -1);
    opt.output_stats_data_file = cli::simple::get_option<std::string>(argv, argv+argc, "--out-stats-file");
    opt.output_trace_data_file = cli::simple::get_option<std::string>(argv, argv+argc, "--out-trace-file");
    opt.rng_seed = cli::simple::get_option<unsigned long>(argv, argv+argc, "--rng-seed", 5489);
    opt.scenario_file = cli::simple::get_option<std::string>(argv, argv+argc, "--scenario");
    opt.service_delay_tolerance = cli::simple::get_option<double>(argv, argv+argc, "--service-delay-tol", 1e-5);
    opt.sim_ci_level = cli::simple::get_option<double>(argv, argv+argc, "--ci-level", 0.95);
    opt.sim_ci_rel_precision = cli::simple::get_option<double>(argv, argv+argc, "--ci-rel-precision", 0.04);
    opt.sim_max_num_replications = cli::simple::get_option<std::size_t>(argv, argv+argc, "--sim-max-num-rep", 0);
    opt.sim_max_replication_duration = cli::simple::get_option<double>(argv, argv+argc, "--sim-max-rep-len", 0);
    opt.verbosity = cli::simple::get_option<short>(argv, argv+argc, "--verbosity", 0);
    if (opt.verbosity < 0)
    {
        opt.verbosity = 0;
    }
    else if (opt.verbosity > 9)
    {
        opt.verbosity = 9;
    }

    // Check CLI options
    if (opt.scenario_file.empty())
    {
        DCS_EXCEPTION_THROW( std::invalid_argument, "Scenario file not specified" );
    }

    return opt;
}

template <typename CharT, typename CharTraitsT>
std::basic_ostream<CharT,CharTraitsT>& operator<<(std::basic_ostream<CharT,CharTraitsT>& os, const cli_options_t& opts)
{
    os  << "help: " << opts.help
        << ", coalition-formation: " << opts.coalition_formation
        << ", coalition-formation-interval: " << opts.coalition_formation_interval
        << ", coalition-value-division: " << opts.coalition_value_division
        << ", optim-relative-tolerance: " << opts.optim_relative_tolerance
        << ", optim-time-limit: " << opts.optim_time_limit
        << ", output-stats-data-file: " << opts.output_stats_data_file
        << ", output-trace-data-file: " << opts.output_trace_data_file
        << ", random-generator-seed: " << opts.rng_seed
        << ", scenario-file: " << opts.scenario_file
        << ", sim-ci-level: " << opts.sim_ci_level
        << ", sim-ci-relative-precision: " << opts.sim_ci_rel_precision
        << ", sim-max-num-replications: " << opts.sim_max_num_replications
        << ", sim-max-replication-duration: " << opts.sim_max_replication_duration
        << ", service-delay-tolerance: " << opts.service_delay_tolerance
        << ", verbosity: " << opts.verbosity;

    return os;
}

void usage(char const* progname)
{
    std::cerr << "Usage: " << progname << " [options]" << std::endl
              << "Options:" << std::endl
              << "--help" << std::endl
              << "  Show this message." << std::endl
              << "--service-delay-tol <num>" << std::endl
              << "  Real number in [0,1] denoting the relative tolerance for the delay used in the service performance model." << std::endl
              << "--find-all-parts" << std::endl
              << "  For each time interval, find all possible stable partitions." << std::endl
              << "--formation {'nash'}" << std::endl
              << "  Coalition formation category, where:" << std::endl
              << "  * 'nash' refers to the Nash-stable coalition formation;" << std::endl
              << "--formation-interval <num>" << std::endl
              << "  Real number >= 0 denoting the activating time interval of the coalition formation algorithm." << std::endl
              << "--optim-reltol <num>" << std::endl
              << "  Real number in [0,1] denoting the relative tolerance parameter in the optimizer." << std::endl
              << "--optim-tilim <num>" << std::endl
              << "  Real positive number denoting the maximum number of seconds to wait for the termination of the optimizer." << std::endl
              << "--output-stats-file <file>" << std::endl
              << "  The output file where writing statistics." << std::endl
              << "--output-trace-file <file>" << std::endl
              << "  The output file where writing run-trace information." << std::endl
              << "--payoff {'shapley'}" << std::endl
              << "  Payoff division category, where:" << std::endl
              << "  * 'shapley' refers to the Shapley value." << std::endl
              << "--rng-seed <num>" << std::endl
              << "  Set the seed to use for random number generation." << std::endl
              << "--scenario <file>" << std::endl
              << "  The path to the file describing the scenario to use for the experiment." << std::endl
              << "--sim-ci-level <num>" << std::endl
              << "  Level for the confidence intervals (must be a number in [0,1])." << std::endl
              << "--sim-ci-rel-precision <num>" << std::endl
              << "  Relative precision for the half-width of the confidence intervals (must be a number in [0,1])." << std::endl
              << "--sim-max-rep-len <num>" << std::endl
              << "  Real number >= 0 denoting the maximum duration of each independent replication." << std::endl
              << "--sim-max-num-rep <num>" << std::endl
              << "  Integer number >= 0 denoting the maximum number of independent replications. Use 0 for an unlimited number of replications." << std::endl
              << "--verbosity <num>" << std::endl
              << "  An integer number in [0,9] representing the verbosity level (0 for 'minimum verbosity' and 9 for 'maximum verbosity)." << std::endl
              << std::endl;
}

template <typename RealT, typename RNGT>
void run_experiment(const fgt::scenario_t<RealT>& scen, const fgt::options_t<RealT>& opts, RNGT& rng)
{
    boost::timer timer;

    std::cout << "- Scenario: " << scen << std::endl;
    std::cout << "- Options: " << opts << std::endl;

    fgt::experiment_t<RealT> exp;
    exp.setup(scen, opts, rng);
    exp.run();
}

}} // Namespace <unnamed>::detail



int main(int argc, char* argv[])
{
    typedef double real_t;

    try
    {
        detail::cli_options_t cli_opts;
        cli_opts = detail::parse_cli_options(argc, argv);
        if (cli_opts.help)
        {
            detail::usage(argv[0]);
            return 0;
        }

        // Prepare the experiment
        DCS_DEBUG_TRACE("Preparing the experiment...");//XXX
        fgt::scenario_t<real_t> scenario;
        scenario = fgt::make_scenario<real_t>(cli_opts.scenario_file);
        DCS_DEBUG_TRACE("Scenario: " << scenario);
        fgt::options_t<real_t> options;
        options.optim_relative_tolerance = cli_opts.optim_relative_tolerance;
        options.optim_time_limit = cli_opts.optim_time_limit;
        options.coalition_formation = cli_opts.coalition_formation;
        options.coalition_formation_interval = cli_opts.coalition_formation_interval;
        options.coalition_value_division = cli_opts.coalition_value_division;
        options.find_all_best_partitions = cli_opts.find_all_best_partitions;
        options.output_stats_data_file = cli_opts.output_stats_data_file;
        options.output_trace_data_file = cli_opts.output_trace_data_file;
        options.service_delay_tolerance = cli_opts.service_delay_tolerance;
        options.sim_ci_level = cli_opts.sim_ci_level;
        options.sim_ci_rel_precision = cli_opts.sim_ci_rel_precision;
        options.sim_max_num_replications = cli_opts.sim_max_num_replications;
        options.sim_max_replication_duration = cli_opts.sim_max_replication_duration;
        options.verbosity = cli_opts.verbosity;

        //std::default_random_engine rng(cli_opts.rng_seed);
        fgt::random_number_engine_t rng(cli_opts.rng_seed);

        // Run the experiment
        DCS_DEBUG_TRACE("Run the experiment...");//XXX
//FIXME
//options.coalition_formation_trigger_interval = 4*60; // Coalition formation every 4h
        detail::run_experiment(scenario, options, rng);
    }
    catch (const std::invalid_argument& ia)
    {
        dcs::log_error(DCS_LOGGING_AT, ia.what());
        detail::usage(argv[0]);
        return 1;
    }
    catch (const std::exception& e)
    {
        dcs::log_error(DCS_LOGGING_AT, e.what());
        return 1;
    }
}
