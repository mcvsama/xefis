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
#include <xefis/airnav/density_altitude.h>
#include <xefis/airnav/sound_speed.h>

// Local:
#include "adc.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/adc", AirDataComputer);


AirDataComputer::AirDataComputer (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	_altitude_amsl_estimator.set_minimum_integration_time (0.2_s);
	_speed_ias_estimator.set_minimum_integration_time (0.2_s);

	parse_settings (config, {
		{ "ias.valid-minimum", _ias_valid_minimum, true },
		{ "ias.valid-maximum", _ias_valid_maximum, true },
	});

	parse_properties (config, {
		// Input:
		{ "settings.pressure.use-std", _pressure_use_std, false },
		{ "settings.pressure.qnh", _pressure_qnh, true },
		{ "pressure.static.serviceable", _pressure_static_serviceable, false },
		{ "pressure.static", _pressure_static, true },
		{ "ias.serviceable", _ias_serviceable, false },
		{ "ias", _ias, true },
		{ "static-air-temperature", _static_air_temperature, true },
		// Output:
		{ "altitude.amsl.serviceable", _altitude_amsl_serviceable, true },
		{ "altitude.amsl", _altitude_amsl, true },
		{ "altitude.amsl.lookahead", _altitude_amsl_lookahead, true },
		{ "altitude.amsl.qnh", _altitude_amsl_qnh, true },
		{ "altitude.amsl.std", _altitude_amsl_std, true },
		{ "density-altitude", _density_altitude, true },
		{ "speed.ias.serviceable", _speed_ias_serviceable, true },
		{ "speed.ias", _speed_ias, true },
		{ "speed.ias.lookahead", _speed_ias_lookahead, true },
		{ "speed.tas", _speed_tas, true },
		{ "speed.mach", _speed_mach, true },
		{ "speed.sound", _speed_sound, true },
		{ "vertical-speed.serviceable", _vertical_speed_serviceable, true },
		{ "vertical-speed", _vertical_speed, true },
	});

	_altitude_computer.set_minimum_dt (5_ms);
	_altitude_computer.set_callback (std::bind (&AirDataComputer::compute_altitude, this));
	_altitude_computer.add_depending_smoothers ({
		&_altitude_amsl_lookahead_i_smoother,
		&_altitude_amsl_lookahead_o_smoother,
		&_altitude_amsl_smoother,
		&_altitude_amsl_qnh_smoother,
		&_altitude_amsl_std_smoother,
	});
	_altitude_computer.observe ({
		&_pressure_static,
		&_pressure_use_std,
		&_pressure_qnh,
		&_pressure_static_serviceable,
	});

	_density_altitude_computer.set_callback (std::bind (&AirDataComputer::compute_density_altitude, this));
	_density_altitude_computer.observe ({
		&_static_air_temperature,
		&_altitude_amsl,
	});

	_ias_computer.set_callback (std::bind (&AirDataComputer::compute_ias, this));
	_ias_computer.add_depending_smoothers ({
		&_speed_ias_smoother
	});
	_ias_computer.observe ({
		&_ias,
		&_ias_serviceable,
	});

	_ias_lookahead_computer.set_callback (std::bind (&AirDataComputer::compute_ias_lookahead, this));
	_ias_lookahead_computer.add_depending_smoothers ({
		&_speed_ias_lookahead_i_smoother,
		&_speed_ias_lookahead_o_smoother,
	});
	_ias_lookahead_computer.observe ({
		&_ias,
	});

	_sound_speed_computer.set_callback (std::bind (&AirDataComputer::compute_sound_speed, this));
	_sound_speed_computer.observe ({
		&_static_air_temperature,
	});

	_tas_computer.set_callback (std::bind (&AirDataComputer::compute_tas, this));
	_tas_computer.observe ({
		&_speed_ias,
		&_density_altitude,
		&_altitude_amsl,
	});

	_mach_computer.set_callback (std::bind (&AirDataComputer::compute_mach, this));
	_mach_computer.observe ({
		&_speed_tas,
		&_speed_sound,
	});

	_vertical_speed_computer.set_minimum_dt (50_ms);
	_vertical_speed_computer.set_callback (std::bind (&AirDataComputer::compute_vertical_speed, this));
	_vertical_speed_computer.add_depending_smoothers ({
		&_vertical_speed_smoother,
	});
	_vertical_speed_computer.observe ({
		&_altitude_amsl_std,
	});
}


