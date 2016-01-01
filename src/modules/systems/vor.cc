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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>

// Local:
#include "vor.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/vor", VOR);


VOR::VOR (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_properties (config, {
		{ "input.magnetic.declination", _input_magnetic_declination, true },
		{ "input.station.latitude", _input_station_latitude, true },
		{ "input.station.longitude", _input_station_longitude, true },
		{ "input.aircraft.latitude", _input_aircraft_latitude, true },
		{ "input.aircraft.longitude", _input_aircraft_longitude, true },
		{ "input.radial.magnetic", _input_radial_magnetic, true },
		{ "output.radial.magnetic", _output_radial_magnetic, false },
		{ "output.reciprocal.magnetic", _output_reciprocal_magnetic, false },
		{ "output.initial-bearing.magnetic", _output_initial_bearing_magnetic, false },
		{ "output.deviation", _output_deviation, true },
		{ "output.to-flag", _output_to_flag, true },
		{ "output.distance", _output_distance, true },
	});

	_vor_computer.set_callback (std::bind (&VOR::compute, this));
	_vor_computer.add_depending_smoothers ({
		&_deviation_smoother,
	});
	_vor_computer.observe ({
		&_input_magnetic_declination,
		&_input_station_latitude,
		&_input_station_longitude,
		&_input_aircraft_latitude,
		&_input_aircraft_longitude,
		&_input_radial_magnetic,
	});
}


void
VOR::data_updated()
{
	_vor_computer.data_updated (update_time());
}


void
VOR::compute()
{
	using std::abs;

	Time dt = _vor_computer.update_dt();

	if (_input_magnetic_declination.valid() &&
		_input_station_latitude.valid() && _input_station_longitude.valid() &&
		_input_aircraft_latitude.valid() && _input_aircraft_longitude.valid() &&
		_input_radial_magnetic.valid())
	{
		// Convert magnetic radial to true radial:
		Angle const declination = *_input_magnetic_declination;
		Angle const input_radial = *_input_radial_magnetic + declination;

		LonLat const station_position (*_input_station_longitude, *_input_station_latitude);
		LonLat const aircraft_position (*_input_aircraft_longitude, *_input_aircraft_latitude);

		Angle current_radial = normalize (station_position.initial_bearing (aircraft_position));
		Angle deviation = xf::floored_mod (input_radial - current_radial, -180_deg, +180_deg);
		if (abs (deviation) > 90_deg)
			deviation = -denormalize (deviation + 180_deg);

		if (_output_radial_magnetic.configured())
			_output_radial_magnetic = normalize (current_radial - declination);
		if (_output_reciprocal_magnetic.configured())
			_output_reciprocal_magnetic = normalize (current_radial + 180_deg - declination);
		if (_output_initial_bearing_magnetic.configured())
			_output_initial_bearing_magnetic = normalize (aircraft_position.initial_bearing (station_position) - declination);
		_output_to_flag = abs (denormalize (current_radial - input_radial)) > 90_deg;
		_output_deviation = 1_deg * _deviation_smoother.process (deviation.deg(), dt);
		_output_distance = station_position.haversine_earth (aircraft_position);
	}
	else
	{
		_output_deviation.set_nil();
		_output_to_flag.set_nil();
	}
}


inline Angle
VOR::normalize (Angle a)
{
	return xf::floored_mod (a, 0_deg, 360_deg);
}


inline Angle
VOR::denormalize (Angle a)
{
	return xf::floored_mod (a, -180_deg, +180_deg);
}

