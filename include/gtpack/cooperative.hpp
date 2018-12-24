/**
 * \file gtpack/cooperative.hpp
 *
 * \brief Cooperative games.
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

#ifndef GTPACK_COOPERATIVE_HPP
#define GTPACK_COOPERATIVE_HPP


#include <boost/dynamic_bitset.hpp>
#include <boost/math/special_functions/factorials.hpp>
#include <boost/smart_ptr.hpp>
#include <cstddef>
#include <dcs/algorithm/combinatorics.hpp>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
#include <dcs/exception.hpp>
#include <dcs/logging.hpp>
#include <dcs/math/traits/float.hpp>
#include <iostream>
#include <ilconcert/iloalg.h>
#include <ilconcert/iloenv.h>
#include <ilconcert/iloexpression.h>
#include <ilconcert/ilomodel.h>
#include <ilcplex/ilocplex.h>
#include <limits>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>


namespace gtpack {

typedef unsigned int pid_type;
typedef pid_type player_type; //XXX: DEPRECATED
typedef unsigned long cid_type;

static const cid_type empty_cid = 0;
static const cid_type empty_coalition_id = empty_cid; //XXX: DEPRECATED

/// Returns the identifier of the singleton coalition for the given player
inline
cid_type make_coalition_id(pid_type pid)
{
	const cid_type one = 1;

	return one << pid;
}

/// Returns the identifier of the coalition for the given players
template <typename IterT>
cid_type make_coalition_id(IterT first_player, IterT last_player)
{
	// Each coalition $S\subseteq\{1,2,...,N\}$ can be characterized
	// uniquely by the $\sum_{i \in S}{2^{i-1}}$, which is the sum of
	// integers associated to players belonging to a particular coalition.

	const cid_type one = 1;

	cid_type cid = 0;

	while (first_player != last_player)
	{
		const pid_type pid = *first_player;

		// Update only if the player in not already member of this coalition
		if (!(cid & (one << pid)))
		{
			cid += one << pid;
		}

		++first_player;
	}

	return cid;
}

/// Returns the identifier of the grand coalition for \a n players
inline
cid_type make_grand_coalition_id(std::size_t n)
{
	if (n == 0)
	{
		return empty_cid;
	}

	const std::size_t nb = sizeof(cid_type)*8; // # bits in cid_type
	return ~static_cast<cid_type>(0) >> (nb-n);
}

/// Returns the identifier of the complement of a given coalition
inline
cid_type make_complement_coalition_id(std::size_t n, cid_type cid)
{
    return make_grand_coalition_id(n) - cid;
}

/// Returns the identifier of the complement of a coalition for the given players
template <typename IterT>
cid_type make_complement_coalition_id(std::size_t n, IterT first_player, IterT last_player)
{
	return make_complement_coalition_id(n, make_coalition_id(first_player, last_player));
}

template <typename RealT>
struct characteristic_function
{
	typedef RealT real_type;

	public: real_type operator()(cid_type cid) const
	{
		if (cid == empty_coalition_id)
		{
			return 0;
		}

		return this->do_get(cid);
	}

	public: void operator()(cid_type cid, real_type v)
	{
		DCS_ASSERT(cid != empty_coalition_id,
				   DCS_EXCEPTION_THROW(::std::invalid_argument,
									   "Cannot change the value of the empty coalition"));

		this->do_set(cid, v);
	}

//	public: real_type& operator()(cid_type cid)
//	{
//		return this->do_get(cid);
//	}

	private: virtual real_type do_get(cid_type cid) const = 0;

	private: virtual void do_set(cid_type cid, real_type v) = 0;

//	private: virtual real_type& do_get(cid_type cid) = 0;
}; // characteristic_function

template <typename RealT>
class enumerated_characteristic_function: public characteristic_function<RealT>
{
	public: typedef RealT real_type;


	public: enumerated_characteristic_function()
	{
	}

	public: template <typename CidValIterT>
			enumerated_characteristic_function(CidValIterT first, CidValIterT last)
	: map_(first, last)
	{
	}

	private: real_type do_get(cid_type cid) const
	{
		return map_.count(cid) > 0 ? map_.at(cid) : ::std::numeric_limits<real_type>::quiet_NaN();
	}

	private: void do_set(cid_type cid, real_type v)
	{
		map_[cid] = v;
	}

//	private: real_type& do_get(cid_type cid)
//	{
//		return map_[cid];
//	}


	private: ::std::map<cid_type,real_type> map_;
}; // enumerated_characteristic_function


template <typename RealT>
class explicit_characteristic_function: public enumerated_characteristic_function<RealT> { };
/*
{
	public: typedef RealT real_type;


	public: explicit_characteristic_function()
	{
	}

	public: template <typename CidValIterT>
			explicit_characteristic_function(CidValIterT first, CidValIterT last)
	: map_(first, last)
	{
	}

	private: real_type do_get(cid_type cid) const
	{
		return map_.count(cid) > 0 ? map_.at(cid) : ::std::numeric_limits<real_type>::quiet_NaN();
	}

	private: void do_set(cid_type cid, real_type v)
	{
		map_[cid] = v;
	}

//	private: real_type& do_get(cid_type cid)
//	{
//		return map_[cid];
//	}


	private: ::std::map<cid_type,real_type> map_;
}; // explicit_characteristic_function
*/


