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

// Standard:
#include <cstddef>

// Boost:
#include <boost/lexical_cast.hpp>

// Local:
#include "utils.h"
#include "standard_unit_traits.h"


namespace si {

DynamicUnit
parse_unit (std::string_view const& str)
{
	using std::string_view;

	DynamicUnit result (0, 0, 0, 0, 0, 0, 0, 0, DynamicRatio (1, 1), DynamicRatio (0, 1));

	string_view::size_type prev = 0;
	string_view::size_type p = 0;
	string_view::size_type j = 0;

	// Every time we find a '/' character in the sequence,
	// we switch to dividing mode (exponent gets negative).
	int exponent_sign = 1;

	do {
		p = str.find_first_of (" /", p);

		auto single_step = str.substr (prev, p - prev);
		bool divide_char_found = p != str.npos && str[p] == '/';
		// Now single_step should have form "x^n" or just "x".
		if (!single_step.empty())
		{
			j = single_step.find ('^');
			auto unit_str = single_step.substr (0, j);
			auto exponent_str = single_step.substr (j + 1);

			int exponent = 1;

			if (j != string_view::npos)
			{
				try {
					exponent = boost::lexical_cast<int> (exponent_str);
				}
				catch (boost::bad_lexical_cast&)
				{
					throw UnsupportedUnit ("could not process exponent '" + std::string (exponent_str) + "'");
				}
			}

			exponent *= exponent_sign;

			auto unit_it = units_map().find (std::string (unit_str));

			if (unit_it == units_map().end())
				throw UnsupportedUnit ("could not process unit '" + std::string (unit_str) + "'");

			DynamicUnit unit = unit_it->second;

			for (auto& exp: unit.exponents())
				exp *= exponent;

			// Update unit scale:
			DynamicRatio scale (1, 1);
			if (exponent > 0)
				for (int i = 0; i < exponent; ++i)
					scale *= unit.scale();
			else if (exponent < 0)
				for (int i = 0; i < -exponent; ++i)
					scale /= unit.scale();
			result.scale() *= scale;

			// Check offset:
			// TODO support it
			if (unit.offset() != DynamicRatio (0, 1))
				throw UnsupportedUnit ("units with non-zero offset not supported yet");

			// Add new unit to resulting unit:
			for (std::size_t i = 0; i < result.exponents().size(); ++i)
				result.exponents()[i] += unit.exponents()[i];

			// Reset to multiplying mode:
			exponent_sign = 1;
		}

		// If there was a '/' previously, set state to dividing:
		if (divide_char_found)
			exponent_sign = -1;

		prev = p + 1;
	} while (p++ != str.npos);

	return result;
}

} // namespace si

