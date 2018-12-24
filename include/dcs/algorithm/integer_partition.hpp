/**
 * \file dcs/algorithm/integer_partition.hpp
 *
 * \brief Integer partitions
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 *
 * <hr/>
 *
 * Copyright 2014 Marco Guazzone (marco.guazzone@gmail.com)
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

#ifndef DCS_ALGORITHM_INTEGER_PARTITION_HPP
#define DCS_ALGORITHM_INTEGER_PARTITION_HPP

namespace dcs { namespace algorithm {

template <typename UIntT=unsigned int>
class integer_partition
{
	public: typedef UIntT uint_type;


	public: integer_partition(uint_type n)
	: n_(n)
	{
	}


	private: uint_type n_;
	private: ::std::vector<uint_type> k_;
	private: ::std::vector<uint_type> m_;
}; // integer_partition

}} // Namespace dcs::algorithm

#endif // DCS_ALGORITHM_INTEGER_PARTITION_HPP