template <typename RealT>
class players_coalition
{
	public: typedef unsigned long cid_type;
	public: typedef RealT real_type;
	private: typedef ::boost::dynamic_bitset<> bitset_container;
	private: typedef typename bitset_container::size_type size_type;


//	template <typename C, typename CT, typename R>
//	friend
//	::std::basic_ostream<C,CT>& operator<<(::std::basic_ostream<C,CT>&, players_coalition<R> const&);


	public: template <typename IterT>
			static cid_type make_id(IterT first_player, IterT last_player)
	{
		// Each coalition $S\subseteq\{1,2,...,N\}$ can be characterized
		// uniquely by the $\sum_{i \in S}{2^{i-1}}$, which is the sum of
		// integers associated to players belonging to a particular coalition.

		const cid_type one(1);

		cid_type cid(0);

		while (first_player != last_player)
		{
			cid += one << *first_player;

			++first_player;
		}

		return cid;
	}


	public: players_coalition(::std::size_t n, cid_type cid)
	: id_(cid),
	  players_(),
	  //players_bs_(n, id_),
	  players_bs_(::std::max(num_bits(id_), n), id_),
	  n_(players_bs_.count()),
	  v_(0)
	{
		for (size_type player = players_bs_.find_first();
			 player != bitset_container::npos;
			 player = players_bs_.find_next(player))
		{
			players_.push_back(player);
		}
	}

//	public: template <typename IterT>
//			players_coalition(::std::size_t n, IterT first_player, IterT last_player)
//	: id_(make_id(first_player, last_player)),
//	  players_(n, id_),
//	  n_(players_.count()),
//	  v_(0)
//	{
//	}

	public: template <typename IterT>
			players_coalition(IterT first_player, IterT last_player)
	: id_(make_id(first_player, last_player)),
	  players_(first_player, last_player),
	  players_bs_(players_.size(), id_),
	  n_(players_.size()),
	  v_(0)
	{
	}

	public: cid_type id() const
	{
		return id_;
	}

	public: ::std::size_t size() const
	{
		return n_;
	}

	public: bool has_player(pid_type player) const
	{
		return players_bs_.test(player);
	}

	public: ::std::vector<pid_type> players() const
	{
		return players_;
	}

	public: ::std::size_t num_players() const
	{
		return players_.size();
	}

	public: void value(real_type v)
	{
		v_ = v;
	}

	public: real_type value() const
	{
		return v_;
	}


//	private: static unsigned int count_bit_set(unsigned long v)
//	{
//		unsigned int c(0); // c accumulates the total bits set in v
//		while (v)
//		{
//			v &= v-1; // Clear the least significant bit set
//			++c;
//		}
//
//		return c;
//	}

	private: static ::std::size_t num_bits(unsigned long v)
	{
		return ::std::floor(::std::log(v)/::std::log(2)+1);
	}

	//private: ::std::bitset<8*sizeof(cid_type)> players_;
	private: cid_type id_;
	private: ::std::vector<pid_type> players_;
	private: bitset_container players_bs_;
	private: ::std::size_t n_;
	private: RealT v_;
}; // players_coalition


template <typename CharT,
		  typename CharTraitsT,
		  typename RealT>
