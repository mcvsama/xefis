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

#ifndef NEUTRINO__MATH__FIELD_H__INCLUDED
#define NEUTRINO__MATH__FIELD_H__INCLUDED

// Standard:
#include <cstddef>
#include <initializer_list>
#include <tuple>
#include <vector>
#include <type_traits>

// Neutrino:
#include <neutrino/map.h>
#include <neutrino/numeric.h>
#include <neutrino/sequence_utils.h>
#include <neutrino/synchronized.h>
#include <neutrino/types.h>


namespace neutrino {

// TODO make class Subfield SubField sf = field.subfield (0.0, 10_deg);

template<class Argument0, class ...RemainingArgumentsAndValue>
	class Field
	{
		static_assert (sizeof...(RemainingArgumentsAndValue) >= 1, "Field<> needs at least two parameters, argument and value types");

		static constexpr bool kLinearExtrapolate = false;

	  public:
		static constexpr std::size_t kNumArguments	= sizeof...(RemainingArgumentsAndValue);

		using ArgumentsAndValue		= std::tuple<Argument0, RemainingArgumentsAndValue...>;
		using Arguments				= TupleSlice<0, std::tuple_size_v<ArgumentsAndValue> - 1, ArgumentsAndValue>;
		using Value					= std::remove_cvref_t<std::tuple_element_t<kNumArguments, ArgumentsAndValue>>;
		using DataMap				= MultiDimensionalMap<Arguments, Value>;

		/**
		 * A point in the field subspace. Arguments and value.
		 */
		template<class Tuple>
			struct SubspacePoint
			{
			  public:
				Tuple	arguments;
				Value	value;

			  public:
				explicit
				SubspacePoint (Tuple const& arguments, Value const& value):
					arguments (arguments),
					value (value)
				{ }
			};

		using Point = SubspacePoint<Arguments>;

		template<std::size_t N>
			using NthArgument =
				std::remove_cvref_t<std::tuple_element_t<N, Arguments>>;

		template<class ...Args>
			using ArgumentType =
				std::conditional_t<sizeof...(Args) == 0,
								   NthArgument<sizeof...(Args)>,
								   std::optional<NthArgument<sizeof...(Args)>>>;

		template<class ...Args>
			using ValueType =
				std::conditional_t<sizeof...(Args) == 0,
								   Value,
								   std::optional<Value>>;

		template<class ...Args>
			using PointType =
				std::conditional_t<sizeof...(Args) == 0,
								   Point,
								   std::optional<Point>>;

		template<class ...Args>
			using DomainType =
				std::conditional_t<sizeof...(Args) == 0,
								   Range<NthArgument<sizeof...(Args)>>,
								   std::optional<Range<NthArgument<sizeof...(Args)>>>>;

		template<class ...Args>
			using CodomainType =
				std::conditional_t<sizeof...(Args) == 0,
								   Range<Value>,
								   std::optional<Range<Value>>>;

		template<class Map, class ...Args>
			using MinMaxSubspacePoint =
				std::conditional_t<sizeof...(Args) == 0,
								   SubspacePoint<RecursiveMapKeysTuple<Map>>,
								   std::optional<SubspacePoint<RecursiveMapKeysTuple<Map>>>>;

		/**
		 * Thrown when coefficients map passed to the Field
		 * doesn't have any points defined.
		 */
		class EmptyDomainException: public Exception
		{
		  public:
			explicit
			EmptyDomainException():
				Exception ("field domain must not be empty")
			{ }
		};

	  public:
		/**
		 * Create a data-table from argument-value points given in the map.
		 */
		explicit
		Field (DataMap const&);

		/**
		 * Create a data-table from argument-value points given in the map.
		 */
		explicit
		Field (DataMap&&);

		/**
		 * Create a data-table from a list of key-value pairs.
		 */
		explicit
		Field (std::initializer_list<typename DataMap::value_type>&&);

		// Copy ctor
		Field (Field const&) = default;

		// Move ctor
		Field (Field&&);

		// Copy operator
		Field&
		operator= (Field const&) = default;

		// Move operator
		Field&
		operator= (Field&&);

