/* vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */

/**
 * \file dcs/fgt/nash_stable_coalition_analyzer.hpp
 *
 * \brief Formation of Nash-stable coalitions.
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

#ifndef DCS_FGT_COALITION_FORMATION_NASH_STABLE_HPP
#define DCS_FGT_COALITION_FORMATION_NASH_STABLE_HPP


#include <cstddef>
#include <dcs/algorithm/combinatorics.hpp>
#include <dcs/assert.hpp>
#include <dcs/fgt/coalition_formation/commons.hpp>
#include <dcs/debug.hpp>
#include <dcs/exception.hpp>
#include <gtpack/cooperative.hpp>
#include <dcs/math/traits/float.hpp>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <vector>


namespace dcs { namespace fgt {

template <typename RealT>
struct nash_stable_partition_selector_t
{
#if 0
	std::vector<partition_info_t<RealT>> operator()(const gtpack::cooperative_game<RealT>& game, const std::map<gtpack::cid_type,coalition_info_t<RealT>>& visited_coalitions)
	{
		namespace alg = dcs::algorithm;
		namespace gt = gtpack;

		// Generate all partitions and select the ones that are Nash-stable

		std::vector<partition_info_t<RealT>> best_partitions;

		auto const players = game.players();
		auto const np = players.size();

		alg::lexicographic_partition partition(np);

		while (partition.has_next())
		{
			// Each subset is a collection of coalitions
			auto subsets = alg::next_partition(players.begin(), players.end(), partition);

			DCS_DEBUG_TRACE("--- PARTITION: " << partition);//XXX

			partition_info_t<RealT> candidate_partition;

			for (auto const& subset : subsets)
			{
				const gt::cid_type cid = gt::make_coalition_id(subset.begin(), subset.end());

				if (visited_coalitions.count(cid) == 0)
				{
					continue;
				}

				DCS_DEBUG_TRACE("--- COALITION: " << game.coalition(cid) << " (CID=" << cid << ")");//XXX

				candidate_partition.coalitions.insert(cid);

				for (auto pid : subset)
				{
					if (visited_coalitions.at(cid).payoffs.count(pid) > 0)
					{
						candidate_partition.payoffs[pid] = visited_coalitions.at(cid).payoffs.at(pid);
					}
					else
					{
						candidate_partition.payoffs[pid] = std::numeric_limits<RealT>::quiet_NaN();
					}
				}
			}

			// Check Nash-stability

			bool nash_stable = true;

			// For all players $p$
			for (std::size_t p = 0; p < np && nash_stable; ++p)
			{
				const gt::player_type pid = players[p];

				// For all $S_k$ \in \Pi \cup \{\emptyset\}$
				bool found_singleton = false;
				for (auto const& subset : subsets)
				{
					std::set<gt::pid_type> coal_players(subset.begin(), subset.end());

					if (coal_players.count(pid) == 0)
					{
						// This coalition doesn't include player pid, go on

						// Evaluate $S_k \cup {p}
						coal_players.insert(pid);

						const gt::cid_type cid = gt::make_coalition_id(coal_players.begin(), coal_players.end());

						DCS_DEBUG_TRACE("--- PID: " << pid << " - AUGMENTED COALITION: " << game.coalition(cid) << " (CID=" << cid << ") - AUGMENTED PAYOFF: " << (visited_coalitions.at(cid).payoffs.count(pid) ? visited_coalitions.at(cid).payoffs.at(pid) : std::numeric_limits<RealT>::quiet_NaN()) << " - CANDIDATE PAYOFF: " << candidate_partition.payoffs.at(pid));///XXX

						// Check player's preference
						if (visited_coalitions.at(cid).payoffs.count(pid) == 0
							|| dcs::math::float_traits<RealT>::definitely_greater(visited_coalitions.at(cid).payoffs.at(pid), candidate_partition.payoffs.at(pid)))
						{
							DCS_DEBUG_TRACE("--- PID: " << pid << " - AUGMENTED COALITION: " << game.coalition(cid) << " (CID=" << cid << "): NOT NASH STABLE");//XXX
							nash_stable = false;
							break;
						}
					}
					else if (coal_players.size() == 1)
					{
						found_singleton = true;
					}

					if (!nash_stable)
					{
						break;
					}
				}

				// Check singleton coalition
				if (!found_singleton)
				{
					const gt::cid_type cid = gt::make_coalition_id(players[p]);

					//DCS_DEBUG_TRACE("--- PID: " << pid << " - AUGMENTED COALITION: " << game.coalition(cid) << " (CID=" << cid << ")");//XXX
					DCS_DEBUG_TRACE("--- PID: " << pid << " - AUGMENTED COALITION: " << game.coalition(cid) << " (CID=" << cid << ") - AUGMENTED PAYOFF: " << (visited_coalitions.count(cid) && visited_coalitions.at(cid).payoffs.count(pid) ? visited_coalitions.at(cid).payoffs.at(pid) : std::numeric_limits<RealT>::quiet_NaN()) << " - CANDIDATE PAYOFF: " << candidate_partition.payoffs.at(pid));///XXX

					if (candidate_partition.coalitions.count(cid) == 0)
					{
						// This partition doesn't contain this singleton coalition
						if (visited_coalitions.at(cid).payoffs.count(pid) == 0
							|| dcs::math::float_traits<RealT>::definitely_greater(visited_coalitions.at(cid).payoffs.at(pid), candidate_partition.payoffs.at(pid)))
						{
							DCS_DEBUG_TRACE("--- PID: " << pid << " - AUGMENTED COALITION: " << game.coalition(cid) << " (CID=" << cid << "): NOT NASH STABLE");//XXX
							nash_stable = false;
							break;
						}
					}
				}
			}

			if (nash_stable)
			{
				best_partitions.push_back(candidate_partition);
			}
		}

		return best_partitions;
	}
#endif

	std::vector<partition_info_t<RealT>> operator()(const gtpack::cooperative_game<RealT>& game, const std::map<gtpack::cid_type,coalition_info_t<RealT>>& visited_coalitions)
	{
		namespace alg = dcs::algorithm;
		namespace gt = gtpack;

		// Generate all partitions and select the ones that are Nash-stable

		std::vector<partition_info_t<RealT>> best_partitions;

		auto const players = game.players();
		auto const np = players.size();

		alg::lexicographic_partition partition(np);

		while (partition.has_next())
		{
			// Each subset is a collection of coalitions
			auto subsets = alg::next_partition(players.begin(), players.end(), partition);

			DCS_DEBUG_TRACE("--- PARTITION: " << partition);//XXX

			partition_info_t<RealT> candidate_partition;

            candidate_partition.value = 0;
			for (auto const& subset : subsets)
			{
				const gt::cid_type cid = gt::make_coalition_id(subset.begin(), subset.end());

				if (visited_coalitions.count(cid) == 0)
				{
					continue;
				}

				DCS_DEBUG_TRACE("--- COALITION: " << game.coalition(cid) << ", VALUE: " << game.value(cid) << " (CID=" << cid << ")");//XXX

                candidate_partition.value += game.value(cid);
				candidate_partition.coalitions.insert(cid);

				for (auto pid : subset)
				{
					if (visited_coalitions.at(cid).payoffs.count(pid) > 0)
					{
						candidate_partition.payoffs[pid] = visited_coalitions.at(cid).payoffs.at(pid);
					}
					else
					{
						candidate_partition.payoffs[pid] = std::numeric_limits<RealT>::quiet_NaN();
					}
				}
			}

            bool nash_stable = check_nash_stability(game, visited_coalitions, candidate_partition.coalitions.begin(), candidate_partition.coalitions.end());
    DCS_DEBUG_TRACE("OUTSIDE NASH STABLE: " << nash_stable);

			if (nash_stable)
			{
				best_partitions.push_back(candidate_partition);
for (auto const& best_partition : best_partitions)
{
    DCS_DEBUG_TRACE("Best partition: " << best_partition.value);
}
			}
		}

		return best_partitions;
	}

    template <typename CidIterT>
    bool check_nash_stability(const gtpack::cooperative_game<RealT>& game,
                              const std::map<gtpack::cid_type,coalition_info_t<RealT>>& visited_coalitions,
                              CidIterT cid_first,
                              CidIterT cid_last)
    {
        //typedef typename std::set<gtpack::cid_type>::const_iterator partition_iterator;
        typedef typename std::set<gtpack::cid_type>::const_iterator partition_iterator;

        //const std::set<gtpack::cid_type> partition(cid_first, cid_last);
        const std::set<gtpack::cid_type> partition(cid_first, cid_last);
        const partition_iterator part_end_it = partition.end();

        bool nash_stable = true;
        for (; cid_first != cid_last && nash_stable; ++cid_first)
        {
            const gtpack::cid_type cid1 = *cid_first;

            DCS_DEBUG_TRACE("Evaluating COALITION: " << game.coalition(cid1) << " (CID: " << cid1 << ") - VALUE: " << game.value(cid1)); //XXX

            const std::vector<gtpack::pid_type> players = game.coalition(cid1).players();
            const std::size_t np = players.size();

            /**
             * Check Nash-stability:
             *   S_{\Pi}(i) \succeq_i S_k \cup \{i\}\f$ for all \f$S_k \in \Pi \cup \{\emptyset\}\f$
             */
            for (std::size_t i = 0; i < np && nash_stable; ++i)
            {
                const gtpack::pid_type pid = players[i];

                DCS_DEBUG_TRACE("Evaluating PID: " << pid << " - PAYOFF: " << (visited_coalitions.at(cid1).payoffs.count(pid) ? visited_coalitions.at(cid1).payoffs.at(pid) : std::numeric_limits<RealT>::quiet_NaN())); //XXX

                // Check Nash-stability for current player over all coalitions' partition (\f$S_k in \Pi\f$)
                for (partition_iterator part_it = partition.begin();
                     part_it != part_end_it;
                     ++part_it)
                {
                    gtpack::cid_type cid2 = *part_it;

                    DCS_DEBUG_TRACE("Evaluating ALTERNATIVE COALITION: " << game.coalition(cid2) << " (CID: " << cid2 << ") - VALUE: " << game.value(cid2)); //XXX
                    if (cid1 == cid2)
                    {
                        continue;
                    }

                    // Add the current player i to current coalition S_k, that is: S_k \cup \{i\}
                    std::vector<gtpack::pid_type> cid2_players = game.coalition(cid2).players();
                    cid2_players.push_back(pid);
                    cid2 = gtpack::make_coalition_id(cid2_players.begin(), cid2_players.end());

                    DCS_DEBUG_TRACE("--- PID: " << pid << " - AUGMENTED COALITION: " << game.coalition(cid2) << " (CID=" << cid2 << ") - AUGMENTED PAYOFF: " << (visited_coalitions.at(cid2).payoffs.count(pid) ? visited_coalitions.at(cid2).payoffs.at(pid) : std::numeric_limits<RealT>::quiet_NaN()) << " - CANDIDATE PAYOFF: " << (visited_coalitions.at(cid1).payoffs.count(pid) ? visited_coalitions.at(cid1).payoffs.at(pid) : std::numeric_limits<RealT>::quiet_NaN()));///XXX

                    // Check preference
                    if (visited_coalitions.at(cid2).payoffs.count(pid) == 0
                        || dcs::math::float_traits<RealT>::definitely_greater(visited_coalitions.at(cid2).payoffs.at(pid), visited_coalitions.at(cid1).payoffs.at(pid)))
                    {
						DCS_DEBUG_TRACE("--- PID: " << pid << " - AUGMENTED COALITION: " << game.coalition(cid2) << " (CID=" << cid2 << "): NOT NASH STABLE");//XXX
                        nash_stable = false;
                        break;
                    }
                }

                // Check Nash-stability for current player over singleton coalition (\f$S_k in \{\emptyset\}\f$)
                if (nash_stable)
                {
                    const gtpack::cid_type cid2 = gtpack::make_coalition_id(pid);

                    DCS_DEBUG_TRACE("Evaluating ALTERNATIVE COALITION: <EMPTY>"); //XXX
                    DCS_DEBUG_TRACE("--- PID: " << pid << " - AUGMENTED COALITION: " << game.coalition(cid2) << " (CID=" << cid2 << ") - AUGMENTED PAYOFF: " << (visited_coalitions.at(cid2).payoffs.count(pid) ? visited_coalitions.at(cid2).payoffs.at(pid) : std::numeric_limits<RealT>::quiet_NaN()) << " - CANDIDATE PAYOFF: " << (visited_coalitions.at(cid1).payoffs.count(pid) ? visited_coalitions.at(cid1).payoffs.at(pid) : std::numeric_limits<RealT>::quiet_NaN()));///XXX

                    // This partition doesn't contain this singleton coalition
                    if (visited_coalitions.at(cid2).payoffs.count(pid) == 0
                        || dcs::math::float_traits<RealT>::definitely_greater(visited_coalitions.at(cid2).payoffs.at(pid), visited_coalitions.at(cid1).payoffs.at(pid)))
                    {
                        DCS_DEBUG_TRACE("--- PID: " << pid << " - AUGMENTED COALITION: " << game.coalition(cid2) << " (CID=" << cid2 << "): NOT NASH STABLE");//XXX
                        nash_stable = false;
                        break;
                    }
                }
            }
        }

DCS_DEBUG_TRACE("INSIDE NASH STABLE: " << nash_stable);
        return nash_stable;
    }

}; // nash_stable_partition_selector_t

}} // Namespace dcs::fgt


#endif // DCS_FGT_COALITION_FORMATION_NASH_STABLE_HPP
