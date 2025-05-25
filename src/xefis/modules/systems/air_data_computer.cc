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
#include "air_data_computer.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/aerodynamics/reynolds_number.h>
#include <xefis/support/airframe/airframe.h>
#include <xefis/support/atmosphere/standard_atmosphere.h>
#include <xefis/support/nature/constants.h>
#include <xefis/utility/converger.h>

// Neutrino:
#include <neutrino/exception.h>
#include <neutrino/qt/qdom.h>

// Standard:
#include <cstddef>
#include <memory>
#include <map>


AirDataComputer::AirDataComputer (xf::ProcessingLoop& loop, xf::Airframe* airframe, xf::Logger const& logger, std::string_view const& instance):
	AirDataComputerIO (loop, instance),
	_logger (logger.with_context (std::string (kLoggerScope) + "#" + instance)),
	_airframe (airframe)
{
	_total_pressure_computer.set_callback (std::bind (&AirDataComputer::recover_total_pressure, this));
	_total_pressure_computer.observe ({
		&_io.pressure_total,			// ← input
		&_io.sensed_cas,				// ← input
		&_io.pressure_static,			// ← input
	});

	_altitude_amsl_estimator.set_minimum_integration_time (0.2_s);
	_speed_cas_estimator.set_minimum_integration_time (0.2_s);
	_speed_ias_estimator.set_minimum_integration_time (0.2_s);

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
		&_io.pressure_static,			// ← input
		&_io.pressure_use_std,			// ← input
		&_io.pressure_qnh,				// ← input
	});

	_ias_computer.set_callback (std::bind (&AirDataComputer::compute_ias, this));
	_ias_computer.add_depending_smoothers ({
		&_speed_ias_smoother,
	});
	_ias_computer.observe ({
		&_io.pressure_static,			// ← input
		&_io.pressure_dynamic,			// ← _total_pressure_computer
		&_io.recovered_pressure_total,	// ← input
		&_io.air_density,				// ← _air_density_computer
	});

	_ias_lookahead_computer.set_callback (std::bind (&AirDataComputer::compute_ias_lookahead, this));
	_ias_lookahead_computer.add_depending_smoothers ({
		&_speed_ias_lookahead_i_smoother,
		&_speed_ias_lookahead_o_smoother,
	});
	_ias_lookahead_computer.observe ({
		&_io.speed_ias,					// ← _ias_computer
	});

	_cas_computer.set_callback (std::bind (&AirDataComputer::compute_cas, this));
	_cas_computer.add_depending_smoothers ({
		&_speed_cas_smoother,
	});
	_cas_computer.observe ({
		&_io.sensed_cas,				// ← input
		&_io.pressure_static,			// ← input
		&_io.recovered_pressure_total,	// ← _total_pressure_computer
	});

	_cas_lookahead_computer.set_callback (std::bind (&AirDataComputer::compute_cas_lookahead, this));
	_cas_lookahead_computer.add_depending_smoothers ({
		&_speed_cas_lookahead_i_smoother,
		&_speed_cas_lookahead_o_smoother,
	});
	_cas_lookahead_computer.observe ({
		&_io.speed_cas,					// ← _cas_computer
	});

	_mach_computer.set_callback (std::bind (&AirDataComputer::compute_mach, this));
	_mach_computer.observe ({
		&_io.pressure_dynamic,			// ← _total_pressure_computer
		&_io.pressure_static,			// ← input
		&_io.recovered_pressure_total,	// ← _total_pressure_computer
	});

	_sat_computer.set_callback (std::bind (&AirDataComputer::compute_sat_and_viscosity, this));
	_sat_computer.observe ({
		&_mach_computer,				// ← _mach_computer
		&_io.total_air_temperature,		// ← input
	});

	_air_density_computer.set_callback (std::bind (&AirDataComputer::compute_air_density, this));
	_air_density_computer.observe ({
		&_io.pressure_static,			// ← input
		&_io.static_air_temperature,	// ← _sat_computer
	});

	_density_altitude_computer.set_callback (std::bind (&AirDataComputer::compute_density_altitude, this));
	_density_altitude_computer.observe ({
		&_io.static_air_temperature,	// ← input
		&_io.altitude_amsl,				// ← _altitude_computer
	});

	_speed_of_sound_computer.set_callback (std::bind (&AirDataComputer::compute_speed_of_sound, this));
	_speed_of_sound_computer.observe ({
		&_io.static_air_temperature,	// ← _sat_computer
	});

	_tas_computer.set_callback (std::bind (&AirDataComputer::compute_tas, this));
	_tas_computer.observe ({
		&_io.speed_cas,					// ← _cas_computer
		&_io.density_altitude,			// ← _density_altitude_computer
		&_io.altitude_amsl,				// ← _altitude_computer
	});

	_eas_computer.set_callback (std::bind (&AirDataComputer::compute_eas, this));
	_eas_computer.observe ({
		&_io.speed_tas,					// ← _tas_computer
		&_io.air_density,				// ← _air_density_computer
	});

	_vertical_speed_computer.set_minimum_dt (50_ms);
	_vertical_speed_computer.set_callback (std::bind (&AirDataComputer::compute_vertical_speed, this));
	_vertical_speed_computer.add_depending_smoothers ({
		&_vertical_speed_smoother,
	});
	_vertical_speed_computer.observe ({
		&_io.altitude_amsl_std,			// ← _altitude_computer
	});

	_reynolds_computer.set_minimum_dt (1_s);
	_reynolds_computer.set_callback (std::bind (&AirDataComputer::compute_reynolds, this));
	_reynolds_computer.observe ({
		&_io.speed_tas,					// ← _tas_computer
		&_io.air_density,				// ← _air_density_computer
		&_io.dynamic_viscosity,			// ← _sat_computer
	});
}


