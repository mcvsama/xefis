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
#include <xefis/utility/qdom.h>//TODO

// Local:
#include "rcms.h"


RemoteControlManagementSystem::RemoteControlManagementSystem (std::unique_ptr<RemoteControlManagementSystemIO> module_io, std::string const& instance):
	Module (std::move (module_io), instance)
{
	prepare_configurator_widget();

	_distance_computer.set_callback (std::bind (&RemoteControlManagementSystem::compute_distances_to_home, this));
	_distance_computer.observe ({
		&io.input_home_longitude,
		&io.input_home_latitude,
		&io.input_home_altitude_amsl,
		&io.input_position_longitude,
		&io.input_position_latitude,
		&io.input_position_altitude_amsl,
	});
}


QWidget*
RemoteControlManagementSystem::configurator_widget()
{
	return _configurator_widget.get();
}


void
RemoteControlManagementSystem::process (v2::Cycle const& cycle)
{
	if (!home_is_valid())
		acquire_home();

	_distance_computer.process (cycle.update_time());
}


void
RemoteControlManagementSystem::acquire_home()
{
	if (io.input_position_longitude && io.input_position_latitude && io.input_position_altitude_amsl)
	{
		io.input_home_longitude = io.input_position_longitude;
		io.input_home_latitude = io.input_position_latitude;
		io.input_home_altitude_amsl = io.input_position_altitude_amsl;
		_home_acquired = true;
	}
}


void
RemoteControlManagementSystem::prepare_configurator_widget()
{
	_configurator_widget = std::make_unique<QWidget> (nullptr);
	_configurator_widget->setSizePolicy (QSizePolicy::Minimum, QSizePolicy::Minimum);

	QPushButton* acquire_home_button = new QPushButton ("Acquire HOME position", _configurator_widget.get());
	QObject::connect (acquire_home_button, SIGNAL (clicked (bool)), this, SLOT (acquire_home()));

	QGridLayout* layout = new QGridLayout (_configurator_widget.get());
	layout->setMargin (WidgetMargin);
	layout->setSpacing (WidgetSpacing);
	layout->addWidget (acquire_home_button, 0, 0);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 0, 1);
	layout->addItem (new QSpacerItem (0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding), 1, 0);
}


bool
RemoteControlManagementSystem::home_is_valid() const
{
	return io.input_home_longitude && io.input_home_latitude && io.input_home_altitude_amsl;
}


bool
RemoteControlManagementSystem::position_is_valid() const
{
	return io.input_position_longitude && io.input_position_latitude && io.input_position_altitude_amsl;
}


void
RemoteControlManagementSystem::compute_distances_to_home()
{
	if (home_is_valid() && position_is_valid())
	{
		si::LonLat home (*io.input_home_longitude, *io.input_home_latitude);
		si::LonLat curr (*io.input_position_longitude, *io.input_position_latitude);
		si::Length ground_dist = xf::haversine_earth (curr, home);
		si::Length alt_diff = *io.input_position_altitude_amsl - *io.input_home_altitude_amsl;

		io.output_distance_vertical = alt_diff;
		io.output_distance_ground = ground_dist;
		io.output_distance_vlos = si::sqrt (ground_dist * ground_dist + alt_diff * alt_diff);
		io.output_true_home_direction = xf::floored_mod (xf::initial_bearing (curr, home), 360_deg);
	}
	else
	{
		io.output_distance_vlos.set_nil();
		io.output_distance_ground.set_nil();
		io.output_distance_vertical.set_nil();
		io.output_true_home_direction.set_nil();
	}
}

