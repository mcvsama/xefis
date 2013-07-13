/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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
#include "rcms.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/rcms", RemoteControlManagementSystem);


RemoteControlManagementSystem::RemoteControlManagementSystem (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	for (QDomElement& e: config)
	{
		if (e == "properties")
		{
			parse_properties (e, {
				// Input:
				{ "home.longitude", _home_longitude, true },
				{ "home.latitude", _home_latitude, true },
				{ "home.altitude-amsl", _home_altitude_amsl, true },
				{ "position.longitude", _position_longitude, true },
				{ "position.latitude", _position_latitude, true },
				{ "position.altitude-amsl", _position_altitude_amsl, true },
				// Output:
				{ "home.distance.vlos", _distance_vlos, false },
				{ "home.distance.ground", _distance_ground, false },
				{ "home.distance.vertical", _distance_vertical, false },
				{ "home.true-direction", _true_home_direction, false },
			});
		}
	}

	_distance_computer.set_callback (std::bind (&RemoteControlManagementSystem::compute_distances_to_home, this));
	_distance_computer.observe ({ &_home_longitude, &_home_latitude, &_home_altitude_amsl,
								  &_position_longitude, &_position_latitude, &_position_altitude_amsl });
}


bool
RemoteControlManagementSystem::home_is_valid() const
{
	return _home_longitude.valid() && _home_latitude.valid() && _home_altitude_amsl.valid();
}


bool
RemoteControlManagementSystem::position_is_valid() const
{
	return _position_longitude.valid() && _position_latitude.valid() && _position_altitude_amsl.valid();
}


void
RemoteControlManagementSystem::acquire_home()
{
	if (_position_longitude.valid() && _position_latitude.valid() && _position_altitude_amsl.valid() &&
		!_home_longitude.is_singular() && !_home_latitude.is_singular() && !_home_altitude_amsl.is_singular())
	{
		_home_longitude.copy (_position_longitude);
		_home_latitude.copy (_position_latitude);
		_home_altitude_amsl.copy (_position_altitude_amsl);
		_home_acquired = true;
	}
}


void
RemoteControlManagementSystem::data_updated()
{
	_now = Time::now();

	if (!home_is_valid())
		acquire_home();

	_distance_computer.data_updated();
}


void
RemoteControlManagementSystem::compute_distances_to_home()
{
	if (home_is_valid() && position_is_valid())
	{
		LonLat home (*_home_longitude, *_home_latitude);
		LonLat curr (*_position_longitude, *_position_latitude);
		Length ground_dist = curr.haversine_earth (home);
		Length alt_diff = *_position_altitude_amsl - *_home_altitude_amsl;

		_distance_vertical.write (alt_diff);
		_distance_ground.write (ground_dist);
		_distance_vlos.write (1_nm * std::sqrt (ground_dist.nm() * ground_dist.nm() + alt_diff.nm() * alt_diff.nm()));
		_true_home_direction.write (curr.initial_bearing (home));
	}
	else
	{
		_distance_vlos.set_nil();
		_distance_ground.set_nil();
		_distance_vertical.set_nil();
		_true_home_direction.set_nil();
	}
}

