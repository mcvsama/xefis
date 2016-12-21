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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/datatable2d.h>

// Local:
#include "drag.h"


namespace xf {

Drag::Drag (QDomElement const& config)
{
	decltype (_aoa_to_cd)::element_type::DataMap data;

	for (QDomElement const& e: config)
	{
		if (e == "point")
		{
			if (!e.hasAttribute ("aoa"))
				throw MissingDomAttribute (e, "aoa");
			if (!e.hasAttribute ("cd"))
				throw MissingDomAttribute (e, "cd");

			auto aoa = parse<Angle> (e.attribute ("aoa").toStdString());
			DragCoefficient cd = e.attribute ("cd").toDouble();
			data[aoa] = cd;
		}
	}

	if (data.empty())
		throw BadConfiguration ("drag module not properly configured");

	_aoa_to_cd = std::make_unique<Datatable2D<Angle, DragCoefficient>> (std::move (data));
}


DragCoefficient
Drag::get_cd (Angle const& aoa) const
{
	return _aoa_to_cd->extrapolated_value (aoa);
}

} // namespace xf

