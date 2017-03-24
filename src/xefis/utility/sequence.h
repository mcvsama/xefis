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

#ifndef XEFIS__UTILITY__SEQUENCE_H__INCLUDED
#define XEFIS__UTILITY__SEQUENCE_H__INCLUDED

// Standard:
#include <cstddef>
#include <stdexcept>
#include <map>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/range.h>


namespace xf {

/**
 * Find two adjacent iterators a and b that satisfy a <= value && value <= b.
 * Result might be two identical iterators, or two end iterators.
 */
template<class ConstIterator, class Value, class Accessor>
	inline std::pair<ConstIterator, ConstIterator>
	extended_adjacent_find (ConstIterator begin, ConstIterator end, Value const& value, Accessor access)
	{
		if (begin == end)
			return { end, end };

		auto predicate = [&](auto const& a, auto const& b) {
			return access (a) <= value && value <= access (b);
		};

		ConstIterator it = std::adjacent_find (begin, end, predicate);

		if (it == end)
		{
			if (value < access (*begin))
				return { begin, begin };
			else
			{
				ConstIterator pre_end = end;
				--pre_end;
				return { pre_end, pre_end };
			}
		}
		else
		{
			ConstIterator ne = it;
			return { it, ++ne };
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
	find_range_exclusive (ForwardIt first, ForwardIt last, Range<Value> value_range, Compare compare)
	{
		auto a = std::upper_bound (first, last, value_range.min(), compare);

		if (a == last)
			return { last, last };

		auto b = std::lower_bound (a, last, value_range.max(), compare);

		return { a, b };
	}

} // namespace xf

#endif

