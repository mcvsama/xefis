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

#ifndef SI__FUNCTIONS_H__INCLUDED
#define SI__FUNCTIONS_H__INCLUDED

// Standard:
#include <cstddef>


namespace si {

using namespace quantities;
using namespace units;
using namespace literals;


/*
 * Angle stuff
 */

#define FORWARD_ANGLE_TO_STD_FUNCTION_1(function_name)							\
	inline quantities::Angle::Value function_name (quantities::Angle const& a)	\
	{																			\
		return std::function_name (a.quantity<units::Radian>());				\
	}

FORWARD_ANGLE_TO_STD_FUNCTION_1 (sin)
FORWARD_ANGLE_TO_STD_FUNCTION_1 (cos)
FORWARD_ANGLE_TO_STD_FUNCTION_1 (tan)

#undef FORWARD_ANGLE_TO_STD_FUNCTION_1

extern std::string
to_dms (Angle a, bool three_digits);

extern std::string
to_latitude_dms (Angle a);

extern std::string
to_longitude_dms (Angle a);

/**
 * Mean value for two angles on a circle.
 */
extern Angle
mean (Angle lhs, Angle rhs);

} // namespace si

#endif

