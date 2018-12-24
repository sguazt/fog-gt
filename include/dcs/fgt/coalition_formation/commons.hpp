/* vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */

/**
 * \file dcs/fgt/coalition_formation.hpp
 *
 * \brief Formation of coalitions of fog nodes.
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

#ifndef DCS_FGT_COALITION_FORMATION_COMMONS_HPP
#define DCS_FGT_COALITION_FORMATION_COMMONS_HPP


#include <dcs/fgt/vm_allocation.hpp>
#include <gtpack/cooperative.hpp>
#include <limits>
#include <map>
#include <set>
#include <vector>


namespace dcs { namespace fgt {

enum coalition_formation_category
{
	nash_stable_coalition_formation
};

enum coalition_value_division_category
{
	shapley_coalition_value_division
};

template <typename RealT>
struct coalition_info_t
{
	coalition_info_t()
	: //fnid_to_idx(),
	  value(std::numeric_limits<RealT>::quiet_NaN()),
	  core_empty(true),
	  payoffs(),
	  payoffs_in_core(false),
	  cid(gtpack::empty_cid)
	{
	}

	//std::map<fnid_type,std::size_t> fnid_to_idx;
	//std::map< std::size_t,std::size_t> usr_to_idx;
	//std::vector< std::size_t > usr_to_provs;
	vm_allocation_t<RealT> vm_allocation;
	RealT value;
	bool core_empty;
	std::map<gtpack::pid_type, RealT> payoffs;
	bool payoffs_in_core;
	gtpack::cid_type cid;
};

template <typename RealT>
struct partition_info_t
{
	partition_info_t()
	: value(-std::numeric_limits<RealT>::infinity())
	{
	}

	RealT value;
	std::set<gtpack::cid_type> coalitions;
	std::map<gtpack::pid_type, RealT> payoffs;
	std::map<gtpack::pid_type, RealT> coalition_change_penalties;
};

template <typename RealT>
struct coalition_formation_info_t
{
	std::map<gtpack::cid_type, coalition_info_t<RealT>> coalitions;
	std::vector<partition_info_t<RealT>> best_partitions;
};

}} // Namespace dcs::fgt


#endif // DCS_FGT_COALITION_FORMATION_COMMONS_HPP
