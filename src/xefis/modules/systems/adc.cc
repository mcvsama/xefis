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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/config/exception.h>
#include <xefis/core/module_manager.h>
#include <xefis/support/air/air.h>
#include <xefis/support/airframe/airframe.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/convergence.h>

// Local:
#include "adc.h"


AirDataComputer::AirDataComputer (xf::Airframe* airframe, std::string const& instance):
	Module (instance),
	_airframe (airframe)
{
	_altitude_amsl_estimator.set_minimum_integration_time (0.2_s);
	_speed_ias_estimator.set_minimum_integration_time (0.2_s);

	// Map of temperature <-> dynamic viscosity taken from
	// <http://www.engineeringtoolbox.com/air-absolute-kinematic-viscosity-d_601.html>
	std::map<si::Temperature, si::DynamicViscosity> const temperature_to_dynamic_viscosity_map {
		{  -40_degF, 157.591e-7_Pas },
		{  -20_degF, 159.986e-7_Pas },
		{    0_degF, 157.591e-7_Pas },
		{   10_degF, 164.776e-7_Pas },
		{   20_degF, 167.650e-7_Pas },
		{   30_degF, 171.482e-7_Pas },
		{   40_degF, 172.440e-7_Pas },
		{   50_degF, 176.272e-7_Pas },
		{   60_degF, 179.625e-7_Pas },
		{   70_degF, 182.978e-7_Pas },
		{   80_degF, 184.894e-7_Pas },
		{   90_degF, 186.810e-7_Pas },
		{  100_degF, 188.726e-7_Pas },
		{  120_degF, 192.558e-7_Pas },
		{  140_degF, 197.827e-7_Pas },
		{  160_degF, 202.138e-7_Pas },
		{  180_degF, 207.886e-7_Pas },
		{  200_degF, 215.071e-7_Pas },
		{  300_degF, 238.063e-7_Pas },
		{  400_degF, 250.996e-7_Pas },
		{  500_degF, 277.820e-7_Pas },
		{  750_degF, 326.199e-7_Pas },
		{ 1000_degF, 376.015e-7_Pas },
		{ 1500_degF, 455.050e-7_Pas },
	};
	_temperature_to_dynamic_viscosity =
		std::make_unique<xf::Datatable2D<si::Temperature, si::DynamicViscosity>> (std::move (temperature_to_dynamic_viscosity_map));

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
		&input_pressure_static,
		&input_pressure_use_std,
		&input_pressure_qnh,
		&input_pressure_static_serviceable,
	});

	_ias_computer.set_callback (std::bind (&AirDataComputer::compute_ias, this));
	_ias_computer.add_depending_smoothers ({
		&_speed_ias_smoother,
	});
	_ias_computer.observe ({
		&input_ias,
		&input_ias_serviceable,
	});

	_ias_lookahead_computer.set_callback (std::bind (&AirDataComputer::compute_ias_lookahead, this));
	_ias_lookahead_computer.add_depending_smoothers ({
		&_speed_ias_lookahead_i_smoother,
		&_speed_ias_lookahead_o_smoother,
	});
	_ias_lookahead_computer.observe ({
		&input_ias,
	});

	_mach_computer.set_callback (std::bind (&AirDataComputer::compute_mach, this));
	_mach_computer.observe ({
		&input_pressure_static,
		&input_pressure_total,
	});

	_sat_computer.set_callback (std::bind (&AirDataComputer::compute_sat_and_viscosity, this));
	_sat_computer.observe ({
		&_mach_computer,
		&input_total_air_temperature,
	});

	_density_altitude_computer.set_callback (std::bind (&AirDataComputer::compute_density_altitude, this));
	_density_altitude_computer.observe ({
		&output_static_air_temperature,
		&output_altitude_amsl,
	});

	_sound_speed_computer.set_callback (std::bind (&AirDataComputer::compute_sound_speed, this));
	_sound_speed_computer.observe ({
		&output_static_air_temperature,
	});

	_tas_computer.set_callback (std::bind (&AirDataComputer::compute_tas, this));
	_tas_computer.observe ({
		&output_speed_ias,
		&output_density_altitude,
		&output_altitude_amsl,
	});

	_vertical_speed_computer.set_minimum_dt (50_ms);
	_vertical_speed_computer.set_callback (std::bind (&AirDataComputer::compute_vertical_speed, this));
	_vertical_speed_computer.add_depending_smoothers ({
		&_vertical_speed_smoother,
	});
	_vertical_speed_computer.observe ({
		&output_altitude_amsl_std,
		&output_altitude_amsl_serviceable,
	});

	_reynolds_computer.set_minimum_dt (1_s);
	_reynolds_computer.set_callback (std::bind (&AirDataComputer::compute_reynolds, this));
	_reynolds_computer.observe ({
		&output_speed_tas,
		&output_air_density_static,
		&output_dynamic_viscosity,
	});
}


