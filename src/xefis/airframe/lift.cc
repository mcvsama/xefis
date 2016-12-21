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
#include <algorithm>
#include <map>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/sequence.h>
#include <xefis/utility/datatable2d.h>

// Local:
#include "lift.h"


namespace xf {

Lift::Lift (QDomElement const& config)
{
	decltype (_aoa_to_cl)::element_type::DataMap data;

	for (QDomElement const& e: config)
	{
		if (e == "point")
		{
			if (!e.hasAttribute ("aoa"))
				throw MissingDomAttribute (e, "aoa");
			if (!e.hasAttribute ("cl"))
				throw MissingDomAttribute (e, "cl");

			auto aoa = parse<Angle> (e.attribute ("aoa").toStdString());
			LiftCoefficient cl (e.attribute ("cl").toDouble());
			data[aoa] = cl;
		}
	}

	if (data.empty())
		throw BadConfiguration ("lift module not properly configured");

	_aoa_to_cl = std::make_unique<Datatable2D<Angle, LiftCoefficient>> (std::move (data));

	// Find maximum C_L and AOA angle for maximum C_L (critical AOA):
	auto max_cl_point = _aoa_to_cl->max_value();
	_critical_aoa = max_cl_point.argument;
	_max_cl = max_cl_point.value;
}


LiftCoefficient
Lift::get_cl (Angle const& aoa) const
{
	return _aoa_to_cl->extrapolated_value (aoa);
}


LiftCoefficient
Lift::max_cl() const noexcept
{
	return _max_cl;
}


Angle
Lift::critical_aoa() const noexcept
{
	return _critical_aoa;
}


Optional<Angle>
Lift::get_aoa_in_normal_regime (LiftCoefficient const& cl) const noexcept
{
	auto aoas = _aoa_to_cl->arguments (cl, { _aoa_to_cl->min_argument().argument, _critical_aoa });

	if (aoas.empty())
		return { };

	// If AOA/C_L is not-monotonic, there may be multiple results.
	// In such case return largest matching AOA:
	return aoas.back().argument;
}

} // namespace xf

