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
#include <xefis/utility/field.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/qdom_iterator.h>
#include <xefis/utility/sequence_utils.h>

// Local:
#include "lift.h"


namespace xf {

Lift::Lift (QDomElement const& config)
{
	decltype (_aoa_to_cl)::DataMap data;

	for (QDomElement const& e: xf::iterate_sub_elements (config))
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

	_aoa_to_cl = Field<Angle, LiftCoefficient> (std::move (data));
	// TODO initialize _cl_to_aoa_normal_regime

	// Find maximum C_L and AOA angle for maximum C_L (critical AOA):
	auto p = _aoa_to_cl.max_value_point();
	_max_cl = p.value;
	_critical_aoa = std::get<0> (p.arguments);
}


LiftCoefficient
Lift::get_cl (Angle const& aoa) const
{
	return _aoa_to_cl.extrapolated_value (aoa);
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


std::optional<Angle>
Lift::get_aoa_in_normal_regime (LiftCoefficient const& cl) const noexcept
{
	return _cl_to_aoa_normal_regime (cl);
}

} // namespace xf

