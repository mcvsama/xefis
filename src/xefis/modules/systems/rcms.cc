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

// Qt:
#include <QtXml/QDomElement>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGridLayout>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/navigation/earth.h>

// Local:
#include "rcms.h"


RemoteControlManagementSystem::RemoteControlManagementSystem (std::unique_ptr<RemoteControlManagementSystemIO> module_io, std::string_view const& instance):
	Module (std::move (module_io), instance)
{
	_distance_computer.set_callback (std::bind (&RemoteControlManagementSystem::compute_distances_to_home, this));
	_distance_computer.observe ({
		&io.home_longitude,
		&io.home_latitude,
		&io.home_altitude_amsl,
		&io.position_longitude,
		&io.position_latitude,
		&io.position_altitude_amsl,
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
	return io.home_longitude && io.home_latitude && io.home_altitude_amsl;
}


bool
RemoteControlManagementSystem::position_is_valid() const
{
	return io.position_longitude && io.position_latitude && io.position_altitude_amsl;
}


void
RemoteControlManagementSystem::compute_distances_to_home()
{
	if (home_is_valid() && position_is_valid())
	{
		si::LonLat home (*io.home_longitude, *io.home_latitude);
		si::LonLat curr (*io.position_longitude, *io.position_latitude);
		si::Length ground_dist = xf::haversine_earth (curr, home);
		si::Length alt_diff = *io.position_altitude_amsl - *io.home_altitude_amsl;

		io.distance_vertical = alt_diff;
		io.distance_ground = ground_dist;
		io.distance_vlos = si::sqrt (ground_dist * ground_dist + alt_diff * alt_diff);
		io.true_home_direction = xf::floored_mod (xf::initial_bearing (curr, home), 360_deg);
	}
	else
	{
		io.distance_vlos = xf::nil;
		io.distance_ground = xf::nil;
		io.distance_vertical = xf::nil;
		io.true_home_direction = xf::nil;
	}
}

