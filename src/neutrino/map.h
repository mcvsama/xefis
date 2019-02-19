/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef NEUTRINO__MAP_H__INCLUDED
#define NEUTRINO__MAP_H__INCLUDED

// Standard:
#include <cstddef>

// Neutrino:
#include <neutrino/types.h>


namespace neutrino {
namespace detail {

template<class Arguments, class Value, std::size_t N, std::size_t I>
	struct MultiDimensionalMapH
	{
		using value	= typename MultiDimensionalMapH<Arguments, Value, N, I + 1>::type;
		using type	= std::map<std::remove_cvref_t<std::tuple_element_t<I - 1, Arguments>>, value>;
	};


template<class Arguments, class Value, std::size_t N>
	struct MultiDimensionalMapH<Arguments, Value, N, N>
	{
		using type = std::map<std::remove_cvref_t<std::tuple_element_t<N - 1, Arguments>>, Value>;
	};


template<class Map, bool mapped_type_is_map>
	struct RecursiveMapKeysTupleH
	{ };


template<class Map>
	struct RecursiveMapKeysTupleH<Map, false>
	{
		using tuple = std::tuple<typename Map::key_type>;
	};


template<class Map>
	struct RecursiveMapKeysTupleH<Map, true>
	{
	  private:
		using tuple1 = std::tuple<typename Map::key_type>;
		using tuple2 = typename RecursiveMapKeysTupleH<typename Map::mapped_type, is_specialization_v<typename Map::mapped_type::mapped_type, std::map>>::tuple;

	  public:
		using tuple = TupleCat<tuple1, tuple2>;
	};

} // namespace detail


/**
 * \param	Arguments
 *			Tuple of keys.
 * \param	Value
 *			Mapped type.
 */
template<class Arguments, class Value>
	using MultiDimensionalMap = typename detail::MultiDimensionalMapH<Arguments, Value, std::tuple_size_v<Arguments>, 1>::type;


/**
 * For std::map<A, std::map<B, std::map<C, V>>> return std::tuple<A, B, C>.
 */
template<class Map>
	using RecursiveMapKeysTuple = typename detail::RecursiveMapKeysTupleH<Map, is_specialization_v<typename Map::mapped_type, std::map>>::tuple;


} // namespace neutrino

#endif

