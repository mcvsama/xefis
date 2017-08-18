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

// Local:
#include "unit.h"
#include "standard_unit_traits.h"


namespace si {

std::string
DynamicUnit::symbol() const
{
	auto s = symbols_map().find (*this);

	if (s != symbols_map().end())
		return s->second;
	else
	{
		if (scale().numerator() != 1 || scale().denominator() != 1 ||
			offset().numerator() != 0 || offset().denominator() != 1)
		{
			return "[unknown or scaled/offset unit]";
		}

		std::string result;
		// In such order:
		add_single_unit_symbol (result, mass_exponent(), "kg");
		add_single_unit_symbol (result, length_exponent(), "m");
		add_single_unit_symbol (result, time_exponent(), "s");
		add_single_unit_symbol (result, current_exponent(), "A");
		add_single_unit_symbol (result, temperature_exponent(), "K");
		add_single_unit_symbol (result, amount_exponent(), "mol");
		add_single_unit_symbol (result, luminous_intensity_exponent(), "cd");
		add_single_unit_symbol (result, angle_exponent(), "rad");
		return result;
	}
}


inline void
DynamicUnit::add_single_unit_symbol (std::string& result, int exponent, const char* symbol)
{
	using std::to_string;

	if (exponent >= 1 || exponent < 0)
		if (!result.empty())
			result += kDotProductSymbol_utf8;

	if (exponent == 1)
	{
		result += symbol;
	}
	else if (exponent > 1 || exponent < 0)
	{
		result += symbol;
		result += "^";
		result += to_string (exponent);
	}
}

} // namespace si

