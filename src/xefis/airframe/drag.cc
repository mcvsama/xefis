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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "drag.h"


namespace Xefis {

Drag::Drag (QDomElement const& config)
{
	for (QDomElement const& e: config)
	{
		if (e == "point")
		{
			if (!e.hasAttribute ("aoa"))
				throw MissingDomAttribute (e, "aoa");
			if (!e.hasAttribute ("cd"))
				throw MissingDomAttribute (e, "cd");

			Angle aoa;
			aoa.parse (e.attribute ("aoa").toStdString());
			double cd = e.attribute ("cd").toDouble();
			_coeffs[aoa] = cd;
		}
	}

	if (_coeffs.empty())
		throw BadConfiguration ("lift module not properly configured");
}


double
Drag::get_cd (Angle const& aoa) const
{
	// Assume _coeffs is not empty (see ctor).

	Coefficients::const_iterator ub = _coeffs.upper_bound (aoa);

	if (ub == _coeffs.end())
		return (--ub)->second;

	if (ub == _coeffs.begin())
		return ub->second;

	Coefficients::const_iterator lb = ub;
	--lb;

	return xf::renormalize (aoa, Range<Angle> (lb->first, ub->first), Range<double> (lb->second, ub->second));
}

} // namespace Xefis

