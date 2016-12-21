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
#include <boost/format.hpp>

// Xefis:
#include <xefis/utility/numeric.h>


namespace si {

std::string
to_dms (Angle a, bool three_digits)
{
	auto const angle_degs = a.quantity<Degree>();
	auto const degs = std::trunc (xf::floored_mod (angle_degs, -180.0, +180.0));
	auto const remainder = 60.0 * std::abs (angle_degs - degs);
	auto const mins = std::floor (remainder);
	auto const secs = 60.0 * std::abs (remainder - mins);

	char const* fmt = three_digits
		? "%03d°%02d'%02d\""
		: "%02d°%02d'%02d\"";
	return (boost::format (fmt) % static_cast<int> (degs) % static_cast<int> (mins) % static_cast<int> (secs)).str();
}


std::string
to_latitude_dms (Angle a)
{
	std::string dms = to_dms (a, false);

	if (dms.empty())
		return dms;
	else if (dms[0] == '-')
		return "S" + dms.substr (1);
	else
		return "N" + dms;
}


std::string
to_longitude_dms (Angle a)
{
	std::string dms = to_dms (a, true);

	if (dms.empty())
		return dms;
	else if (dms[0] == '-')
		return "W" + dms.substr (1);
	else
		return "E" + dms;
}


/**
 * Mean value for two angles on a circle.
 */
Angle
mean (Angle lhs, Angle rhs)
{
	using std::sin;
	using std::cos;
	using std::atan2;

	Angle::Value x = 0.5 * cos (lhs) + cos (rhs);
	Angle::Value y = 0.5 * sin (lhs) + sin (rhs);
	return 1_rad * atan2 (y, x);
}

} // namespace si