void
AirDataComputer::process (xf::Cycle const& cycle)
{
	// Order is important:
	xf::SocketObserver* computers[] = {
		// Computers that depend on input sockets only:
		&_total_pressure_computer,
		&_altitude_computer,
		// Computers depending on other computers:
		&_mach_computer,
		&_sat_computer,
		&_air_density_computer,
		&_ias_computer,
		&_ias_lookahead_computer,
		&_cas_computer,
		&_cas_lookahead_computer,
		&_density_altitude_computer,
		&_speed_of_sound_computer,
		&_tas_computer,
		&_eas_computer,
		&_vertical_speed_computer,
		&_reynolds_computer,
	};

	for (auto* o: computers)
		o->process (cycle.update_time());
}


void
AirDataComputer::compute_altitude()
{
	si::Time update_time = _altitude_computer.update_time();
	si::Time update_dt = _altitude_computer.update_dt();

	if (_io.pressure_static &&
		(*_io.pressure_use_std || _io.pressure_qnh))
	{
		auto do_compute_altitude = [&](si::Pressure pressure_setting) -> si::Length {
			// Good for heights below tropopause (36 kft):
			double a = 6.8755856e-6;
			double b = 5.2558797;
			double p = _io.pressure_static->in<si::InchOfMercury>();
			double p0 = pressure_setting.in<si::InchOfMercury>();
			return 1_ft * -(std::pow (p / p0, 1.0 / b) - 1.0) / a;
		};

		bool hide_alt_lookahead = false;
		si::Pressure pressure_setting;

		if (*_io.pressure_use_std)
		{
			pressure_setting = xf::kStdAirPressure;

			if (!_prev_use_standard_pressure)
				hide_alt_lookahead = true;

			_prev_use_standard_pressure = true;
		}
		else
		{
			pressure_setting = *_io.pressure_qnh;

			if (_prev_use_standard_pressure)
				hide_alt_lookahead = true;

			_prev_use_standard_pressure = false;
		}

		if (hide_alt_lookahead)
			_hide_alt_lookahead_until = update_time + _altitude_amsl_lookahead_o_smoother.smoothing_time() + _altitude_amsl_lookahead_i_smoother.smoothing_time();

		si::Length height = do_compute_altitude (pressure_setting);
		si::Length qnh_height = do_compute_altitude (*_io.pressure_qnh);
		si::Length std_height = do_compute_altitude (xf::kStdAirPressure);

		_io.altitude_amsl = _altitude_amsl_smoother (height, update_dt);
		_io.altitude_amsl_qnh = _altitude_amsl_qnh_smoother (qnh_height, update_dt);
		_io.altitude_amsl_std = _altitude_amsl_std_smoother (std_height, update_dt);
	}
	else
	{
		_io.altitude_amsl = xf::nil;
		_io.altitude_amsl_qnh = xf::nil;
		_io.altitude_amsl_std = xf::nil;
		_altitude_amsl_smoother.invalidate();
		_altitude_amsl_qnh_smoother.invalidate();
		_altitude_amsl_std_smoother.invalidate();
	}

	if (_io.altitude_amsl && update_time > _hide_alt_lookahead_until)
	{
		si::Length est = _altitude_amsl_estimator (_altitude_amsl_lookahead_i_smoother (*_io.altitude_amsl, update_dt), update_dt);
		est = _altitude_amsl_lookahead_o_smoother (est, update_dt);
		_io.altitude_amsl_lookahead = est;

		if (si::abs (est - *_io.altitude_amsl) > 1_ft)
			_altitude_computer.touch();
	}
	else
	{
		_io.altitude_amsl_lookahead = xf::nil;
		_altitude_amsl_estimator.invalidate();
		_altitude_amsl_lookahead_i_smoother.invalidate();
		_altitude_amsl_lookahead_o_smoother.invalidate();
	}
}


