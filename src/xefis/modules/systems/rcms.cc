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
#include "rcms.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/earth/earth.h>

// Qt:
#include <QtXml/QDomElement>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGridLayout>

// Standard:
#include <cstddef>


RemoteControlManagementSystem::RemoteControlManagementSystem (std::string_view const& instance):
	RemoteControlManagementSystemIO (instance)
{
	_distance_computer.set_callback (std::bind (&RemoteControlManagementSystem::compute_distances_to_home, this));
	_distance_computer.observe ({
		&_io.home_longitude,
		&_io.home_latitude,
		&_io.home_altitude_amsl,
		&_io.position_longitude,
		&_io.position_latitude,
		&_io.position_altitude_amsl,
	});
}


void
RemoteControlManagementSystem::process (xf::Cycle const& cycle)
{
	_distance_computer.process (cycle.update_time());
}


bool
RemoteControlManagementSystem::home_is_valid() const
{
	return _io.home_longitude && _io.home_latitude && _io.home_altitude_amsl;
}


bool
RemoteControlManagementSystem::position_is_valid() const
{
	return _io.position_longitude && _io.position_latitude && _io.position_altitude_amsl;
}


void
RemoteControlManagementSystem::compute_distances_to_home()
{
	if (home_is_valid() && position_is_valid())
	{
		si::LonLat home (*_io.home_longitude, *_io.home_latitude);
		si::LonLat curr (*_io.position_longitude, *_io.position_latitude);
		si::Length ground_dist = xf::haversine_earth (curr, home);
		si::Length alt_diff = *_io.position_altitude_amsl - *_io.home_altitude_amsl;

		_io.distance_vertical = alt_diff;
		_io.distance_ground = ground_dist;
		_io.distance_vlos = si::sqrt (ground_dist * ground_dist + alt_diff * alt_diff);
		_io.true_home_direction = xf::floored_mod (xf::initial_bearing (curr, home), 360_deg);
	}
	else
	{
		_io.distance_vlos = xf::nil;
		_io.distance_ground = xf::nil;
		_io.distance_vertical = xf::nil;
		_io.true_home_direction = xf::nil;
	}
}

