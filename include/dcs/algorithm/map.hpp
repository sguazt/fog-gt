/**
 * \file dcs/algorithm/map.hpp
 *
 * \brief Algorithm for map containers.
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

#ifndef DCS_ALGORITHM_MAP_HPP
#define DCS_ALGORITHM_MAP_HPP


#include <vector>


namespace dcs { namespace algorithm {

/// Returns the values of the map defined by the [\a first, \a last) iterator pair
template <typename T, typename MapIterT>
std::vector<T> map_values(MapIterT first, MapIterT last)
{
	std::vector<T> res;

	while (first != last)
	{
		res.push_back(first->second);
		++first;
	}

	return res;
}

/// Returns the values of the map defined by the [\a map_first, \a map_last) iterator pair
template <typename T, typename MapIterT, typename KeyIterT>
std::vector<T> map_values(MapIterT map_first, MapIterT map_last, KeyIterT key_first, KeyIterT key_last)
{
	std::vector<T> res;

	MapIterT map_it = map_first;

	while (map_first != map_last)
	{
		KeyIterT key_it = key_first;

		while (key_it != key_last)
		{
			if (map_first->first == *key_it)
			{
				res.push_back(map_first->second);
				break;
			}

			++key_it;
		}

		++map_first;
	}

	return res;
}

/// Returns the values of the map \a m
template <typename MapT>
std::vector<typename MapT::mapped_type> map_values(MapT const& m)
{
	return map_values<typename MapT::mapped_type>(m.begin(), m.end());
}

/// Returns the values of the map \a m corresponding to keys defined by the [\a key_first, \a key_last] iterator pair
template <typename MapT, typename KeyIterT>
std::vector<typename MapT::mapped_type> map_values(MapT const& m, KeyIterT key_first, KeyIterT key_last)
{
	std::vector<typename MapT::mapped_type> res;

	while (key_first != key_last)
	{
		if (m.count(*key_first) > 0)
		{
			res.push_back(m.at(*key_first));
		}

		++key_first;
	}
	return res;
}

/// Returns the keys of the map defined by the [\a first, \a last) iterator pair
template <typename T, typename MapIterT>
std::vector<T> map_keys(MapIterT first, MapIterT last)
{
	std::vector<T> res;

	while (first != last)
	{
		res.push_back(first->first);
		++first;
	}

	return res;
}

/// Returns the keys of the map \c m
template <typename MapT>
std::vector<typename MapT::key_type> map_key(MapT const& m)
{
	return map_key<typename MapT::key_type>(m.begin(), m.end());
}

}} // Namespace dcs::algorithm

#endif // DCS_ALGORITHM_MAP_HPP