void
AirDataComputer::compute_air_density()
{
	// Also compute air density:
	if (_io.pressure_static && _io.static_air_temperature)
		_io.air_density = *_io.pressure_static / (xf::kDryAirSpecificConstant * *_io.static_air_temperature);
	else
		_io.air_density = xf::nil;
}


void
AirDataComputer::compute_density_altitude()
{
	if (_io.static_air_temperature && _io.altitude_amsl)
		_io.density_altitude = xf::density_altitude (*_io.altitude_amsl, *_io.static_air_temperature);
	else
		_io.density_altitude = xf::nil;
}


void
AirDataComputer::compute_ias()
{
	si::Time update_dt = _ias_computer.update_dt();

	if (_io.pressure_static && _io.pressure_dynamic && _io.recovered_pressure_total && _io.air_density)
	{
		si::Velocity const tmp_ias = sqrt (2 * *_io.pressure_dynamic / *_io.air_density);
		_io.speed_ias = _speed_ias_smoother (tmp_ias, update_dt);
	}
	else
	{
		_io.speed_ias = xf::nil;
		_speed_ias_smoother.invalidate();
	}

	_ias_in_valid_range = _io.speed_ias && *_io.ias_valid_minimum <= *_io.speed_ias && *_io.speed_ias <= *_io.ias_valid_maximum;
}


void
AirDataComputer::compute_ias_lookahead()
{
	if (_ias_in_valid_range)
	{
		si::Time update_dt = _ias_lookahead_computer.update_dt();

		si::Velocity est = _speed_ias_estimator (_speed_ias_lookahead_i_smoother (*_io.speed_ias, update_dt), update_dt);
		est = _speed_ias_lookahead_o_smoother (est, update_dt);
		_io.speed_ias_lookahead = est;

		if (si::abs (est - *_io.speed_ias) > 1.0_kt)
			_ias_lookahead_computer.touch();
	}
	else
	{
		_io.speed_ias_lookahead = xf::nil;
		_speed_ias_estimator.invalidate();
		_speed_ias_lookahead_i_smoother.invalidate();
		_speed_ias_lookahead_o_smoother.invalidate();
	}
}


