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

#ifndef NEUTRINO__SEQUENCE_UTILS_H__INCLUDED
#define NEUTRINO__SEQUENCE_UTILS_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>
#include <map>

// Neutrino:
#include <neutrino/range.h>


namespace neutrino {

/**
 * Find two adjacent iterators a and b that satisfy a <= value && value <= b.
 * Result might be two identical iterators, or two end iterators.
 * If value < *begin, { begin, begin } is returned.
 * If value > *--end, { std::prev (end), std::prev (end) } is returned.
 */
template<class ConstIterator, class Value, class Accessor>
	inline std::pair<ConstIterator, ConstIterator>
	adjacent_find (ConstIterator begin, ConstIterator end, Value const& value, Accessor&& get_value)
	{
		if (begin == end)
			return { end, end };

		auto predicate = [&](auto const& a, auto const& b) {
			return get_value (a) <= value && value <= get_value (b);
		};

		ConstIterator it = std::adjacent_find (begin, end, predicate);

		if (it == end)
		{
			if (value < get_value (*begin))
				return { begin, begin };
			else
				return { std::prev (end), std::prev (end) };
		}
		else
		{
			ConstIterator ne = it;
			return { it, ++ne };
		}
	}


/**
 * Find two adjacent iterators a and b that satisfy a <= value && value <= b.
 * If no such iterators can be found, return two first or two last iterators from the sequence, as long
 * as sequence has at least two iterators.
 * If sequence has length 1, both returned iterators will point to the only element of the sequence.
 *
 * First value of returned tuple is true only if the iterators satisfy initial condition (a <= value && value <= b).
 */
template<class ConstIterator, class Value, class Accessor>
	inline std::tuple<bool, ConstIterator, ConstIterator>
	adjacent_find_for_extrapolation (ConstIterator begin, ConstIterator end, Value const& value, Accessor&& get_value)
	{
		switch (std::distance (begin, end))
		{
			case 0:
				return { false, end, end };

			case 1:
				return { get_value (*begin) == value, begin, begin };

			case 2:
				return { get_value (*begin) <= value && value <= get_value (*std::prev (end)), begin, std::prev (end) };

			default:
			{
				auto [a, b] = adjacent_find (begin, end, value, std::forward<Accessor> (get_value));

				if (a == end && b == end)
				{
					if (value < get_value (*begin))
						return { false, begin, std::next (begin) };
					else
						return { false, std::prev (end, 2), std::prev (end) };
				}
				else if (a == b)
				{
					if (value < get_value (*a))
						return { false, begin, std::next (begin) };
					else if (get_value (*b) < value)
						return { false, std::prev (end, 2), std::prev (end) };
					else
						return { get_value (*a) == value, a, b };
				}
				else
					return { true, a, b };
			}
		}
	}


/**
 * A simple trick to change const_iterator to iterator.
 */
template<class Container, typename ConstIterator>
	inline typename Container::iterator
	remove_constness (Container& container, ConstIterator iterator)
	{
		return container.erase (iterator, iterator);
	}


/**
 * Find a range of iterators which fall inside range of [min, max] values.
 * If no iterator matches inside [min, max] range, both result iterators
 * are set to 'last'.
 */
template<class ForwardIt, class Value, class Compare>
	inline std::pair<ForwardIt, ForwardIt>
	find_range_exclusive (ForwardIt first, ForwardIt last, Range<Value> const& value_range, Compare compare)
	{
		auto a = std::upper_bound (first, last, value_range.min(), compare);

		if (a == last)
			return { last, last };

		auto b = std::lower_bound (a, last, value_range.max(), compare);

		return { a, b };
	}

} // namespace neutrino

#endif

