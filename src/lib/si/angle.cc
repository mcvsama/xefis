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
Angle::to_dms() const
{
	double const degs = std::trunc (Xefis::floored_mod (deg(), -180.0, +180.0));
	double const remainder = 60.0 * std::abs (deg() - degs);
	double const mins = std::floor (remainder);
	double const secs = 60.0 * std::abs (remainder - mins);

	return (boost::format ("%02d°%02d'%02d\"") % static_cast<int> (degs) % static_cast<int> (mins) % static_cast<int> (secs)).str();
}


std::string
Angle::to_latitude_dms() const
{
	std::string dms = to_dms();

	if (dms.empty())
		return dms;
	else if (dms[0] == '-')
		return dms.substr (1) + "S";
	else
		return dms + "N";
}


std::string
Angle::to_longitude_dms() const
{
	std::string dms = to_dms();

	if (dms.empty())
		return dms;
	else if (dms[0] == '-')
		return dms.substr (1) + "W";
	else
		return dms + "E";
}

} // namespace SI

