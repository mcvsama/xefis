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

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>

// Local:
#include "slip_skid.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/slip-skid", SlipSkid);


SlipSkid::SlipSkid (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_properties (config, {
		// Input:
		{ "acceleration.y", _y_acceleration, true },
		{ "acceleration.z", _z_acceleration, true },
		// Output:
		{ "slip-skid", _slip_skid, true },
	});

	_slip_skid_computer.set_callback (std::bind (&SlipSkid::compute_slip_skid, this));
	_slip_skid_computer.observe ({ &_y_acceleration, &_z_acceleration });
}


void
SlipSkid::data_updated()
{
	_slip_skid_computer.data_updated (update_time());
}


void
SlipSkid::compute_slip_skid()
{
	if (_y_acceleration.valid() && _z_acceleration.valid())
	{
		float const scale = 10.f;
		_slip_skid.write (scale * std::atan2 (*_y_acceleration, -*_z_acceleration) / (2.f * M_PI));
	}
	else
		_slip_skid.set_nil();
}