::std::basic_ostream<CharT,CharTraitsT>& operator<<(::std::basic_ostream<CharT,CharTraitsT>& os, players_coalition<RealT> const& coalition)
{
	typedef ::std::vector<pid_type> player_container;
	typedef typename player_container::const_iterator player_iterator;

	bool first(true);

	os << "{";
	player_container players(coalition.players());
	player_iterator end_it(players.end());
	for (player_iterator it = players.begin(); it != end_it; ++it)
	{
		if (!first)
		{
			os << ",";
		}
		else
		{
			first = false;
		}
		os << *it;
	}
	os << "}";

	return os;
}


template <typename RealT>
class cooperative_game
{
	public: typedef RealT real_type;


	public: cooperative_game()
	{
	}

	public: cooperative_game(::std::size_t n,
							 ::boost::shared_ptr< characteristic_function<RealT> > const& p_v)
	: n_(n),
	  p_v_(p_v)
	{
		for (pid_type i = 0; i < n_; ++i)
		{
			players_.insert(i);
		}

		coal_struc_.insert(this->coalition(players_.begin(), players_.end()).id());
	}

	public: template <typename IterT>
			cooperative_game(IterT first_player,
							 IterT last_player,
							 ::boost::shared_ptr< characteristic_function<RealT> > const& p_v)
	: n_(0),
	  p_v_(p_v),
	  players_(first_player,last_player)
	{
		n_ = players_.size();

		coal_struc_.insert(this->coalition(players_.begin(), players_.end()).id());
	}

	public: ::std::size_t num_players() const
	{
		return n_;
	}

	public: ::std::vector<pid_type> players() const
	{
		return ::std::vector<pid_type>(players_.begin(), players_.end());
	}

	public: real_type value(cid_type cid) const
	{
		return (*p_v_)(cid);
	}

	public: void value(cid_type cid, real_type value)
	{
		(*p_v_)(cid, value);
	}

	public: players_coalition<real_type> coalition(cid_type cid) const
	{
		players_coalition<real_type> c(n_, cid);

		c.value(this->value(cid));

		return c;
	}

	public: template <typename PlayerIterT>
			players_coalition<real_type> coalition(PlayerIterT first, PlayerIterT last) const
	{
		players_coalition<real_type> c(first, last);

		c.value(this->value(c.id()));

		return c;
	}

	public: template <typename CidIterT>
			void coalition_structure(CidIterT first, CidIterT last)
	{
		coal_struc_.clear();
		while (first != last)
		{
			const cid_type cid(*first);

			coal_struc_.insert(cid);

			++first;
		}
	}

//	public: ::std::vector<cid_type> coalition_structure() const
//	{
//		return ::std::vector<cid_type>(coal_struc_.begin(), coal_struc_.end());
//	}
	public: ::std::set<cid_type> coalition_structure() const
	{
		return coal_struc_;
	}

	public: template <typename PlayerIterT>
			cooperative_game<real_type> subgame(PlayerIterT first, PlayerIterT last) const
	{
		return cooperative_game<real_type>(first, last, p_v_);
	}

	public: cooperative_game<real_type> subgame(cid_type cid) const
	{
		const ::std::vector<pid_type> players = this->coalition(n_, cid).players();

		return this->subgame(players.begin(), players.end());
	}


	private: ::std::size_t n_; ///< Number of players
	//private: ::boost::function<real_type (cid_type)> p_v_; ///< Pointer to the characteristic function
	private: ::boost::shared_ptr< characteristic_function<real_type> > p_v_; ///< Pointer to the characteristic function
	private: ::std::set<pid_type> players_; ///< The set of players
	private: ::std::set<cid_type> coal_struc_; ///< The set of coalition ID making the coalition structure for this game
}; // cooperative_game


template <typename RealT>
class core
{
	public: typedef RealT real_type;


	public: core(/*::boost::shared_ptr< cooperative_game<real_type> > const& p_game*/)
	: empty_(true)
	{
	}

	public: template <typename IterT>
			core(IterT first_payoff, IterT last_payoff)
	: empty_(false),
	  x_(first_payoff, last_payoff)
	{
	}

	public: ::std::vector<real_type> imputation() const
	{
		return x_;
	}

	public: bool empty() const
	{
		return empty_;
	}


	private: bool empty_;
	private: ::std::vector<real_type> x_;
};


/**
 * \brief Compute the Shapley value for a given player.
 *
 * Given a game \f$(N,v)\f$, the Shapley value \f$\phi_i\f$ for player \f$i\f$
 * is computed as:
 * \f[
 *  \phi_i(v)=\sum_{S \subseteq N \setminus \{i\}} \frac{|S|!\; (|N|-|S|-1)!}{|N|!}(v(S\cup\{i\})-v(S)) 
 * \f]
 */
