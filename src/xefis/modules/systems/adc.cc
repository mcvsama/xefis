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
#include <memory>
#include <map>

// Neutrino:
#include <neutrino/exception.h>
#include <neutrino/qt/qdom.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/aerodynamics/reynolds.h>
#include <xefis/support/airframe/airframe.h>
#include <xefis/support/earth/air/standard_atmosphere.h>
#include <xefis/support/nature/constants.h>
#include <xefis/utility/convergence.h>

// Local:
#include "adc.h"


AirDataComputer::AirDataComputer (std::unique_ptr<AirDataComputerIO> module_io, xf::Airframe* airframe, xf::Logger const& logger, std::string_view const& instance):
	Module (std::move (module_io), instance),
	_logger (logger.with_scope (std::string (kLoggerScope) + "#" + instance)),
	_airframe (airframe)
{
	_total_pressure_computer.set_callback (std::bind (&AirDataComputer::recover_total_pressure, this));
	_total_pressure_computer.observe ({
		&io.pressure_total,				// ← input
		&io.sensed_cas,					// ← input
		&io.pressure_static,			// ← input
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
		&io.pressure_static,			// ← input
		&io.pressure_use_std,			// ← input
		&io.pressure_qnh,				// ← input
	});

	_ias_computer.set_callback (std::bind (&AirDataComputer::compute_ias, this));
	_ias_computer.add_depending_smoothers ({
		&_speed_ias_smoother,
	});
	_ias_computer.observe ({
		&io.pressure_static,			// ← input
		&io.pressure_dynamic,			// ← _total_pressure_computer
		&io.recovered_pressure_total,	// ← input
		&io.air_density,				// ← _air_density_computer
	});

	_ias_lookahead_computer.set_callback (std::bind (&AirDataComputer::compute_ias_lookahead, this));
	_ias_lookahead_computer.add_depending_smoothers ({
		&_speed_ias_lookahead_i_smoother,
		&_speed_ias_lookahead_o_smoother,
	});
	_ias_lookahead_computer.observe ({
		&io.speed_ias,					// ← _ias_computer
	});

	_cas_computer.set_callback (std::bind (&AirDataComputer::compute_cas, this));
	_cas_computer.add_depending_smoothers ({
		&_speed_cas_smoother,
	});
	_cas_computer.observe ({
		&io.sensed_cas,					// ← input
		&io.pressure_static,			// ← input
		&io.recovered_pressure_total,	// ← _total_pressure_computer
	});

	_cas_lookahead_computer.set_callback (std::bind (&AirDataComputer::compute_cas_lookahead, this));
	_cas_lookahead_computer.add_depending_smoothers ({
		&_speed_cas_lookahead_i_smoother,
		&_speed_cas_lookahead_o_smoother,
	});
	_cas_lookahead_computer.observe ({
		&io.speed_cas,					// ← _cas_computer
	});

	_mach_computer.set_callback (std::bind (&AirDataComputer::compute_mach, this));
	_mach_computer.observe ({
		&io.pressure_dynamic,			// ← _total_pressure_computer
		&io.pressure_static,			// ← input
		&io.recovered_pressure_total,	// ← _total_pressure_computer
	});

	_sat_computer.set_callback (std::bind (&AirDataComputer::compute_sat_and_viscosity, this));
	_sat_computer.observe ({
		&_mach_computer,				// ← _mach_computer
		&io.total_air_temperature,		// ← input
	});

	_air_density_computer.set_callback (std::bind (&AirDataComputer::compute_air_density, this));
	_air_density_computer.observe ({
		&io.pressure_static,			// ← input
		&io.static_air_temperature,		// ← _sat_computer
	});

	_density_altitude_computer.set_callback (std::bind (&AirDataComputer::compute_density_altitude, this));
	_density_altitude_computer.observe ({
		&io.static_air_temperature,		// ← input
		&io.altitude_amsl,				// ← _altitude_computer
	});

	_speed_of_sound_computer.set_callback (std::bind (&AirDataComputer::compute_speed_of_sound, this));
	_speed_of_sound_computer.observe ({
		&io.static_air_temperature,		// ← _sat_computer
	});

	_tas_computer.set_callback (std::bind (&AirDataComputer::compute_tas, this));
	_tas_computer.observe ({
		&io.speed_cas,					// ← _cas_computer
		&io.density_altitude,			// ← _density_altitude_computer
		&io.altitude_amsl,				// ← _altitude_computer
	});

	_eas_computer.set_callback (std::bind (&AirDataComputer::compute_eas, this));
	_eas_computer.observe ({
		&io.speed_tas,					// ← _tas_computer
		&io.air_density,				// ← _air_density_computer
	});

	_vertical_speed_computer.set_minimum_dt (50_ms);
	_vertical_speed_computer.set_callback (std::bind (&AirDataComputer::compute_vertical_speed, this));
	_vertical_speed_computer.add_depending_smoothers ({
		&_vertical_speed_smoother,
	});
	_vertical_speed_computer.observe ({
		&io.altitude_amsl_std,			// ← _altitude_computer
	});

	_reynolds_computer.set_minimum_dt (1_s);
	_reynolds_computer.set_callback (std::bind (&AirDataComputer::compute_reynolds, this));
	_reynolds_computer.observe ({
		&io.speed_tas,					// ← _tas_computer
		&io.air_density,				// ← _air_density_computer
		&io.dynamic_viscosity,			// ← _sat_computer
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

	if (io.pressure_static &&
		(*io.pressure_use_std || io.pressure_qnh))
	{
		auto do_compute_altitude = [&](si::Pressure pressure_setting) -> si::Length {
			// Good for heights below tropopause (36 kft):
			double a = 6.8755856e-6;
			double b = 5.2558797;
			double p = io.pressure_static->in<si::InchOfMercury>();
			double p0 = pressure_setting.in<si::InchOfMercury>();
			return 1_ft * -(std::pow (p / p0, 1.0 / b) - 1.0) / a;
		};

		bool hide_alt_lookahead = false;
		si::Pressure pressure_setting;

		if (*io.pressure_use_std)
		{
			pressure_setting = xf::kStdAirPressure;

			if (!_prev_use_standard_pressure)
				hide_alt_lookahead = true;

			_prev_use_standard_pressure = true;
		}
		else
		{
			pressure_setting = *io.pressure_qnh;

			if (_prev_use_standard_pressure)
				hide_alt_lookahead = true;

			_prev_use_standard_pressure = false;
		}

		if (hide_alt_lookahead)
			_hide_alt_lookahead_until = update_time + _altitude_amsl_lookahead_o_smoother.smoothing_time() + _altitude_amsl_lookahead_i_smoother.smoothing_time();

		si::Length height = do_compute_altitude (pressure_setting);
		si::Length qnh_height = do_compute_altitude (*io.pressure_qnh);
		si::Length std_height = do_compute_altitude (xf::kStdAirPressure);

		io.altitude_amsl = _altitude_amsl_smoother (height, update_dt);
		io.altitude_amsl_qnh = _altitude_amsl_qnh_smoother (qnh_height, update_dt);
		io.altitude_amsl_std = _altitude_amsl_std_smoother (std_height, update_dt);
	}
	else
	{
		io.altitude_amsl = xf::nil;
		io.altitude_amsl_qnh = xf::nil;
		io.altitude_amsl_std = xf::nil;
		_altitude_amsl_smoother.invalidate();
		_altitude_amsl_qnh_smoother.invalidate();
		_altitude_amsl_std_smoother.invalidate();
	}

	if (io.altitude_amsl && update_time > _hide_alt_lookahead_until)
	{
		si::Length est = _altitude_amsl_estimator (_altitude_amsl_lookahead_i_smoother (*io.altitude_amsl, update_dt), update_dt);
		est = _altitude_amsl_lookahead_o_smoother (est, update_dt);
		io.altitude_amsl_lookahead = est;

		if (si::abs (est - *io.altitude_amsl) > 1_ft)
			_altitude_computer.touch();
	}
	else
	{
		io.altitude_amsl_lookahead = xf::nil;
		_altitude_amsl_estimator.invalidate();
		_altitude_amsl_lookahead_i_smoother.invalidate();
		_altitude_amsl_lookahead_o_smoother.invalidate();
	}
}


void
AirDataComputer::compute_air_density()
{
	// Also compute air density:
	if (io.pressure_static && io.static_air_temperature)
		io.air_density = *io.pressure_static / (xf::kDryAirSpecificConstant * *io.static_air_temperature);
	else
		io.air_density = xf::nil;
}


void
AirDataComputer::compute_density_altitude()
{
	if (io.static_air_temperature && io.altitude_amsl)
		io.density_altitude = xf::density_altitude (*io.altitude_amsl, *io.static_air_temperature);
	else
		io.density_altitude = xf::nil;
}


void
AirDataComputer::compute_ias()
{
	si::Time update_dt = _ias_computer.update_dt();

	if (io.pressure_static && io.pressure_dynamic && io.recovered_pressure_total && io.air_density)
	{
		si::Velocity const tmp_ias = sqrt (2 * *io.pressure_dynamic / *io.air_density);
		io.speed_ias = _speed_ias_smoother (tmp_ias, update_dt);
	}
	else
	{
		io.speed_ias = xf::nil;
		_speed_ias_smoother.invalidate();
	}

	_ias_in_valid_range = io.speed_ias && *io.ias_valid_minimum <= *io.speed_ias && *io.speed_ias <= *io.ias_valid_maximum;
}


void
AirDataComputer::compute_ias_lookahead()
{
	if (_ias_in_valid_range)
	{
		si::Time update_dt = _ias_lookahead_computer.update_dt();

		si::Velocity est = _speed_ias_estimator (_speed_ias_lookahead_i_smoother (*io.speed_ias, update_dt), update_dt);
		est = _speed_ias_lookahead_o_smoother (est, update_dt);
		io.speed_ias_lookahead = est;

		if (si::abs (est - *io.speed_ias) > 1.0_kt)
			_ias_lookahead_computer.touch();
	}
	else
	{
		io.speed_ias_lookahead = xf::nil;
		_speed_ias_estimator.invalidate();
		_speed_ias_lookahead_i_smoother.invalidate();
		_speed_ias_lookahead_o_smoother.invalidate();
	}
}


void
AirDataComputer::compute_cas()
{
	si::Time update_dt = _cas_computer.update_dt();

	if (io.pressure_static && io.recovered_pressure_total)
	{
		// Using formula from <http://en.wikipedia.org/wiki/Airspeed#Calibrated_airspeed>
		// Impact pressure (dynamic pressure) - difference between total pressure and static pressure:
		si::Pressure qc = *io.recovered_pressure_total - *io.pressure_static;

		// TODO For supersonic speeds: <https://en.wikipedia.org/wiki/Calibrated_airspeed#Calculation_from_impact_pressure>
		si::Velocity tmp_cas = xf::kStdSpeedOfSound * std::sqrt (5.0 * (std::pow (qc / xf::kStdAirPressure + 1.0, 2.0 / 7.0) - 1.0));
		io.speed_cas = _speed_cas_smoother (tmp_cas, update_dt);
	}
	else
	{
		io.speed_cas = xf::nil;
		_speed_cas_smoother.invalidate();
	}

	_cas_in_valid_range = io.speed_cas && *io.ias_valid_minimum <= *io.speed_cas && *io.speed_cas <= *io.ias_valid_maximum;
}


void
AirDataComputer::compute_cas_lookahead()
{
	if (_cas_in_valid_range)
	{
		si::Time update_dt = _cas_lookahead_computer.update_dt();

		si::Velocity est = _speed_cas_estimator (_speed_cas_lookahead_i_smoother (*io.speed_cas, update_dt), update_dt);
		est = _speed_cas_lookahead_o_smoother (est, update_dt);
		io.speed_cas_lookahead = est;

		if (si::abs (est - *io.speed_cas) > 1.0_kt)
			_cas_lookahead_computer.touch();
	}
	else
	{
		io.speed_cas_lookahead = xf::nil;
		_speed_cas_estimator.invalidate();
		_speed_cas_lookahead_i_smoother.invalidate();
		_speed_cas_lookahead_o_smoother.invalidate();
	}
}


void
AirDataComputer::compute_speed_of_sound()
{
	if (io.static_air_temperature)
		io.speed_sound = xf::speed_of_sound (*io.static_air_temperature);
	else
		io.speed_sound = xf::nil;
}


void
AirDataComputer::compute_tas()
{
	if (_ias_in_valid_range && io.altitude_amsl)
	{
		si::Velocity const tmp_ias = *io.speed_ias;

		if (io.density_altitude)
			io.speed_tas = xf::true_airspeed (*io.speed_ias, *io.density_altitude);
		else
			// Very simple equation for TAS when DA is unavailable:
			io.speed_tas = tmp_ias + 0.02 * tmp_ias * (*io.altitude_amsl / 1000_ft);
	}
	else
		io.speed_tas = xf::nil;
}


void
AirDataComputer::compute_eas()
{
	// TODO General formula for EAS: <https://en.wikipedia.org/wiki/Equivalent_airspeed>
	if (io.speed_tas && io.air_density)
	{
		auto rho = *io.air_density;
		io.speed_eas = *io.speed_tas * std::sqrt (rho / xf::kStdAirDensity);
	}
	else
		io.speed_eas = xf::nil;
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

	if (io.pressure_static && io.recovered_pressure_total && io.pressure_dynamic)
	{
		// Dynamic pressure behind the normal shock (no-one will ever fly above Mach 1, so doesn't really
		// matter where the sensor is installed). Use normal total pressure source:
		si::Pressure qc = *io.pressure_dynamic;
		// Static pressure:
		si::Pressure p = *io.pressure_static;

		// Compute sub-sonic Mach:
		double subsonic_mach = std::sqrt (5.0 * (std::pow (qc / p + 1, 2.0 / 7.0) - 1.0));

		if (subsonic_mach < 1.0)
			io.speed_mach = subsonic_mach;
		else
		{
			// If Mach turned out to be > 1, try to converge subsonic_mach from the second formula.
			// Limit iterations to 100.

			auto const initial_mach = io.speed_mach ? *io.speed_mach : subsonic_mach;

			io.speed_mach = xf::converge<double> (initial_mach, 1e-9, 100, [&](double M_it) {
				return 0.88128485 * std::sqrt ((qc / p + 1.0) * std::pow (1.0 - 1 / (7.0 * M_it * M_it), 2.5));
			});

			if (!io.speed_mach)
				_logger << "Mach number did not converge." << std::endl;
		}
	}
	else
		io.speed_mach = xf::nil;
}


void
AirDataComputer::compute_sat_and_viscosity()
{
	// SAT = TAT * (1 + ram_rise_factor * M^2)
	if (io.total_air_temperature && io.speed_mach)
	{
		double mach = *io.speed_mach;
		si::Temperature sat = *io.total_air_temperature / (1.0 + *io.ram_rise_factor * mach * mach);

		io.static_air_temperature = sat;
		// Unit is Poiseuville (Pascal * second):
		io.dynamic_viscosity = xf::dynamic_air_viscosity (sat);
	}
	else
	{
		io.static_air_temperature = xf::nil;
		io.dynamic_viscosity = xf::nil;
	}
}


void
AirDataComputer::compute_vertical_speed()
{
	// Vertical speed:
	if (io.altitude_amsl_std)
	{
		si::Time update_dt = _vertical_speed_computer.update_dt();

		// If previous climb-rate was invalid, use current STD pressure
		// as source for 'previous altitude' value:
		if (io.vertical_speed.is_nil())
			_prev_altitude_amsl = *io.altitude_amsl_std;

		si::Length alt_diff = *io.altitude_amsl_std - _prev_altitude_amsl;
		si::Velocity computed_vertical_speed = alt_diff / update_dt;
		_prev_altitude_amsl = *io.altitude_amsl_std;
		io.vertical_speed = _vertical_speed_smoother (computed_vertical_speed, update_dt);
	}
	else
	{
		io.vertical_speed = xf::nil;
		_vertical_speed_smoother.invalidate();
	}
}


void
AirDataComputer::compute_reynolds()
{
	if (_airframe &&
		io.speed_tas &&
		io.air_density &&
		io.dynamic_viscosity)
	{
		si::Length const travelled_length = _airframe->wings_chord();
		io.reynolds_number = xf::reynolds_number (*io.air_density, *io.speed_tas, travelled_length, *io.dynamic_viscosity);
	}
	else
		io.reynolds_number = xf::nil;
}


void
AirDataComputer::recover_total_pressure()
{
	if (io.pressure_total)
		io.recovered_pressure_total = io.pressure_total;
	else
	{
		// If we're using ready-made IAS sensor, we need to recover total pressure
		// from static pressure and TAS.
		if (*io.using_cas_sensor)
		{
			if (io.sensed_cas && io.pressure_static)
			{
				si::Pressure p = *io.pressure_static;
				// Formula from <http://en.wikipedia.org/wiki/Airspeed#Calibrated_airspeed>
				// solved for qc (dynamic (impact) pressure):
				double ia0 = *io.sensed_cas / xf::kStdSpeedOfSound;
				si::Pressure qc = xf::kStdAirPressure * (std::pow (ia0 * ia0 / 5.0 + 1.0, 7.0 / 2.0) - 1.0);
				io.recovered_pressure_total = qc + p;
			}
			else
				io.recovered_pressure_total = xf::nil;
		}
	}

	// Compute dynamic pressure:
	if (io.pressure_static)
		io.pressure_dynamic = *io.recovered_pressure_total - *io.pressure_static;
}