void
AirDataComputer::compute_cas()
{
	si::Time update_dt = _cas_computer.update_dt();

	if (_io.pressure_static && _io.recovered_pressure_total)
	{
		// Using formula from <http://en.wikipedia.org/wiki/Airspeed#Calibrated_airspeed>
		// Impact pressure (dynamic pressure) - difference between total pressure and static pressure:
		si::Pressure qc = *_io.recovered_pressure_total - *_io.pressure_static;

		// TODO For supersonic speeds: <https://en.wikipedia.org/wiki/Calibrated_airspeed#Calculation_from_impact_pressure>
		si::Velocity tmp_cas = xf::kStdSpeedOfSound * std::sqrt (5.0 * (std::pow (qc / xf::kStdAirPressure + 1.0, 2.0 / 7.0) - 1.0));
		_io.speed_cas = _speed_cas_smoother (tmp_cas, update_dt);
	}
	else
	{
		_io.speed_cas = xf::nil;
		_speed_cas_smoother.invalidate();
	}

	_cas_in_valid_range = _io.speed_cas && *_io.ias_valid_minimum <= *_io.speed_cas && *_io.speed_cas <= *_io.ias_valid_maximum;
}


void
AirDataComputer::compute_cas_lookahead()
{
	if (_cas_in_valid_range)
	{
		si::Time update_dt = _cas_lookahead_computer.update_dt();

		si::Velocity est = _speed_cas_estimator (_speed_cas_lookahead_i_smoother (*_io.speed_cas, update_dt), update_dt);
		est = _speed_cas_lookahead_o_smoother (est, update_dt);
		_io.speed_cas_lookahead = est;

		if (si::abs (est - *_io.speed_cas) > 1.0_kt)
			_cas_lookahead_computer.touch();
	}
	else
	{
		_io.speed_cas_lookahead = xf::nil;
		_speed_cas_estimator.invalidate();
		_speed_cas_lookahead_i_smoother.invalidate();
		_speed_cas_lookahead_o_smoother.invalidate();
	}
}


void
AirDataComputer::compute_speed_of_sound()
{
	if (_io.static_air_temperature)
		_io.speed_sound = xf::speed_of_sound (*_io.static_air_temperature);
	else
		_io.speed_sound = xf::nil;
}


void
AirDataComputer::compute_tas()
{
	if (_ias_in_valid_range && _io.altitude_amsl)
	{
		si::Velocity const tmp_ias = *_io.speed_ias;

		if (_io.density_altitude)
			_io.speed_tas = xf::true_airspeed (*_io.speed_ias, *_io.density_altitude);
		else
			// Very simple equation for TAS when DA is unavailable:
			_io.speed_tas = tmp_ias + 0.02 * tmp_ias * (*_io.altitude_amsl / 1000_ft);
	}
	else
		_io.speed_tas = xf::nil;
}


void
AirDataComputer::compute_eas()
{
	// TODO General formula for EAS: <https://en.wikipedia.org/wiki/Equivalent_airspeed>
	if (_io.speed_tas && _io.air_density)
	{
		auto rho = *_io.air_density;
		_io.speed_eas = *_io.speed_tas * std::sqrt (rho / xf::kStdAirDensity);
	}
	else
		_io.speed_eas = xf::nil;
}


void
AirDataComputer::compute_mach()
{
	// The approximate speed of sound in dry (0% humidity) air is TAS / sound-of-speed.
	// We don't want to use either, because they depend on SAT, and SAT is calculated
	// using Mach number, so we'd have a cycle.
	//
	// Instead use algorithm described here:
	// <http://en.wikipedia.org/wiki/Mach_number#Calculating_Mach_Number_from_Pitot_Tube_Pressure>

	if (_io.pressure_static && _io.recovered_pressure_total && _io.pressure_dynamic)
	{
		// Dynamic pressure behind the normal shock (no-one will ever fly above Mach 1, so doesn't really
		// matter where the sensor is installed). Use normal total pressure source:
		si::Pressure qc = *_io.pressure_dynamic;
		// Static pressure:
		si::Pressure p = *_io.pressure_static;

		// Compute sub-sonic Mach:
		double subsonic_mach = std::sqrt (5.0 * (std::pow (qc / p + 1, 2.0 / 7.0) - 1.0));

		if (subsonic_mach < 1.0)
			_io.speed_mach = subsonic_mach;
		else
		{
			// If Mach turned out to be > 1, try to converge subsonic_mach from the second formula.
			// Limit iterations to 100.

			auto const initial_mach = _io.speed_mach ? *_io.speed_mach : subsonic_mach;

			_io.speed_mach = xf::converge<double> (initial_mach, 1e-9, 100, [&](double M_it) {
				return 0.88128485 * std::sqrt ((qc / p + 1.0) * std::pow (1.0 - 1 / (7.0 * M_it * M_it), 2.5));
			});

			if (!_io.speed_mach)
				_logger << "Mach number did not converge." << std::endl;
		}
	}
	else
		_io.speed_mach = xf::nil;
}