template <typename RealT>
RealT shapley_value(cooperative_game<RealT> const& game, pid_type player)
{
	const ::std::size_t n(game.num_players());
	const RealT n_fact(::boost::math::factorial<RealT>(n));

	::std::vector<pid_type> players(game.players());
	::std::set<pid_type> other_players(players.begin(), players.end());
	players.clear();
	other_players.erase(player);

	RealT sv(0);

	if (other_players.size() > 0)
	{
		::dcs::algorithm::lexicographic_subset subset(other_players.size(), true);
		while (subset.has_next())
		{
			::std::size_t s(subset.size());
			RealT s_fact(::boost::math::factorial<RealT>(s));
			RealT nmsm1_fact(::boost::math::factorial<RealT>(n-s-1));

			::std::vector<pid_type> tmp_players(subset(other_players.begin(), other_players.end()));
			cid_type s_cid = players_coalition<RealT>::make_id(tmp_players.begin(), tmp_players.end());
			tmp_players.push_back(player);
			cid_type sui_cid = players_coalition<RealT>::make_id(tmp_players.begin(), tmp_players.end());

			sv += s_fact*nmsm1_fact*(game.value(sui_cid)-game.value(s_cid));

			++subset;
		}
	}
	else
	{
		cid_type cid = players_coalition<RealT>::make_id(&player, &player+1);
		sv += game.value(cid);
	}

	return sv/n_fact;
}

/**
 * \brief Compute the Shapley value for all the players of a given game.
 *
 * Given a game \f$(N,v)\f$, the Shapley value \f$\phi_i\f$ for player \f$i\f$
 * is computed as:
 * \f[
 *  \phi_i(v)=\sum_{S \subseteq N \setminus \{i\}} \frac{|S|!\; (|N|-|S|-1)!}{|N|!}(v(S\cup\{i\})-v(S)) 
 * \f]
 */
template <typename RealT>
::std::map<pid_type,RealT> shapley_value(cooperative_game<RealT> const& game)
{
	const ::std::size_t n(game.num_players());
	const RealT n_fact(::boost::math::factorial<RealT>(n));

	::std::vector<pid_type> players(game.players());

	::std::map<pid_type,RealT> sv_map;

	::std::vector<pid_type>::const_iterator players_end_it(players.end());
	for (::std::vector<pid_type>::const_iterator players_it = players.begin();
		 players_it != players_end_it;
		 ++players_it)
	{
		pid_type pid(*players_it);

		RealT sv(0);

		::std::set<pid_type> other_players(players.begin(), players.end());
		other_players.erase(pid);

		if (other_players.size() > 0)
		{
			::dcs::algorithm::lexicographic_subset subset(other_players.size(), true);
			while (subset.has_next())
			{
				const ::std::size_t s(subset.size());
				const RealT s_fact(::boost::math::factorial<RealT>(s));
				const RealT nmsm1_fact(::boost::math::factorial<RealT>(n-s-1));

				::std::vector<pid_type> tmp_players(subset(other_players.begin(), other_players.end()));
				cid_type s_cid = players_coalition<RealT>::make_id(tmp_players.begin(), tmp_players.end());
				tmp_players.push_back(pid);
				const cid_type sui_cid = players_coalition<RealT>::make_id(tmp_players.begin(), tmp_players.end());

				sv += s_fact*nmsm1_fact*(game.value(sui_cid)-game.value(s_cid));

				++subset;
			}
		}
		else
		{
			cid_type cid = players_coalition<RealT>::make_id(&pid, &pid+1);
			sv += game.value(cid);
		}

		sv_map[pid] = sv/n_fact;
	}

	return sv_map;
}

/**
 * \brief Compute the Banzhaf value for all the players of a given game.
 *
 * Given a game \f$(N,v)\f$, the Banzhaf value \f$\beta_i\f$ for player \f$i\f$
 * is computed as:
 * \f[
 *  \beta_i(v)=\frac{1}{2^{|N|-1}}\sum_{S \subseteq N \setminus \{i\}} (v(S\cup\{i\})-v(S)) 
 * \f]
 */
