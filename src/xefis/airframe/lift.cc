/* vim:ts=4
 *
 * Copyleft 2012…2015  Michał Gawron
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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/sequence.h>

// Local:
#include "lift.h"


namespace Xefis {

Lift::Lift (QDomElement const& config)
{
	for (QDomElement const& e: config)
	{
		if (e == "point")
		{
			if (!e.hasAttribute ("aoa"))
				throw MissingDomAttribute (e, "aoa");
			if (!e.hasAttribute ("cl"))
				throw MissingDomAttribute (e, "cl");

			Angle aoa;
			aoa.parse (e.attribute ("aoa").toStdString());
			LiftCoefficient cl (e.attribute ("cl").toDouble());
			_coeffs[aoa] = cl;
		}
	}

	if (_coeffs.empty())
		throw BadConfiguration ("lift module not properly configured");

	// Find maximum C_L and AOA angle for maximum C_L (critical AOA):
	_max_cl = _coeffs.begin()->second;
	for (auto cl: _coeffs)
	{
		if (cl.second > _max_cl)
		{
			_max_cl = cl.second;
			_critical_aoa = cl.first;
		}
	}
}


LiftCoefficient
Lift::get_cl (Angle const& aoa) const
{
	// Assume _coeffs is not empty (see ctor).

	auto range = extended_adjacent_find (_coeffs.begin(), _coeffs.end(), aoa, [](Coefficients::value_type pair) { return pair.first; });

	Range<Angle> from (range.first->first, range.second->first);
	Range<LiftCoefficient::Value> to (range.first->second.value(), range.second->second.value());

	return LiftCoefficient (xf::renormalize (aoa, from, to));
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


Angle
Lift::get_aoa_in_normal_regime (LiftCoefficient const& cl) const noexcept
{
	// Assume _coeffs is not empty (see ctor).

	auto range = extended_adjacent_find (_coeffs.begin(), _coeffs.end(), cl, [](Coefficients::value_type pair) { return pair.second; });

	return xf::renormalize (cl.value(),
							range.first->second.value(), range.second->second.value(),
							range.first->first, range.second->first);
}

} // namespace Xefis