		/**
		 * Return number of dimensions of this field.
		 */
		static constexpr std::size_t
		dimensions() noexcept;

		/**
		 * Return value for given arguments.
		 * Result will be defined only if arguments are within bounds of data-table arguments.
		 * The result will be interpolated.
		 */
		template<class ...Args>
			[[nodiscard]]
			std::optional<Value>
			value (Args&&...) const;

		/**
		 * Return value for given arguments.
		 * If arguments are outside bounds of data-table arguments, result will be extrapolated;
		 * otherwise result will be the same as from calling value().
		 */
		template<class ...Args>
			[[nodiscard]]
			Value
			extrapolated_value (Args&&...) const;

		/**
		 * Alias for extrapolated_value().
		 */
		template<class ...Args>
			[[nodiscard]]
			Value
			operator() (Args&&...) const;

		/**
		 * Return the point of minimum known argument.
		 */
		template<class ...Args>
			[[nodiscard]]
			ArgumentType<Args...>
			min_argument (Args&&...) const;

		/**
		 * Return the point of maximum known argument.
		 */
		template<class ...Args>
			[[nodiscard]]
			ArgumentType<Args...>
			max_argument (Args&&...) const;

		/**
		 * Return the point of minimum value.
		 *
		 * The less arguments are given, the more dimensional space is searched.
		 * For no argument, whole space is searched, and Value is returned. If any argument is given, returned value is std::optional<Value>, since certain
		 * arguments may be outside of domain.
		 */
		template<class ...Args>
			[[nodiscard]]
			PointType<Args...>
			min_value_point (Args&&...) const;

		/**
		 * Return the point of maximum value.
		 *
		 * Same notes as for min_value_point() apply.
		 */
		template<class ...Args>
			[[nodiscard]]
			PointType<Args...>
			max_value_point (Args&&...) const;

		/**
		 * Return the minimum value for given subspace.
		 */
		template<class ...Args>
			[[nodiscard]]
			ValueType<Args...>
			min_value (Args&&...) const;

		/**
		 * Return the minimum value for given subspace.
		 */
		template<class ...Args>
			[[nodiscard]]
			ValueType<Args...>
			max_value (Args&&...) const;

		/**
		 * Return domain (range of arguments) of the field.
		 */
		template<class ...Args>
			[[nodiscard]]
			DomainType<Args...>
			domain (Args&&...) const;

		/**
		 * Return codomain (range of values) of the field.
		 */
		template<class ...Args>
			[[nodiscard]]
			CodomainType<Args...>
			codomain (Args&&...) const;

#if 0
		/**
		 * Return a vector of arguments for given value.
		 * This is a primitive version that searches only the last dimension.
		 */
		[[nodiscard]]
		std::vector<Point>
		arguments (ArgumentsWithoutLast const&, Value const& value) const;

		/**
		 * Return a vector of arguments for given value.
		 * Search only the given domain (range is inclusive).
		 * This is a primitive version that searches only the last dimension.
		 */
		[[nodiscard]]
		std::vector<Point>
		arguments (ArgumentsWithoutLast const&, Value const& value, Range<LastArgument> search_domain) const;

		/**
		 * Compute average value of all points.
		 */
		template<class ...Args>
			[[nodiscard]]
			Value
			average() const;

		/**
		 * Compute average value for given range of arguments.
		 */
		template<class = std::enable_if_t<dimensions() == 1>>
			[[nodiscard]]
			Value
			average (Range<Argument> domain) const;
#endif

		/**
		 * Return reference to underlying map of values used by this Field.
		 */
		[[nodiscard]]
		DataMap const&
		data_map() const;

	  private:
		/**
		 * Search all submaps and throw EmptyDomainException if any of the maps
		 * is empty.
		 */
		template<class Map>
			static void
			validate (Map const&);

		/**
		 * Helper for min_argument() and max_argument().
		 */
		template<class Map, class GetIter>
			[[nodiscard]]
			typename Map::key_type
			compute_minmax_argument (Map const&, GetIter&&) const;