template <typename RealT>
::std::map<pid_type,RealT> banzhaf_value(cooperative_game<RealT> const& game)
{
	const ::std::size_t n(game.num_players());
	const RealT prod(1.0/::std::pow(2, n-1));
	//const RealT prod(1.0/(1 << (n-1)));

	::std::vector<pid_type> players(game.players());

	::std::map<pid_type,RealT> bv_map;

	::std::vector<pid_type>::const_iterator players_end_it(players.end());
	for (::std::vector<pid_type>::const_iterator players_it = players.begin();
		 players_it != players_end_it;
		 ++players_it)
	{
		pid_type player(*players_it);

		RealT bv(0);

		::std::set<pid_type> other_players(players.begin(), players.end());
		other_players.erase(player);

		if (other_players.size() > 0)
		{
			::dcs::algorithm::lexicographic_subset subset(other_players.size(), true);
			while (subset.has_next())
			{
				::std::vector<pid_type> tmp_players(subset(other_players.begin(), other_players.end()));
				cid_type s_cid = players_coalition<RealT>::make_id(tmp_players.begin(), tmp_players.end());
				tmp_players.push_back(player);
				cid_type sui_cid = players_coalition<RealT>::make_id(tmp_players.begin(), tmp_players.end());

				bv += (game.value(sui_cid)-game.value(s_cid));

				++subset;
			}
		}
		else
		{
			cid_type cid = players_coalition<RealT>::make_id(&player, &player+1);
			bv += game.value(cid);
		}

		bv_map[player] = bv*prod;
	}

	return bv_map;
}

/**
 * \brief Compute the normalized Banzhaf value for all the players of a given
 *  game.
 *
 * Given a game \f$(N,v)\f$, the normalized Banzhaf value \f$\bar{\beta}_i\f$
 * for player \f$i\f$ is computed as:
 * \f[
 *  \bar{\beta}_i=\frac{\beta_i}{\sum_{j \in N}\beta_j}
 * \f]
 * where
 * \f[
 *  \beta_i(v)=\frac{1}{2^{|N|-1}}\sum_{S \subseteq N \setminus \{i\}} (v(S\cup\{i\})-v(S)) 
 * \f]
 * is the non-normalized Banzhaf value.
 */
template <typename RealT>
::std::map<pid_type,RealT> norm_banzhaf_value(cooperative_game<RealT> const& game)
{
	::std::map<pid_type,RealT> bv_map = banzhaf_value(game);

	RealT den(0);
	// Compute the sum of all non-normalized Banzhaf values
	{
		typename ::std::map<pid_type,RealT>::const_iterator end_it(bv_map.end());
		for (typename ::std::map<pid_type,RealT>::const_iterator it = bv_map.begin();
			 it != end_it;
			 ++it)
		{
			den += it->second;
		}
	}
	// Normalize each Banzhaf value
	{
		::std::vector<pid_type> players(game.players());
		cid_type gc_cid = players_coalition<RealT>::make_id(players.begin(), players.end());
		RealT v_gc = game.value(gc_cid);
		typename ::std::map<pid_type,RealT>::iterator end_it(bv_map.end());
		for (typename ::std::map<pid_type,RealT>::iterator it = bv_map.begin();
			 it != end_it;
			 ++it)
		{
			it->second *= v_gc/den;
		}
//		typename ::std::map<pid_type,RealT>::iterator end_it(bv_map.end());
//		for (typename ::std::map<pid_type,RealT>::iterator it = bv_map.begin();
//			 it != end_it;
//			 ++it)
//		{
//			it->second /= den;
//		}
	}

	return bv_map;
}

/**
 * \brief Compute the Aumann-Dreze value for all the players of a given game.
 *
 * Given a game \f$(N,v)\f$ with coalition structure \f$\mathcal{P}\f$, the
 * Aumann-Dreze value \f$\text{AD}_i\f$ for player \f$i\f$ is computed as:
 * \f[
 *  \text{AD}_i(v)=\sum_{S \subseteq \mathcal{P}(i) \setminus \{i\}} \frac{|S|!\; (|\mathcal{P}(i)|-|S|-1)!}{|\mathcal{P}(i)|!}(v(S\cup\{i\})-v(S)) 
 * \f]
 * where \f$\mathcal{P}(i)\in\mathcal{P}\f$ is the coalition containing the
 * player $i$.
 *
 * References:
 * -# R.J. Aumann and J.H. Dr{\`e}ze,
 *    Cooperative games with coalition structures,
 *    International Journal of Game Theory 3:217-237, 1974
 * .
 */