void
AirDataComputer::process (x2::Cycle const& cycle)
{
	x2::PropertyObserver* computers[] = {
		// Order is important:
		&_altitude_computer,
		&_ias_computer,
		&_ias_lookahead_computer,
		&_mach_computer,
		&_sat_computer,
		&_density_altitude_computer,
		&_sound_speed_computer,
		&_tas_computer,
		&_vertical_speed_computer,
	};

	for (auto* o: computers)
		o->process (cycle.update_time());
}


void
AirDataComputer::compute_altitude()
{
	si::Time update_time = _altitude_computer.update_time();
	si::Time update_dt = _altitude_computer.update_dt();

	if (input_pressure_static &&
		(*input_pressure_use_std || input_pressure_qnh))
	{
		auto do_compute_altitude = [&](si::Pressure pressure_setting) -> si::Length {
			// Good for heights below tropopause (36 kft):
			double a = 6.8755856e-6;
			double b = 5.2558797;
			double p = input_pressure_static->quantity<si::InchOfMercury>();
			double p0 = pressure_setting.quantity<si::InchOfMercury>();
			return 1_ft * -(std::pow (p / p0, 1.0 / b) - 1.0) / a;
		};

		bool hide_alt_lookahead = false;
		si::Pressure pressure_setting;

		if (*input_pressure_use_std)
		{
			pressure_setting = 29.92_inHg;
			if (!_prev_use_standard_pressure)
				hide_alt_lookahead = true;
			_prev_use_standard_pressure = true;
		}
		else
		{
			pressure_setting = *input_pressure_qnh;
			if (_prev_use_standard_pressure)
				hide_alt_lookahead = true;
			_prev_use_standard_pressure = false;
		}

		if (hide_alt_lookahead)
			_hide_alt_lookahead_until = update_time + _altitude_amsl_lookahead_o_smoother.smoothing_time() + _altitude_amsl_lookahead_i_smoother.smoothing_time();

		si::Length height = do_compute_altitude (pressure_setting);
		si::Length qnh_height = do_compute_altitude (*input_pressure_qnh);
		si::Length std_height = do_compute_altitude (29.92_inHg);

		output_altitude_amsl = _altitude_amsl_smoother (height, update_dt);
		output_altitude_amsl_qnh = _altitude_amsl_qnh_smoother (qnh_height, update_dt);
		output_altitude_amsl_std = _altitude_amsl_std_smoother (std_height, update_dt);
	}
	else
	{
		output_altitude_amsl.set_nil();
		output_altitude_amsl_qnh.set_nil();
		output_altitude_amsl_std.set_nil();
		_altitude_amsl_smoother.invalidate();
		_altitude_amsl_qnh_smoother.invalidate();
		_altitude_amsl_std_smoother.invalidate();
	}

	if (output_altitude_amsl && update_time > _hide_alt_lookahead_until)
	{
		si::Length est = _altitude_amsl_estimator (_altitude_amsl_lookahead_i_smoother (*output_altitude_amsl, update_dt), update_dt);
		est = _altitude_amsl_lookahead_o_smoother (est, update_dt);
		output_altitude_amsl_lookahead = est;

		if (si::abs (est - *output_altitude_amsl) > 1_ft)
			_altitude_computer.touch();
	}
	else
	{
		output_altitude_amsl_lookahead.set_nil();
		_altitude_amsl_estimator.invalidate();
		_altitude_amsl_lookahead_i_smoother.invalidate();
		_altitude_amsl_lookahead_o_smoother.invalidate();
	}

	output_altitude_amsl_serviceable = input_pressure_static_serviceable;
}