		/**
		 * Helper for min_argument() and max_argument().
		 */
		template<class Map, class GetIter, class X, class ...Xs>
			[[nodiscard]]
			std::optional<NthArgument<sizeof...(Xs) + 1u>>
			compute_minmax_argument (Map const&, GetIter&&, X&&, Xs&&...) const;

		/**
		 * Helper for min_argument() and max_argument().
		 */
		template<class Map, class IsBetter>
			[[nodiscard]]
			MinMaxSubspacePoint<Map>
			compute_minmax_value (Map const&, IsBetter&&) const;

		/**
		 * Helper for min_argument() and max_argument().
		 */
		template<class Map, class IsBetter, class X, class ...Xs>
			[[nodiscard]]
			MinMaxSubspacePoint<Map, X, Xs...>
			compute_minmax_value (Map const&, IsBetter&&, X&&, Xs&&...) const;

		/**
		 * Return interpolated value for given map.
		 * Argument must be within data domain for that map.
		 */
		template<class Map, class X, class ...Xs>
			[[nodiscard]]
			std::optional<Value>
			compute_value (Map&&, bool extrapolate, X&&, Xs&&...) const;

		/**
		 * Renormalization helper for all values of a tuple.
		 */
		template<std::size_t I, class V, class A, class Tuple>
			static void
			renormalize_tuple_helper (V const& v, Range<A> const& from_range, Tuple const& tuple_min, Tuple const& tuple_max, Tuple& result)
			{
				auto const to_range = Range { std::get<I> (tuple_min), std::get<I> (tuple_max) };
				std::get<I> (result) = renormalize (v, from_range, to_range);

				if constexpr (I < std::tuple_size_v<Tuple> - 1)
					renormalize_tuple_helper<I + 1> (v, from_range, tuple_min, tuple_max, result);
			}

		/**
		 * Renormalization helper for all values of a tuple.
		 */
		template<class V, class A, class Tuple>
			static Tuple
			renormalize_tuple (V const& v, Range<A> const& from_range, Tuple const& tuple_min, Tuple const& tuple_max)
			{
				Tuple result;
				renormalize_tuple_helper<0> (v, from_range, tuple_min, tuple_max, result);
				return result;
			}

