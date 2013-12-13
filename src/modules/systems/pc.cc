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
#include <xefis/config/exception.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>
#include <xefis/airnav/wind_triangle.h>

// Local:
#include "pc.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/pc", PerformanceComputer);


PerformanceComputer::PerformanceComputer (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	_wind_direction_smoother.set_winding ({ 0.0, 360.0 });

	parse_settings (config, {
		{ "total-energy-variometer.minimum-ias", _total_energy_variometer_min_ias, true },
	});

	parse_properties (config, {
		// Input:
		{ "aircraft.weight", _aircraft_weight_g, true },
		{ "speed.ias", _speed_ias, true },
		{ "speed.tas", _speed_tas, true },
		{ "speed.gs", _speed_gs, true },
		{ "vertical-speed", _vertical_speed, true },
		{ "altitude.amsl.std", _altitude_amsl_std, true },
		{ "track.lateral.true", _track_lateral_true, true },
		{ "orientation.heading.true", _orientation_heading_true, true },
		{ "magnetic.declination", _magnetic_declination, true },
		// Output:
		{ "wind.from.true", _wind_from_true, true },
		{ "wind.from.magnetic", _wind_from_magnetic, true },
		{ "wind.speed.tas", _wind_tas, true },
		{ "glide-ratio", _glide_ratio, true },
		{ "total-energy-variometer", _total_energy_variometer, true },
	});

	_wind_computer.set_callback (std::bind (&PerformanceComputer::compute_wind, this));
	_wind_computer.observe ({
		&_speed_tas,
		&_speed_gs,
		&_track_lateral_true,
		&_orientation_heading_true,
		&_magnetic_declination,
	});

	_glide_ratio_computer.set_callback (std::bind (&PerformanceComputer::compute_glide_ratio, this));
	_glide_ratio_computer.observe ({
		&_speed_gs,
		&_vertical_speed,
	});

	_total_energy_variometer_computer.set_callback (std::bind (&PerformanceComputer::compute_total_energy_variometer, this));
	_total_energy_variometer_computer.observe ({
		&_aircraft_weight_g,
		&_altitude_amsl_std,
		&_speed_ias,
	});
}


void
PerformanceComputer::data_updated()
{
	Xefis::PropertyObserver* computers[] = {
		// Order is important:
		&_wind_computer,
		&_glide_ratio_computer,
		&_total_energy_variometer_computer,
	};

	for (Xefis::PropertyObserver* o: computers)
		o->data_updated (update_time());
}


void
PerformanceComputer::compute_wind()
{
	if (_speed_tas.valid() &&
		_speed_gs.valid() &&
		_track_lateral_true.valid() &&
		_orientation_heading_true.valid() &&
		_magnetic_declination.valid())
	{
		Time update_dt = _wind_computer.update_dt();

		Xefis::WindTriangle wt;
		wt.set_aircraft_tas (*_speed_tas);
		wt.set_aircraft_track (*_track_lateral_true);
		wt.set_aircraft_ground_speed (*_speed_gs);
		wt.set_aircraft_heading (*_orientation_heading_true);
		// TODO instead of smoothing result, smooth all input parameters
		wt.update();
		_wind_from_true.write (Xefis::floored_mod (1_deg * _wind_direction_smoother.process (wt.wind_direction().deg(), update_dt), 360_deg));
		_wind_from_magnetic.write (Xefis::true_to_magnetic (*_wind_from_true, *_magnetic_declination));
		_wind_tas.write (wt.wind_speed());
	}
	else
	{
		_wind_from_true.set_nil();
		_wind_from_magnetic.set_nil();
		_wind_tas.set_nil();
		_wind_direction_smoother.invalidate();
	}
}


void
PerformanceComputer::compute_glide_ratio()
{
	if (_speed_gs.valid() && _vertical_speed.valid())
	{
		Speed forward_speed = *_speed_gs;
		int ratio = (forward_speed > 1_kt)
			? Xefis::limit<int> (forward_speed / *_vertical_speed, -99, +99)
			: 0;
		_glide_ratio.write (ratio);
	}
	else
		_glide_ratio.set_nil();
}


void
PerformanceComputer::compute_total_energy_variometer()
{
	if (_total_energy_variometer.configured())
	{
		Time update_dt = _total_energy_variometer_computer.update_dt();

		if (_aircraft_weight_g.valid() &&
			_altitude_amsl_std.valid() &&
			_speed_ias.valid() &&
			*_speed_ias > _total_energy_variometer_min_ias)
		{
			if ((_total_energy_variometer_time += update_dt) > 0.1_s)
			{
				double const m = *_aircraft_weight_g;
				double const g = 9.81;
				double const v = (*_speed_ias).mps();
				double const Ep = m * g * (*_altitude_amsl_std).m();
				double const Ek = m * v * v * 0.5;
				double const total_energy = Ep + Ek;

				// If total energy was nil (invalid), reset _prev_total_energy
				// value to the current _total_energy:
				if (_total_energy_variometer.is_nil())
					_prev_total_energy = total_energy;

				double const energy_diff = total_energy - _prev_total_energy;
				Speed tev = (1_m * energy_diff / (m * g)) / _total_energy_variometer_time;

				_total_energy_variometer_time = 0_s;
				_total_energy_variometer.write (1_fpm * _total_energy_variometer_smoother.process (tev.fpm(), update_dt));

				_prev_total_energy = total_energy;
			}
		}
		else
		{
			_total_energy_variometer_time = 0_s;
			_total_energy_variometer.set_nil();
			_total_energy_variometer_smoother.invalidate();
		}
	}
}