template <typename RealT>
::std::map<pid_type,RealT> aumann_dreze_value(cooperative_game<RealT> const& game)
{
	const ::std::vector<cid_type> structure(game.coalition_structure());
	const ::std::size_t nc(structure.size());

	::std::map<pid_type,RealT> ad_map;

	for (::std::size_t i = 0; i < nc; ++i)
	{
		const cid_type cid(structure[i]);

		const ::std::vector<pid_type> players = game.coalition(cid).players();

		::std::map<pid_type,RealT> sh_map = shapley_value(game.subgame(cid));
		ad_map.insert(sh_map.begin(), sh_map.end());
	}

	return ad_map;
}

/**
 * \brief Compute the Chi-value for all the players of a given
 *  game.
 *
 * Given a game \f$(N,v)\f$ with coalition structure \f$\mathcal{P}\f$, the
 * Chi-value \f$\Chi_i\f$ for player \f$i\f$ is computed as:
 * \f[
 *  Chi_i=\phi_i(N,v)+\frac{v(\mathcal{P}(i)-\sum_{j\in\mathcal{P}(i)}\phi_j(N,v)}{|\mathcal{P}(i)|}
 * \f]
 * where\f$\mathcal{P}(i)\f$ is the coalition in \f$\mathcal{P}\f$ that contains
 * the player \f$i\f$.
 *
 * References:
 * -# Andre` Casajus. "Outside options, component efficiency, and stability", Games and Economic Behavior 65:49-61, 2009.
 * .
 */
template <typename RealT>
::std::map<pid_type,RealT> chi_value(cooperative_game<RealT> const& game)
{
	const ::std::vector<cid_type> structure(game.coalition_structure());
	const ::std::size_t nc(structure.size());
	const ::std::map<pid_type,RealT> shapley_map(shapley_value(game));

	::std::map< ::std::size_t,RealT> coal_shapley_map;

	for (::std::size_t i = 0; i < nc; ++i)
	{
		const cid_type cid = structure[i];
		const ::std::vector<pid_type> players = game.coalition(cid).players();
		const ::std::size_t np = players.size();

		RealT sh(0);
		for (::std::size_t j = 0; j < np; ++j)
		{
			const pid_type pid(players[j]);

			sh += shapley_map.at(pid);
		}
		coal_shapley_map[cid] = sh;
	}

	::std::map<pid_type,RealT> chi_map;

	for (::std::size_t i = 0; i < nc; ++i)
	{
		const cid_type cid = structure[i];
		const players_coalition<RealT> coalition = game.coalition(cid);
		const ::std::vector<pid_type> players = coalition.players();
		const ::std::size_t np = players.size();
		const RealT add = (coalition.value()-coal_shapley_map.at(cid))/np;

		for (::std::size_t j = 0; j < np; ++j)
		{
			const pid_type pid(players[j]);

			chi_map[pid] = shapley_map.at(pid)+add;
		}
	}

	return chi_map;
}


/**
 * \brief Compute the core of the given cooperative game.
 *
 * Given a cooperative game G=(N,v), the core C(v) is the set
 * \f[
 *  \biggl\bigl{x | \sum_{i \in N} x_i=v(N), \text{ and } x_i \ge v(i) \forall i \in N, \text{ and } \sum_{i\in S} x_i \ge v(S) \forall S \subset N\setminus \{\emptyset\bigr\}\biggr\}
 * \f]
 *
 * TODO: manage the case of games with a coalition structure CS. For such games, the core is defined as the set of payoff vectors x = (x1 , ..., xn ) and a coalition Structure CS = (C1 , ...,Ck ) such that:
 * - \forall C \subseteq N, x(C) \ge v(C) and
 * - x(C_j) = v(C_j) for any C_j \in CS
 * .
 */