void
AirDataComputer::compute_density_altitude()
{
	if (output_static_air_temperature && output_altitude_amsl)
		output_density_altitude = xf::compute_density_altitude (*output_altitude_amsl, *output_static_air_temperature);
	else
		output_density_altitude.set_nil();

	// Also compute air density:
	if (input_pressure_static && output_static_air_temperature)
	{
		si::SpecificHeatCapacity dry_air_specific_constant { 287.058 };
		output_air_density_static = *input_pressure_static / (dry_air_specific_constant * *output_static_air_temperature);
	}
	else
		output_air_density_static.set_nil();
}


void
AirDataComputer::compute_ias()
{
	si::Time update_dt = _ias_computer.update_dt();
	// Sound speed in STD sea level:
	si::Velocity const a0 = 661.4788_kt;
	// STD sea level pressure:
	si::Pressure const p0 = 29.92126_inHg;

	// If we're using ready-made IAS sensor, we need to recover total pressure
	// from static pressure and TAS.
	if (*setting_using_ias_sensor)
	{
		if (input_ias && input_pressure_static)
		{
			si::Pressure p = *input_pressure_static;
			// Formula from <http://en.wikipedia.org/wiki/Airspeed#Calibrated_airspeed>
			// solved for qc (dynamic (impact) pressure):
			double ia0 = *input_ias / a0;
			si::Pressure qc = p0 * (std::pow (ia0 * ia0 / 5.0 + 1.0, 7.0 / 2.0) - 1.0);
			input_pressure_total = qc + p;
		}
		else
			input_pressure_total.set_nil();
	}

	if (input_pressure_static && input_pressure_total)
	{
		// Compute dynamic pressure:
		output_pressure_dynamic = *input_pressure_total - *input_pressure_static;

		// Using formula from <http://en.wikipedia.org/wiki/Airspeed#Calibrated_airspeed>
		// Impact pressure (dynamic pressure) - difference between total pressure and static pressure:
		si::Pressure qc = *input_pressure_total - *input_pressure_static;

		si::Velocity tmp_ias = a0 * std::sqrt (5.0 * (std::pow (qc / p0 + 1.0, 2.0 / 7.0) - 1.0));
		output_speed_ias = _speed_ias_smoother (tmp_ias, update_dt);
	}
	else
	{
		output_speed_ias.set_nil();
		_speed_ias_smoother.invalidate();
	}

	_ias_in_valid_range = output_speed_ias && *setting_ias_valid_minimum <= *output_speed_ias && *output_speed_ias <= *setting_ias_valid_maximum;

	output_speed_ias_serviceable = input_ias_serviceable;
}


void
AirDataComputer::compute_ias_lookahead()
{
	if (_ias_in_valid_range)
	{
		si::Time update_dt = _ias_lookahead_computer.update_dt();

		si::Velocity est = _speed_ias_estimator (_speed_ias_lookahead_i_smoother (*input_ias, update_dt), update_dt);
		est = _speed_ias_lookahead_o_smoother (est, update_dt);
		output_speed_ias_lookahead = est;

		if (si::abs (est - *input_ias) > 1.0_kt)
			_ias_lookahead_computer.touch();
	}
	else
	{
		output_speed_ias_lookahead.set_nil();
		_speed_ias_estimator.invalidate();
		_speed_ias_lookahead_i_smoother.invalidate();
		_speed_ias_lookahead_o_smoother.invalidate();
	}
}