void
AirDataComputer::compute_sat_and_viscosity()
{
	// SAT = TAT * (1 + ram_rise_factor * M^2)
	if (_io.total_air_temperature && _io.speed_mach)
	{
		double mach = *_io.speed_mach;
		si::Temperature sat = *_io.total_air_temperature / (1.0 + *_io.ram_rise_factor * mach * mach);

		_io.static_air_temperature = sat;
		// Unit is Poiseuville (Pascal * second):
		_io.dynamic_viscosity = xf::dynamic_air_viscosity (sat);
	}
	else
	{
		_io.static_air_temperature = xf::nil;
		_io.dynamic_viscosity = xf::nil;
	}
}


void
AirDataComputer::compute_vertical_speed()
{
	// Vertical speed:
	if (_io.altitude_amsl_std)
	{
		si::Time update_dt = _vertical_speed_computer.update_dt();

		// If previous climb-rate was invalid, use current STD pressure
		// as source for 'previous altitude' value:
		if (_io.vertical_speed.is_nil())
			_prev_altitude_amsl = *_io.altitude_amsl_std;

		si::Length alt_diff = *_io.altitude_amsl_std - _prev_altitude_amsl;
		si::Velocity computed_vertical_speed = alt_diff / update_dt;
		_prev_altitude_amsl = *_io.altitude_amsl_std;
		_io.vertical_speed = _vertical_speed_smoother (computed_vertical_speed, update_dt);
	}
	else
	{
		_io.vertical_speed = xf::nil;
		_vertical_speed_smoother.invalidate();
	}
}


void
AirDataComputer::compute_reynolds()
{
	if (_airframe &&
		_io.speed_tas &&
		_io.air_density &&
		_io.dynamic_viscosity)
	{
		si::Length const travelled_length = _airframe->wings_chord();
		_io.reynolds_number = xf::reynolds_number (*_io.air_density, *_io.speed_tas, travelled_length, *_io.dynamic_viscosity);
	}
	else
		_io.reynolds_number = xf::nil;
}


void
AirDataComputer::recover_total_pressure()
{
	if (_io.pressure_total)
		_io.recovered_pressure_total = _io.pressure_total;
	else
	{
		// If we're using ready-made IAS sensor, we need to recover total pressure
		// from static pressure and TAS.
		if (*_io.using_cas_sensor)
		{
			if (_io.sensed_cas && _io.pressure_static)
			{
				si::Pressure p = *_io.pressure_static;
				// Formula from <http://en.wikipedia.org/wiki/Airspeed#Calibrated_airspeed>
				// solved for qc (dynamic (impact) pressure):
				double ia0 = *_io.sensed_cas / xf::kStdSpeedOfSound;
				si::Pressure qc = xf::kStdAirPressure * (std::pow (ia0 * ia0 / 5.0 + 1.0, 7.0 / 2.0) - 1.0);
				_io.recovered_pressure_total = qc + p;
			}
			else
				_io.recovered_pressure_total = xf::nil;
		}
	}

	// Compute dynamic pressure:
	if (_io.pressure_static)
		_io.pressure_dynamic = *_io.recovered_pressure_total - *_io.pressure_static;
}