void
AirDataComputer::data_updated()
{
	Xefis::PropertyObserver* computers[] = {
		// Order is important:
		&_altitude_computer,
		&_density_altitude_computer,
		&_ias_computer,
		&_ias_lookahead_computer,
		&_sound_speed_computer,
		&_tas_computer,
		&_mach_computer,
		&_vertical_speed_computer,
	};

	for (Xefis::PropertyObserver* o: computers)
		o->data_updated (update_time());
}


void
AirDataComputer::compute_altitude()
{
	Time update_time = _altitude_computer.update_time();
	Time update_dt = _altitude_computer.update_dt();

	if (_pressure_static.valid() &&
		((_pressure_use_std.read (false)) || _pressure_qnh.valid()))
	{
		auto do_compute_altitude = [&](Pressure pressure_setting) -> Length {
			// Good for heights below tropopause (36 kft):
			double a = 6.8755856e-6;
			double b = 5.2558797;
			double p = (*_pressure_static).inHg();
			double p0 = pressure_setting.inHg();
			return 1_ft * -(std::pow (p / p0, 1.0 / b) - 1.0) / a;
		};

		bool hide_alt_lookahead = false;
		Pressure pressure_setting;
		if (_pressure_use_std.read (false))
		{
			pressure_setting = 29.92_inHg;
			if (!_prev_use_standard_pressure)
				hide_alt_lookahead = true;
			_prev_use_standard_pressure = true;
		}
		else
		{
			pressure_setting = *_pressure_qnh;
			if (_prev_use_standard_pressure)
				hide_alt_lookahead = true;
			_prev_use_standard_pressure = false;
		}

		if (hide_alt_lookahead)
			_hide_alt_lookahead_until = update_time + _altitude_amsl_lookahead_o_smoother.smoothing_time() + _altitude_amsl_lookahead_i_smoother.smoothing_time();

		Length height = do_compute_altitude (pressure_setting);
		Length qnh_height = do_compute_altitude (*_pressure_qnh);
		Length std_height = do_compute_altitude (29.92_inHg);

		_altitude_amsl.write (1_ft * _altitude_amsl_smoother.process (height.ft(), update_dt));
		_altitude_amsl_qnh.write (1_ft * _altitude_amsl_qnh_smoother.process (qnh_height.ft(), update_dt));
		_altitude_amsl_std.write (1_ft * _altitude_amsl_std_smoother.process (std_height.ft(), update_dt));
	}
	else
	{
		_altitude_amsl.set_nil();
		_altitude_amsl_qnh.set_nil();
		_altitude_amsl_std.set_nil();
		_altitude_amsl_smoother.invalidate();
		_altitude_amsl_qnh_smoother.invalidate();
		_altitude_amsl_std_smoother.invalidate();
	}

	if (_altitude_amsl.valid() && update_time > _hide_alt_lookahead_until)
	{
		double est = _altitude_amsl_estimator.process (_altitude_amsl_lookahead_i_smoother.process ((*_altitude_amsl).ft(), update_dt), update_dt);
		est = _altitude_amsl_lookahead_o_smoother.process (est, update_dt);
		_altitude_amsl_lookahead.write (1_ft * est);
	}
	else
	{
		_altitude_amsl_lookahead.set_nil();
		_altitude_amsl_estimator.invalidate();
		_altitude_amsl_lookahead_i_smoother.invalidate();
		_altitude_amsl_lookahead_o_smoother.invalidate();
	}

	_altitude_amsl_serviceable.copy (_pressure_static_serviceable);
}


