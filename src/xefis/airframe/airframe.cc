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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/application.h>
#include <xefis/utility/qdom.h>

// Local:
#include "airframe.h"


namespace Xefis {

Airframe::Airframe (Application*, QDomElement const& config)
{
	if (!config.isNull())
	{
		for (QDomElement const& e: config)
		{
			if (e == "flaps")
			{
				_flaps = std::make_unique<Flaps> (e);
			}
		}
	}
}

} // namespace Xefis