void
AirDataComputer::compute_sound_speed()
{
	if (output_static_air_temperature)
		output_speed_sound = xf::compute_sound_speed (*output_static_air_temperature);
	else
		output_speed_sound.set_nil();
}


void
AirDataComputer::compute_tas()
{
	if (_ias_in_valid_range && output_altitude_amsl)
	{
		si::Velocity tmp_ias = *output_speed_ias;

		if (output_density_altitude)
			output_speed_tas = xf::compute_true_airspeed (*output_speed_ias, *output_density_altitude);
		else
			// Very simple equation for TAS when DA is unavailable:
			output_speed_tas = tmp_ias + 0.02 * tmp_ias * (*output_altitude_amsl / 1000_ft);
	}
	else
		output_speed_tas.set_nil();
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

	if (input_pressure_static && input_pressure_total && output_pressure_dynamic)
	{
		// Dynamic pressure behind the normal shock (no-one will ever fly above Mach 1, so doesn't really
		// matter where the sensor is installed). Use normal total pressure source:
		si::Pressure qc = *output_pressure_dynamic;
		// Static pressure:
		si::Pressure p = *input_pressure_static;

		// Compute sub-sonic Mach:
		double subsonic_mach = std::sqrt (5.0 * (std::pow (qc / p + 1, 2.0 / 7.0) - 1.0));

		if (subsonic_mach < 1.0)
			output_speed_mach = subsonic_mach;
		else
		{
			// If Mach turned out to be > 1, try to converge subsonic_mach from the second formula.
			// Limit iterations to 100.
			output_speed_mach = xf::converge<double> (subsonic_mach, 1e-9, 100, [&](double M_it) {
				return 0.88128485 * std::sqrt ((qc / p + 1.0) * std::pow (1.0 - 1 / (7.0 * M_it * M_it), 2.5));
			});

			if (!output_speed_mach)
				log() << "Mach number did not converge." << std::endl;
		}
	}
	else
		output_speed_mach.set_nil();
}


void
AirDataComputer::compute_sat_and_viscosity()
{
	// SAT = TAT * (1 + ram_rise_factor * M^2)
	if (input_total_air_temperature && output_speed_mach)
	{
		double mach = *output_speed_mach;
		si::Temperature sat = *input_total_air_temperature / (1.0 + *setting_ram_rise_factor * mach * mach);

		output_static_air_temperature = sat;
		// Unit is Poiseuville (Pascal * second):
		output_dynamic_viscosity = _temperature_to_dynamic_viscosity->extrapolated_value (sat);
	}
	else
	{
		output_static_air_temperature.set_nil();
		output_dynamic_viscosity.set_nil();
	}
}


void
AirDataComputer::compute_vertical_speed()
{
	// Vertical speed:
	if (output_altitude_amsl_std)
	{
		si::Time update_dt = _vertical_speed_computer.update_dt();

		// If previous climb-rate was invalid, use current STD pressure
		// as source for 'previous altitude' value:
		if (output_vertical_speed.is_nil())
			_prev_altitude_amsl = *output_altitude_amsl_std;

		si::Length alt_diff = *output_altitude_amsl_std - _prev_altitude_amsl;
		si::Velocity computed_vertical_speed = alt_diff / update_dt;
		_prev_altitude_amsl = *output_altitude_amsl_std;
		output_vertical_speed = _vertical_speed_smoother (computed_vertical_speed, update_dt);
	}
	else
	{
		output_vertical_speed.set_nil();
		_vertical_speed_smoother.invalidate();
	}

	output_vertical_speed_serviceable = output_altitude_amsl_serviceable;
}


void
AirDataComputer::compute_reynolds()
{
	if (_airframe &&
		output_speed_tas &&
		output_air_density_static &&
		output_dynamic_viscosity)
	{
		si::Length const travelled_length = _airframe->wings_chord();
		output_reynolds_number = (*output_air_density_static) * (*output_speed_tas) * travelled_length / (*output_dynamic_viscosity);
	}
	else
		output_reynolds_number.set_nil();
}