	  private:
		DataMap _data_map;
	};


template<class A, class ...R>
	inline
	Field<A, R...>::Field (DataMap const& map):
		_data_map (map)
	{
		validate (_data_map);
	}


template<class A, class ...R>
	inline
	Field<A, R...>::Field (DataMap&& map):
		_data_map (std::move (map))
	{
		validate (_data_map);
	}


template<class A, class ...R>
	inline
	Field<A, R...>::Field (std::initializer_list<typename DataMap::value_type>&& pairs):
		_data_map (std::move (pairs))
	{
		validate (_data_map);
	}


template<class A, class ...R>
	inline
	Field<A, R...>::Field (Field&& other):
		_data_map (std::move (other._data_map))
	{ }


template<class A, class ...R>
	inline Field<A, R...>&
	Field<A, R...>::operator= (Field&& other)
	{
		_data_map = std::move (other._data_map);
		return *this;
	}


template<class A, class ...R>
	constexpr std::size_t
	Field<A, R...>::dimensions() noexcept
	{
		return std::tuple_size_v<Arguments>;
	}


template<class A, class ...R>
	template<class ...Args>
		inline auto
		Field<A, R...>::value (Args&& ...args) const
			-> std::optional<Value>
		{
			static_assert (sizeof...(args) == kNumArguments, "not enough arguments");

			return compute_value (_data_map, false, std::forward<Args> (args)...);
		}


template<class A, class ...R>
	template<class ...Args>
		inline auto
		Field<A, R...>::extrapolated_value (Args&& ...args) const
			-> Value
		{
			static_assert (sizeof...(args) == kNumArguments, "not enough arguments");

			return *compute_value (_data_map, true, std::forward<Args> (args)...);
		}


template<class A, class ...R>
	template<class ...Args>
		inline auto
		Field<A, R...>::operator() (Args&& ...args) const
			-> Value
		{
			return extrapolated_value (std::forward<Args> (args)...);
		}


template<class A, class ...R>
	template<class ...Args>
		inline auto
		Field<A, R...>::min_argument (Args&& ...xs) const
			-> ArgumentType<Args...>
		{
			static_assert (sizeof...(Args) <= kNumArguments - 1, "number of arguments must be between 0 and dimensions()-1");

			auto const get_iter = [](auto&& map) { return map.begin(); };

			if constexpr (sizeof...(Args) == 0)
				return compute_minmax_argument (_data_map, get_iter);
			else
				return compute_minmax_argument (_data_map, get_iter, std::forward<Args> (xs)...);
		}


template<class A, class ...R>
	template<class ...Args>
		inline auto
		Field<A, R...>::max_argument (Args&& ...xs) const
			-> ArgumentType<Args...>
		{
			static_assert (sizeof...(Args) <= kNumArguments - 1, "number of arguments must be between 0 and dimensions()-1");

			auto const get_iter = [](auto&& map) { return map.rbegin(); };

			if constexpr (sizeof...(Args) == 0)
				return compute_minmax_argument (_data_map, get_iter);
			else
				return compute_minmax_argument (_data_map, get_iter, std::forward<Args> (xs)...);
		}


template<class A, class ...R>
	template<class ...Args>
		inline auto
		Field<A, R...>::min_value_point (Args&& ...xs) const
			-> PointType<Args...>
		{
			static_assert (sizeof...(Args) <= kNumArguments - 1, "number of arguments must be between 0 and dimensions()-1");

			auto const is_better = [](auto&& a, auto&& b) { return a < b; };

			return compute_minmax_value (_data_map, is_better, std::forward<Args> (xs)...);
		}


template<class A, class ...R>
	template<class ...Args>
		inline auto
		Field<A, R...>::max_value_point (Args&& ...xs) const
			-> PointType<Args...>
		{
			static_assert (sizeof...(Args) <= kNumArguments - 1, "number of arguments must be between 0 and dimensions()-1");

			auto const is_better = [](auto&& a, auto&& b) { return a > b; };

			return compute_minmax_value (_data_map, is_better, std::forward<Args> (xs)...);
		}


template<class A, class ...R>
	template<class ...Args>
		inline auto
		Field<A, R...>::min_value (Args&& ...xs) const
			-> ValueType<Args...>
		{
			auto const point = min_value_point (xs...);

			if constexpr (sizeof...(Args) == 0)
				return point.value;
			else
			{
				if (point)
					return point->value;
				else
					return std::nullopt;
			}
		}


template<class A, class ...R>
	template<class ...Args>
		inline auto
		Field<A, R...>::max_value (Args&& ...xs) const
			-> ValueType<Args...>
		{
			auto const point = max_value_point (xs...);

			if constexpr (sizeof...(Args) == 0)
				return point.value;
			else
			{
				if (point)
					return point->value;
				else
					return std::nullopt;
			}
		}


template<class A, class ...R>
	template<class ...Args>
		inline auto
		Field<A, R...>::domain (Args&& ...xs) const
			-> DomainType<Args...>
		{
			if constexpr (sizeof...(Args) == 0)
				return { min_argument(), max_argument() };
			else
			{
				auto min = min_argument();

				if (!min)
					return std::nullopt;

				auto max = max_argument();

				if (!max)
					return std::nullopt;

				return { *min, *max };
			}
		}


template<class A, class ...R>
	template<class ...Args>
		[[nodiscard]]
		inline auto
		Field<A, R...>::codomain (Args&& ...args) const
			-> CodomainType<Args...>
		{
			if constexpr (sizeof...(Args) == 0)
				return { min_value(), max_value() };
			else
			{
				auto y_min = min_value_point (std::forward<Args> (args)...);

				if (!y_min)
					return std::nullopt;

				auto y_max = max_value_point (std::forward<Args> (args)...);

				if (!y_max)
					return std::nullopt;

				return { *y_min, *y_max };
			}
		}


#if 0
template<class A, class ...R>
	inline auto
	Field<A, R...>::arguments (ArgumentsWithoutLast const&, Value const& value) const
		-> std::vector<Point>
	{
		return arguments (value, domain());
	}


template<class A, class ...R>
	inline auto
	Field<A, R...>::arguments (ArgumentsWithoutLast const&, Value const& value, Range<LastArgument> search_domain) const;
		-> std::vector<Point>
	{
		if (_data_map.size() >= 2)
		{
			std::vector<Point> result;
			auto a = std::begin (_data_map);
			auto b = a;
			++b;

			// Handle the smallest argument in the domain:
			if (value == a->second && search_domain.includes (a->first))
				result.emplace_back (a->first, a->second);

			// Find range [a, b] such that @value fits into it.
			// Linearly interpolate the argument.
			for (; b != std::end (_data_map); ++a, ++b)
			{
				auto const val_a = a->second;
				auto const val_b = b->second;

				if ((val_a < value && value <= val_b) || (val_b <= value && value < val_a))
				{
					auto const arg_a = a->first;
					auto const arg_b = b->first;

					auto const arg = renormalize (value, val_a, val_b, arg_a, arg_b);

					if (search_domain.includes (arg))
						result.emplace_back (arg, value);
				}
			}

			return result;
		}
		else if (_data_map.size() == 1)
		{
			auto item = *_data_map.begin();

			if (item.second == value && search_domain.includes (item.first))
				return { Point (item.first, item.second) };
			else
				return { };
		}
		else
			return { };
	}


template<class A, class ...R>
	template<class>
		inline auto
		Field<A, R...>::average() const
			-> Value
		{
			return average (domain());
		}


template<class A, class ...R>
	template<class>
		inline auto
		Field<A, R...>::average (Range<Argument> search_domain) const
			-> Value
		{
			if (_data_map.size() > 1)
			{
				Range search_domain_2 {
					std::make_pair (search_domain.min(), Value()),
					std::make_pair (search_domain.max(), Value()),
				};
				auto it_range = find_range_exclusive (_data_map.begin(), _data_map.end(), search_domain_2,
													  [](auto const& a, auto const& b) { return a.first < b.first; });

				// If there's at least one iterator in range:
				if (it_range.first != _data_map.end())
				{
					typedef decltype (std::declval<Argument>() / std::declval<Argument>()) WeightType;
					Value total_avg = Value (0);
					WeightType total_weight = 0;

					auto update_average = [&total_avg, &total_weight](Argument arg_a, Argument arg_b, Value val_a, Value val_b)
					{
						Value avg_value = 0.5 * (val_a + val_b);
						WeightType weight = (arg_b - arg_a) / Argument (1);

						total_avg += avg_value * weight;
						total_weight += weight;
					};

					// Compute average value from domain.min() to first iterator:
					{
						auto iterator = it_range.first;
						update_average (search_domain.min(), iterator->first, extrapolated_value (search_domain.min()), iterator->second);
					}

					// Compute average values between iterators:
					{
						auto a = it_range.first;
						auto b = a;
						++b;

						for (; b != it_range.second; ++a, ++b)
							update_average (a->first, b->first, a->second, b->second);
					}

					// Compute average value from (last iterator - 1) to search_domain.max():
					{
						auto iterator = it_range.second;
						--iterator;

						update_average (iterator->first, search_domain.max(), iterator->second, extrapolated_value (search_domain.max()));
					}

					return total_avg / total_weight;
				}
				else
				{
					// Compute average between search_domain.min() and search_domain.max():
					return 0.5 * (extrapolated_value (search_domain.min()) + extrapolated_value (search_domain.max()));
				}
			}
			else
				return _data_map.begin()->second;
		}
#endif


template<class A, class ...R>
	inline auto
	Field<A, R...>::data_map() const
		-> DataMap const&
	{
		return _data_map;
	}


template<class A, class ...R>
	template<class Map>
		inline void
		Field<A, R...>::validate (Map const& map)
		{
			if (map.empty())
				throw EmptyDomainException();

			if constexpr (is_specialization_v<typename Map::mapped_type, std::map>)
			{
				for (auto const& pair: map)
					validate (pair.second);
			}
		}


template<class A, class ...R>
	template<class Map, class GetIter>
		inline auto
		Field<A, R...>::compute_minmax_argument (Map const& map, GetIter&& get_iter) const
			-> typename Map::key_type
		{
			return get_iter (map)->first;
		}


template<class A, class ...R>
	template<class Map, class GetIter, class X, class ...Xs>
		inline auto
		Field<A, R...>::compute_minmax_argument (Map const& map, GetIter&& get_iter, X&& x_unconverted, Xs&& ...xs) const
			-> std::optional<NthArgument<sizeof...(Xs) + 1u>>
		{
			using MapKey = typename Map::key_type;

			// "x" is a key in the "map" argument.

			auto const x = static_cast<MapKey> (x_unconverted);
			auto const get_first = [](auto pair) { return pair.first; };
			auto const [ia, ib] = adjacent_find (map.begin(), map.end(), x, get_first);

			if (ia == ib)
				return std::nullopt;

			Range const xrange { ia->first, ib->first };

			if constexpr (sizeof...(Xs) == 0)
			{
				Range const yrange { compute_minmax_argument (ia->second, get_iter), compute_minmax_argument (ib->second, get_iter) };
				return renormalize (x, xrange, yrange);
			}
			else
			{
				auto y_min = compute_minmax_argument (ia->second, get_iter);

				if (!y_min)
					return std::nullopt;

				auto y_max = compute_minmax_argument (ib->second, get_iter);

				if (!y_max)
					return std::nullopt;

				Range const yrange { *y_min, *y_max };
				return renormalize (x, xrange, yrange);
			}
		}


template<class A, class ...R>
	template<class Map, class IsBetter>
		inline auto
		Field<A, R...>::compute_minmax_value (Map const& map, IsBetter&& is_better) const
			-> MinMaxSubspacePoint<Map>
		{
			// If map values are also maps, recurse:
			if constexpr (is_specialization_v<typename Map::mapped_type, std::map>)
			{
				auto best = compute_minmax_value (map.begin()->second, is_better);
				auto best_argument = map.begin()->first;

				for (auto it = std::next (map.begin()); it != map.end(); ++it)
				{
					auto const better = compute_minmax_value (it->second, is_better);

					if (is_better (better.value, best.value))
					{
						best = better;
						best_argument = it->first;
					}
				}

				return SubspacePoint (std::tuple_cat (std::make_tuple (best_argument), best.arguments), best.value);
			}
			// If map values are scalars:
			else
			{
				auto best = SubspacePoint (std::make_tuple (map.begin()->first), map.begin()->second);

				for (auto const& i: map)
					if (is_better (i.second, best.value))
						best = SubspacePoint (std::make_tuple (i.first), i.second);

				return best;
			}
		}


template<class A, class ...R>
	template<class Map, class IsBetter, class X, class ...Xs>
		inline auto
		Field<A, R...>::compute_minmax_value (Map const& map, IsBetter&& is_better, X&& x_unconverted, Xs&& ...xs) const
			-> MinMaxSubspacePoint<Map, X, Xs...>
		{
			using MapKey = typename Map::key_type;
			using MapValue = typename Map::mapped_type;

			auto const x = static_cast<MapKey> (x_unconverted);

			// "x" is a key in the "map" argument.

			auto const get_first = [](auto pair) { return pair.first; };
			auto const [inside_domain, ia, ib] = adjacent_find_for_extrapolation (map.begin(), map.end(), x, get_first);
			MapKey const& key_a = ia->first;
			MapKey const& key_b = ib->first;
			MapValue const& value_a = ia->second;
			MapValue const& value_b = ib->second;

			if (!inside_domain)
				return std::nullopt;

			Range const xrange { key_a, key_b };

			if constexpr (sizeof...(xs) > 0)
			{
				auto p_min = compute_minmax_value (ia->second, is_better, std::forward<Xs> (xs)...);

				if (!p_min)
					return std::nullopt;

				auto p_max = compute_minmax_value (ib->second, is_better, std::forward<Xs> (xs)...);

				if (!p_max)
					return std::nullopt;

				auto const yrange = Range { p_min->value, p_max->value };
				auto const interpolated_value = renormalize (x, xrange, yrange);
				auto const interpolated_arguments = renormalize_tuple (x, xrange, p_min->arguments, p_max->arguments);
				return SubspacePoint (std::tuple_cat (std::make_tuple (x), interpolated_arguments), interpolated_value);
			}
			else
			{
				if constexpr (is_specialization_v<typename Map::mapped_type, std::map>)
				{
					// value_a, value_b are of type std::map.

					using Submap = typename Map::mapped_type;
					using SubmapKey = typename Submap::key_type;
					using SubmapValue = typename Submap::mapped_type;
					using BestValue = MinMaxSubspacePoint<Submap>;

					std::optional<BestValue> best;

					auto const best_value = [&] (Submap const& map, SubmapKey const& key) -> std::optional<BestValue> {
						if (auto found = map.find (key); found != map.end())
						{
							SubmapValue const& value = found->second;

							if constexpr (is_specialization_v<std::remove_cvref_t<decltype (value)>, std::map>)
							{
								auto const p = compute_minmax_value (value, is_better);
								return SubspacePoint { std::tuple_cat (std::make_tuple (key), p.arguments), p.value };
							}
							else
								return SubspacePoint { std::make_tuple (key), value };
						}
						else
							return std::nullopt;
					};

					auto const find_best_for_key = [&] (typename Submap::key_type const& subkey) -> void {
						auto const ia2 = best_value (value_a, subkey);

						if (!ia2)
							return;

						auto const ib2 = best_value (value_b, subkey);

						if (!ib2)
							return;

						auto const yrange2 = Range { ia2->value, ib2->value };
						auto const better_value = renormalize (x, xrange, yrange2);
						auto const better_arguments = renormalize_tuple (x, xrange, ia2->arguments, ib2->arguments);

						if (!best || is_better (better_value, best->value))
							best = SubspacePoint { better_arguments, better_value };
					};

					for (auto const& pair: value_a)
						find_best_for_key (pair.first);

					for (auto const& pair: value_b)
						find_best_for_key (pair.first);

					if (best)
						return SubspacePoint { std::tuple_cat (std::make_tuple (x), best->arguments), best->value };
					else
						return std::nullopt;
				}
				else
					return renormalize (x, xrange, Range { value_a, value_b });
			}
		}


template<class A, class ...R>
	template<class Map, class X, class ...Xs>
		inline auto
		Field<A, R...>::compute_value (Map&& map, bool extrapolate, X&& x, Xs&& ...xs) const
			-> std::optional<Value>
		{
			// "x" is a key in the "map" argument.

			auto const get_first = [](auto pair) { return pair.first; };
			auto const [inside_domain, ia, ib] = adjacent_find_for_extrapolation (map.begin(), map.end(), x, get_first);
			auto const& key_a = ia->first;
			auto const& key_b = ib->first;
			auto const& submap_a = ia->second;
			auto const& submap_b = ib->second;
			Range const xrange { key_a, key_b };

			if constexpr (sizeof...(xs) > 0)
			{
				// Get value recursively from the inner map:
				auto const y_min = compute_value (ia->second, extrapolate, std::forward<Xs> (xs)...);

				if (ia == ib)
					return y_min;
				else
				{
					if (!y_min)
						return std::nullopt;

					auto const y_max = compute_value (ib->second, extrapolate, std::forward<Xs> (xs)...);

					if (!y_max)
						return std::nullopt;

					auto const yrange = Range { *y_min, *y_max };

					if constexpr (kLinearExtrapolate)
						return renormalize (x, xrange, yrange);
					else
						return renormalize (clamped (x, Range<std::remove_cvref_t<X>> (xrange)), xrange, yrange);
				}
			}
			else
			{
				if (ia == ib)
				{
					return ia->second;
				}
				else if (inside_domain || extrapolate)
				{
					Range const yrange { submap_a, submap_b };

					if constexpr (kLinearExtrapolate)
						return renormalize (x, xrange, yrange);
					else
						return renormalize (clamped (x, Range<std::remove_cvref_t<X>> (xrange)), xrange, yrange);
				}
				else
					return std::nullopt;
			}
		}

} // namespace neutrino

#endif

