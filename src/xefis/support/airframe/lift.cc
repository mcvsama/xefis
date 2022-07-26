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

// Local:
#include "lift.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/math/field.h>
#include <neutrino/qt/qdom.h>
#include <neutrino/qt/qdom_iterator.h>
#include <neutrino/sequence_utils.h>
#include <neutrino/stdexcept.h>

// Standard:
#include <cstddef>
#include <algorithm>
#include <map>


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

			auto aoa = si::parse<si::Angle> (e.attribute ("aoa").toStdString());
			LiftCoefficient cl (e.attribute ("cl").toDouble());
			data[aoa] = cl;
		}
	}

	if (data.empty())
		throw BadConfiguration ("lift module not properly configured");

	_aoa_to_cl = Field<si::Angle, LiftCoefficient> (std::move (data));
	// TODO initialize _cl_to_aoa_normal_regime

	// Find maximum C_L and AOA angle for maximum C_L (critical AOA):
	auto p = _aoa_to_cl.max_value_point();
	_max_cl = p.value;
	_critical_aoa = std::get<0> (p.arguments);
}


LiftCoefficient
Lift::get_cl (si::Angle const aoa) const
{
	return _aoa_to_cl.extrapolated_value (aoa);
}


LiftCoefficient
Lift::max_cl() const noexcept
{
	return _max_cl;
}


si::Angle
Lift::critical_aoa() const noexcept
{
	return _critical_aoa;
}


std::optional<si::Angle>
Lift::get_aoa_in_normal_regime (LiftCoefficient const& cl) const noexcept
{
	return _cl_to_aoa_normal_regime (cl);
}

} // namespace xf