template <typename RealT>
//core<RealT> find_core(typename coalition<RealT>::cid_type const& cid, ::std::vector<pid_type> const& players, ::std::map<cid_type,RealT> const& coalitions)
core<RealT> find_core(cooperative_game<RealT> const& game)
//typename coalition<RealT>::cid_type const& cid, ::std::vector<pid_type> const& players, ::std::map<cid_type,RealT> const& coalitions)
{
	typedef RealT real_type;
	typedef players_coalition<real_type> coalition_type;
	//typedef typename coalition_type::cid_type cid_type;
	typedef IloNumVarArray var_vector_type;
//	typedef IloArray<IloNumVarArray> var_matrix_type;
	typedef typename ::dcs::algorithm::lexicographic_subset subset_type;
	typedef typename subset_type::const_iterator subset_iterator;
	typedef typename ::dcs::algorithm::subset_traits<pid_type>::subset_container subset_container;
//	typedef typename ::dcs::algorithm::subset_traits<pid_type>::subset_container_const_iterator subset_container_iterator;

	core<real_type> kore;

	// Setting up vars
	try
	{
		// Initialize the Concert Technology app
		IloEnv env;

		IloModel model(env);

		model.setName("Core");

		::std::size_t n(game.num_players());
		::std::vector<pid_type> players(game.players());

		// Decision Variable

		// Variables x_{i} >= 0: the payoff for player i
		var_vector_type x(env, n);
		for (std::size_t i = 0; i < n; ++i)
		{
			std::ostringstream oss;
			oss << "x[" << i << "]";
			x[i] = IloNumVar(env, -IloInfinity, IloInfinity, ILOFLOAT, oss.str().c_str());
			model.add(x[i]);
		}

		// Constraints

		std::size_t cc(0); // Constraint counter
		std::size_t csc(0); // Constraint subcounter

		subset_type lex_subset(n, false);

		// C1: \forall S \subset N, \sum_{i \in S} x[i] >= v(S), and \sum_{i \in N} x[i] = v(N)
		++cc;
		while (lex_subset.has_next())
		{
			++csc;

			subset_container players_subset;

			//players_subset = ::dcs::algorithm::next_subset(players.begin(), players.end(), lex_subset);
			players_subset = lex_subset(players.begin(), players.end());

			cid_type cid = coalition_type::make_id(players_subset.begin(), players_subset.end());

			real_type v_cid = game.value(cid);

			std::ostringstream oss;
			oss << "C" << cc << "_{" << csc << "}";
			IloExpr lhs(env);
			subset_iterator sub_end_it(lex_subset.end());
			for (subset_iterator sub_it = lex_subset.begin();
				 sub_it != sub_end_it;
				 ++sub_it)
			{
				typename subset_iterator::value_type player_num(*sub_it);

				// check: player_num is a valid player number
				DCS_ASSERT(player_num < n,
						   DCS_EXCEPTION_THROW(::std::runtime_error, "Invalid player number"));

				lhs += x[player_num];
			}
			IloConstraint cons;
			if (players_subset.size() == n)
			{
				cons = IloConstraint(lhs == IloNum(v_cid));
			}
			else
			{
				cons = IloConstraint(lhs >= IloNum(v_cid));
			}
			cons.setName(oss.str().c_str());
			model.add(cons);

			++lex_subset;
		}


		// Set objective
		//   max z = \sum_{i=1}^N x_i
		IloObjective z;
		z = IloMaximize(env, IloSum(x));
		model.add(z);

		IloCplex solver(model);
#ifndef DCS_DEBUG
		solver.setOut(env.getNullStream());
		solver.setWarning(env.getNullStream());
#else // DCS_DEBUG
		solver.exportModel("cplex-core_model.lp");
#endif // DCS_DEBUG

		// Set Relative Gap to 1%: CPLEX will stop as soon as it has found a feasible integer solution proved to be within 1% of optimal.
		//if (relative_gap > 0)
		//{
		//	solver.setParam(IloCplex::EpGap, relative_gap);
		//}

		bool solved = solver.solve();

		IloAlgorithm::Status status = solver.getStatus();
		switch (status)
		{
			case IloAlgorithm::Optimal: // The algorithm found an optimal solution.
			case IloAlgorithm::Feasible: // The algorithm found a feasible solution, though it may not necessarily be optimal.
//
//				v = static_cast<RealT>(solver.getObjValue());
				break;
			case IloAlgorithm::Infeasible: // The algorithm proved the model infeasible (i.e., it is not possible to find an assignment of values to variables satisfying all the constraints in the model).
			case IloAlgorithm::Unbounded: // The algorithm proved the model unbounded.
			case IloAlgorithm::InfeasibleOrUnbounded: // The model is infeasible or unbounded.
			case IloAlgorithm::Error: // An error occurred and, on platforms that support exceptions, that an exception has been thrown.
			case IloAlgorithm::Unknown: // The algorithm has no information about the solution of the model.
			{
				::std::ostringstream oss;
				oss << "Optimization was stopped with status = " << status << " (CPLEX status = " << solver.getCplexStatus() << ", sub-status = " << solver.getCplexSubStatus() << ")";
				dcs::log_warn(DCS_LOGGING_AT, oss.str());
			}
		}

		if (solved)
		{
#ifdef DCS_DEBUG
			DCS_DEBUG_TRACE( "-------------------------------------------------------------------------------[" );
			DCS_DEBUG_TRACE( "- Objective value: " << static_cast<real_type>(solver.getObjValue()) );

			DCS_DEBUG_TRACE( "- Decision variables: " );

			// Output x_{i}
			for (std::size_t i = 0; i < n; ++i)
			{
				DCS_DEBUG_STREAM << x[i].getName() << " = " << solver.getValue(x[i]) << ::std::endl;
			}

			DCS_DEBUG_TRACE( "]-------------------------------------------------------------------------------" );
#endif // DCS_DEBUG

			::std::vector<real_type> payoff(n);
			for (std::size_t i = 0; i < n; ++i)
			{
				//payoff[players[i]] = solver.getValue(x[i]);
				payoff[i] = solver.getValue(x[i]);
			}
			kore = core<real_type>(payoff.begin(), payoff.end());
		}

		z.end();
		x.end();

		// Close the Concert Technology app
		env.end();
	}
	catch (IloException const& e)
	{
		::std::ostringstream oss;
		oss << "Got exception from CPLEX: " << e.getMessage();
		DCS_EXCEPTION_THROW(::std::runtime_error,
							oss.str());
	}
	catch (...)
	{
		DCS_EXCEPTION_THROW(::std::runtime_error,
							"Unexpected error during the optimization");
	}

	return kore;
}


