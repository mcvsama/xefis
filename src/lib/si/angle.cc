/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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

// Lib:
#include <boost/format.hpp>

// Xefis:
#include <xefis/utility/numeric.h>

// Local:
#include "angle.h"


namespace SI {

std::vector<std::string> Angle::_supported_units = { "°", "deg", "rad" };


std::string
Angle::to_dms (bool three_digits) const
{
	double const degs = std::trunc (Xefis::floored_mod (deg(), -180.0, +180.0));
	double const remainder = 60.0 * std::abs (deg() - degs);
	double const mins = std::floor (remainder);
	double const secs = 60.0 * std::abs (remainder - mins);

	char const* fmt = three_digits
		? "%03d°%02d'%02d\""
		: "%02d°%02d'%02d\"";
	return (boost::format (fmt) % static_cast<int> (degs) % static_cast<int> (mins) % static_cast<int> (secs)).str();
}


std::string
Angle::to_latitude_dms() const
{
	std::string dms = to_dms (false);

	if (dms.empty())
		return dms;
	else if (dms[0] == '-')
		return "S" + dms.substr (1);
	else
		return "N" + dms;
}


std::string
Angle::to_longitude_dms() const
{
	std::string dms = to_dms (true);

	if (dms.empty())
		return dms;
	else if (dms[0] == '-')
		return "W" + dms.substr (1);
	else
		return "E" + dms;
}


Angle
Angle::mean (Angle lhs, Angle rhs)
{
	ValueType x = 0.5 * std::cos (lhs) + std::cos (rhs);
	ValueType y = 0.5 * std::sin (lhs) + std::sin (rhs);
	return 1_rad * std::atan2 (y, x);
}

} // namespace SI