void
AirDataComputer::compute_density_altitude()
{
	if (_static_air_temperature.valid() && _altitude_amsl.valid())
	{
		Xefis::DensityAltitude da;
		da.set_pressure_altitude (*_altitude_amsl);
		da.set_static_air_temperature (*_static_air_temperature);
		da.update();
		_density_altitude.write (da.density_altitude());
	}
	else
		_density_altitude.set_nil();
}


void
AirDataComputer::compute_ias()
{
	Time update_dt = _ias_computer.update_dt();

	if (_ias.valid())
		_speed_ias.write (1_kt * _speed_ias_smoother.process ((*_ias).kt(), update_dt));
	else
	{
		_speed_ias.set_nil();
		_speed_ias_smoother.invalidate();
	}

	_ias_in_valid_range = _speed_ias.valid() && _ias_valid_minimum <= *_speed_ias && *_speed_ias <= _ias_valid_maximum;

	_speed_ias_serviceable.copy (_ias_serviceable);
}


void
AirDataComputer::compute_ias_lookahead()
{
	if (_ias_in_valid_range)
	{
		Time update_dt = _ias_lookahead_computer.update_dt();

		double est = _speed_ias_estimator.process (_speed_ias_lookahead_i_smoother.process ((*_ias).kt(), update_dt), update_dt);
		est = _speed_ias_lookahead_o_smoother.process (est, update_dt);
		_speed_ias_lookahead.write (1_kt * est);
	}
	else
	{
		_speed_ias_lookahead.set_nil();
		_speed_ias_estimator.invalidate();
		_speed_ias_lookahead_i_smoother.invalidate();
		_speed_ias_lookahead_o_smoother.invalidate();
	}
}


void
AirDataComputer::compute_sound_speed()
{
	if (_static_air_temperature.valid())
	{
		Xefis::SoundSpeed ss;
		ss.set_static_air_temperature (*_static_air_temperature);
		ss.update();
		_speed_sound.write (ss.sound_speed());
	}
	else
		_speed_sound.set_nil();
}


void
AirDataComputer::compute_tas()
{
	if (_ias_in_valid_range && _altitude_amsl.valid())
	{
		Speed cas = *_speed_ias;

		if (_density_altitude.valid())
		{
			// This does not take into account air compressibility factor, so it's valid
			// for low speeds (mach < 0.3) and altitude below tropopause (36 kft):
			_speed_tas.write (cas / std::pow (1 - 6.8755856 * 1e-6 * (*_density_altitude).ft(), 2.127940));
		}
		else
			// Very simple equation for TAS, fix it to use air temperature someday:
			_speed_tas.write (cas + 0.02 * cas * (*_altitude_amsl / 1000_ft));
	}
	else
		_speed_tas.set_nil();
}


void
AirDataComputer::compute_mach()
{
	// The approximate speed of sound in dry (0% humidity) air:
	if (_speed_tas.valid() && _speed_sound.valid())
		_speed_mach.write (*_speed_tas / *_speed_sound);
	else
		_speed_mach.set_nil();
}


void
AirDataComputer::compute_vertical_speed()
{
	// Vertical speed:
	if (_altitude_amsl_std.valid())
	{
		Time update_dt = _vertical_speed_computer.update_dt();

		// If previous climb-rate was invalid, use current STD pressure
		// as source for 'previous altitude' value:
		if (_vertical_speed.is_nil())
			_prev_altitude_amsl = *_altitude_amsl_std;

		Length alt_diff = *_altitude_amsl_std - _prev_altitude_amsl;
		Speed computed_vertical_speed = alt_diff / update_dt;
		_prev_altitude_amsl = *_altitude_amsl_std;
		_vertical_speed.write (1_fpm * _vertical_speed_smoother.process (computed_vertical_speed.fpm(), update_dt));
	}
	else
	{
		_vertical_speed.set_nil();
		_vertical_speed_smoother.invalidate();
	}

	_vertical_speed_serviceable.write (_vertical_speed.valid());
}

