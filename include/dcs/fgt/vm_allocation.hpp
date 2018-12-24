/* vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */

/**
 * \file dcs/fgt/vm_allocation.hpp
 *
 * \brief Solution to the VMs allocation problem.
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

#ifndef DCS_FGT_VM_ALLOCATION_HPP
#define DCS_FGT_VM_ALLOCATION_HPP


#include <limits>
#include <vector>


namespace dcs { namespace fgt {

template <typename RealT>
struct vm_allocation_t
{
	vm_allocation_t()
	: solved(false),
	  optimal(false),
	  objective_value(std::numeric_limits<RealT>::quiet_NaN())
	{
	}


	bool solved;
	bool optimal;
	RealT objective_value;
	std::vector<std::vector<bool>> fn_vm_allocations;
	std::vector<bool> fn_power_states;
}; // vm_allocation_t

}} // Namespace dcs::fgt


#endif // DCS_FGT_VM_ALLOCATION_HPP