/**
 * \brief Compute the core of the given cooperative game.
 *
 * Given a cooperative game G=(N,v), the core C(v) is the set
 * \f[
 *  \biggl\bigl{x | \sum_{i \in N} x_i=v(N), \text{ and } x_i \ge v(i) \forall i \in N, \text{ and } \sum_{i\in S} x_i \ge v(S) \forall S \subset N\setminus \{\emptyset\bigr\}\biggr\}
 * \f]
 */
template <typename RealT, typename IterT>
bool belongs_to_core(cooperative_game<RealT> const& game, IterT first_payoff, IterT last_payoff)
{
	typedef RealT real_type;
	typedef players_coalition<real_type> coalition_type;
	//typedef typename coalition_type::cid_type cid_type;
	typedef typename ::dcs::algorithm::lexicographic_subset subset_type;
	//typedef typename subset_type::const_iterator subset_iterator;
	typedef typename ::dcs::algorithm::subset_traits<pid_type>::subset_container subset_container;
	typedef typename ::dcs::algorithm::subset_traits<pid_type>::subset_container_const_iterator subset_container_iterator;

	::std::map<pid_type,real_type> x(first_payoff, last_payoff);
	::std::size_t n(game.num_players());
	::std::vector<pid_type> players(game.players());

	subset_type lex_subset(n, false);

	// \forall S \subset N, \sum_{i \in S} x[i] >= v(S), and \sum_{i \in N} x[i] = v(N)
	while (lex_subset.has_next())
	{
		subset_container players_subset;

		players_subset = lex_subset(players.begin(), players.end());

		cid_type cid = coalition_type::make_id(players_subset.begin(), players_subset.end());

		real_type v_cid = game.value(cid);
		real_type xs(0);

		subset_container_iterator sub_end_it(players_subset.end());
		for (subset_container_iterator sub_it = players_subset.begin();
			 sub_it != sub_end_it;
			 ++sub_it)
		{
			pid_type player_num(*sub_it);

			xs += x[player_num];
		}
		bool ok;
		if (lex_subset.size() == n)
		{
			//ok = xs == v_cid;
			ok = ::dcs::math::float_traits<real_type>::essentially_equal(xs, v_cid);
		}
		else
		{
			//ok = xs >= v_cid;
			ok = ::dcs::math::float_traits<real_type>::essentially_greater_equal(xs, v_cid);
		}
		if (!ok)
		{
			return false;
		}

		++lex_subset;
	}

	return true;
}

} // Namespace gtpack


#endif // GTPACK_COOPERATIVE_HPP
