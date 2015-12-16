/* vim:ts=4
 *
 * Copyleft 2012…2015  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__UTILITY__DATATABLE2D_H__INCLUDED
#define XEFIS__UTILITY__DATATABLE2D_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/sequence.h>


namespace Xefis {

template<class pArgument, class pValue>
	class Datatable2D
	{
	  public:
		typedef pArgument					Argument;
		typedef pValue						Value;
		typedef std::map<Argument, Value>	DataMap;

		/**
		 * Single row in data-table - argument and value pair.
		 */
		class Point
		{
		  public:
			Argument	argument;
			Value		value;

		  public:
			Point (Argument argument, Value value):
				argument (argument),
				value (value)
			{ }
		};

		/**
		 * Thrown when coefficients map passed to the Datatable2D
		 * doesn't have any points defined.
		 */
		class EmptyDomainException: public xf::Exception
		{
		  public:
			EmptyDomainException():
				Exception ("datatable domain must not be empty")
			{ }
		};

	  public:
		/**
		 * Create a data-table from argument-value points given in the map.
		 */
		explicit Datatable2D (DataMap const&);

		/**
		 * Create a data-table from argument-value points given in the map.
		 */
		explicit Datatable2D (DataMap&&);

		/**
		 * Return value for given argument.
		 * Result will be defined only if argument is within bounds of data-table arguments.
		 * The result will be interpolated.
		 */
		Optional<Value>
		value (Argument const&) const noexcept;

		/**
		 * Return value for given argument.
		 * If argument is outside bounds of data-table arguments, result will be extrapolated;
		 * otherwise result will be the same as from calling value().
		 */
		Value
		extrapolated_value (Argument const&) const noexcept;

		/**
		 * Return the point of minimum known argument.
		 */
		Point
		min_argument() const noexcept;

		/**
		 * Return the point of maximum known argument.
		 */
		Point
		max_argument() const noexcept;

		/**
		 * Return the point of minimum value.
		 */
		Point
		min_value() const noexcept;

		/**
		 * Return the point of maximum value.
		 */
		Point
		max_value() const noexcept;

		/**
		 * Return domain (range of arguments) of the function.
		 */
		Range<Argument>
		domain() const noexcept;

		/**
		 * Return codomain (range of values) of the function.
		 */
		Range<Value>
		codomain() const noexcept;

		/**
		 * Return a vector of arguments for given value.
		 */
		std::vector<Point>
		arguments (Value const& value) const;

		/**
		 * Return a vector of arguments for given value.
		 * Search only the given domain (range is inclusive).
		 */
		std::vector<Point>
		arguments (Value const& value, Range<Argument> search_domain) const;

	  private:
		/**
		 * Return interpolated value.
		 * Argument must be inside data-table domain.
		 */
		Value
		in_domain_value (Argument const&) const noexcept;

	  private:
		DataMap					_data_map;
		mutable Optional<Point>	_cached_min_value;
		mutable Optional<Point>	_cached_max_value;
	};


template<class A, class V>
	inline
	Datatable2D<A, V>::Datatable2D (DataMap const& map):
		_data_map (map)
	{
		if (map.empty())
			throw EmptyDomainException();
	}


template<class A, class V>
	inline
	Datatable2D<A, V>::Datatable2D (DataMap&& map):
		_data_map (std::move (map))
	{
		if (map.empty())
			throw EmptyDomainException();
	}


template<class A, class V>
	inline Optional<typename Datatable2D<A, V>::Value>
	Datatable2D<A, V>::value (Argument const& argument) const noexcept
	{
		// Outside of domain?
		if (argument < _data_map.begin()->first ||
			argument > _data_map.rbegin()->first)
		{
			return { };
		}

		return in_domain_value (argument);
	}


template<class A, class V>
	inline typename Datatable2D<A, V>::Value
	Datatable2D<A, V>::extrapolated_value (Argument const& argument) const noexcept
	{
		return in_domain_value (argument);
	}


template<class A, class V>
	inline typename Datatable2D<A, V>::Point
	Datatable2D<A, V>::min_argument() const noexcept
	{
		return { _data_map.begin()->first, _data_map.begin()->second };
	}


template<class A, class V>
	inline typename Datatable2D<A, V>::Point
	Datatable2D<A, V>::max_argument() const noexcept
	{
		return { _data_map.rbegin()->first, _data_map.rbegin()->second };
	}


template<class A, class V>
	inline typename Datatable2D<A, V>::Point
	Datatable2D<A, V>::min_value() const noexcept
	{
		if (!_cached_min_value)
		{
			auto min = _data_map.begin();
			for (auto i = min; i != _data_map.end(); ++i)
				if (i->second < min->second)
					min = i;
			_cached_min_value = Point { min->first, min->second };
		}

		return *_cached_min_value;
	}


template<class A, class V>
	inline typename Datatable2D<A, V>::Point
	Datatable2D<A, V>::max_value() const noexcept
	{
		if (!_cached_max_value)
		{
			auto max = _data_map.begin();
			for (auto i = max; i != _data_map.end(); ++i)
				if (i->second > max->second)
					max = i;
			_cached_max_value = Point { max->first, max->second };
		}

		return *_cached_max_value;
	}


template<class A, class V>
	inline Range<typename Datatable2D<A, V>::Argument>
	Datatable2D<A, V>::domain() const noexcept
	{
		return { min_argument().argument, max_argument().argument };
	}


template<class A, class V>
	inline Range<typename Datatable2D<A, V>::Value>
	Datatable2D<A, V>::codomain() const noexcept
	{
		return { min_value().value, max_value().value };
	}


template<class A, class V>
	inline std::vector<typename Datatable2D<A, V>::Point>
	Datatable2D<A, V>::arguments (Value const& value) const
	{
		return arguments (value, codomain());
	}


template<class A, class V>
	inline std::vector<typename Datatable2D<A, V>::Point>
	Datatable2D<A, V>::arguments (Value const& value, Range<Argument> search_domain) const
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

					auto const arg = xf::renormalize (value, val_a, val_b, arg_a, arg_b);

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
				return { Point { item.first, item.second } };
			else
				return { };
		}
		else
			return { };
	}


template<class A, class V>
	inline typename Datatable2D<A, V>::Value
	Datatable2D<A, V>::in_domain_value (Argument const& argument) const noexcept
	{
		auto range = extended_adjacent_find (_data_map.begin(), _data_map.end(), argument,
											 [](auto pair) { return pair.first; });

		Range<Argument> from { range.first->first, range.second->first };
		Range<Value> to { range.first->second, range.second->second };

		return renormalize (argument, from, to);
	}

} // namespace Xefis

#endif

