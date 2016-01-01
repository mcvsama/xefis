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
#include <xefis/config/exception.h>
#include <xefis/core/module_manager.h>
#include <xefis/airframe/airframe.h>
#include <xefis/airframe/lift.h>
#include <xefis/airframe/types.h>
#include <xefis/support/air/wind_triangle.h>
#include <xefis/support/air/air.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "pc.h"


XEFIS_REGISTER_MODULE_CLASS ("systems/pc", PerformanceComputer);


PerformanceComputer::PerformanceComputer (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	_wind_direction_smoother.set_winding ({ 0.0, 360.0 });

	parse_settings (config, {
		{ "total-energy-variometer.minimum-ias", _total_energy_variometer_min_ias, true },
	});

	parse_properties (config, {
		// Input:
		{ "input.speed.ias", _speed_ias, true },
		{ "input.speed.tas", _speed_tas, true },
		{ "input.speed.gs", _speed_gs, true },
		{ "input.vertical-speed", _vertical_speed, true },
		{ "input.altitude.amsl.std", _altitude_amsl_std, true },
		{ "input.track.lateral.true", _track_lateral_true, true },
		{ "input.orientation.heading.true", _orientation_heading_true, true },
		{ "input.magnetic.declination", _magnetic_declination, true },
		{ "input.density-altitude", _density_altitude, true },
		{ "input.air-density-static", _input_air_density_static, true },
		{ "input.aircraft-weight", _input_aircraft_weight, true },
		{ "input.flaps-angle", _input_flaps_angle, true },
		{ "input.spoilers-angle", _input_spoilers_angle, true },
		{ "input.aoa.alpha", _input_aoa_alpha, true },
		{ "input.load", _input_load, true },
		{ "input.bank-angle", _input_bank_angle, true },
		// Output:
		{ "output.wind.from.true", _wind_from_true, true },
		{ "output.wind.from.magnetic", _wind_from_magnetic, true },
		{ "output.wind.speed.tas", _wind_tas, true },
		{ "output.glide-ratio", _glide_ratio, true },
		{ "output.glide-ratio.string", _glide_ratio_string, false },
		{ "output.total-energy-variometer", _total_energy_variometer, true },
		{ "output.v-s", _v_s, true },
		{ "output.v-s.0_deg", _v_s_0_deg, true },
		{ "output.v-s.5_deg", _v_s_5_deg, true },
		{ "output.v-s.30_deg", _v_s_30_deg, true },
		{ "output.v-r", _v_r, true },
		{ "output.v-a", _v_a, true },
		{ "output.v-approach", _v_approach, true },
		{ "output.v-1", _v_1, true },
		{ "output.v-bg", _v_bg, true },
		{ "output.v-br", _v_br, true },
		{ "output.v-md", _v_md, true },
		{ "output.v-be", _v_be, true },
		{ "output.v-x", _v_x, true },
		{ "output.v-y", _v_y, true },
		{ "output.critical-aoa", _critical_aoa, true },
		{ "output.lift-coefficient", _lift_coefficient, true },
		{ "output.stall", _stall, true },
		{ "output.estimated-ias", _estimated_ias, true },
		{ "output.estimated-ias.error", _estimated_ias_error, true },
		{ "output.estimated-aoa", _estimated_aoa, true },
		{ "output.estimated-aoa.error", _estimated_aoa_error, true },
	});

	_wind_computer.set_callback (std::bind (&PerformanceComputer::compute_wind, this));
	_wind_computer.add_depending_smoothers ({
		&_wind_direction_smoother,
		&_wind_speed_smoother,
	});
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

	_total_energy_variometer_computer.set_minimum_dt (50_ms);
	_total_energy_variometer_computer.set_callback (std::bind (&PerformanceComputer::compute_total_energy_variometer, this));
	_total_energy_variometer_computer.add_depending_smoothers ({
		&_total_energy_variometer_smoother,
	});
	_total_energy_variometer_computer.observe ({
		&_altitude_amsl_std,
		&_speed_ias,
	});

	_speeds_computer.set_callback (std::bind (&PerformanceComputer::compute_speeds, this));
	_speeds_computer.observe ({
		&_density_altitude,
		&_input_air_density_static,
		&_input_aircraft_weight,
		&_input_flaps_angle,
		&_input_spoilers_angle,
		&_input_bank_angle,
	});

	_aoa_computer.set_minimum_dt (1_ms);
	_aoa_computer.set_callback (std::bind (&PerformanceComputer::compute_critical_aoa, this));
	_aoa_computer.observe ({
		&_input_flaps_angle,
		&_input_spoilers_angle,
		&_input_aoa_alpha,
	});

	_cl_computer.set_minimum_dt (10_ms);
	_cl_computer.set_callback (std::bind (&PerformanceComputer::compute_C_L, this));
	_cl_computer.add_depending_smoothers ({
		&_cl_smoother,
	});
	_cl_computer.observe ({
		&_input_load,
		&_input_aircraft_weight,
		&_input_air_density_static,
		&_speed_tas,
	});

	_estimations_computer.set_minimum_dt (10_ms);
	_estimations_computer.set_callback (std::bind (&PerformanceComputer::compute_estimations, this));
	_estimations_computer.observe ({
		&_input_load,
		&_input_aircraft_weight,
		&_input_air_density_static,
		&_input_flaps_angle,
		&_input_spoilers_angle,
		&_speed_tas,
		&_input_aoa_alpha,
	});
}


void
PerformanceComputer::data_updated()
{
	xf::PropertyObserver* computers[] = {
		// Order is important:
		&_wind_computer,
		&_glide_ratio_computer,
		&_total_energy_variometer_computer,
		&_speeds_computer,
		&_aoa_computer,
		&_cl_computer,
		&_estimations_computer,
	};

	for (xf::PropertyObserver* o: computers)
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

		xf::WindTriangle wt;
		wt.set_air_vector (*_speed_tas, *_orientation_heading_true);
		wt.set_ground_vector (*_speed_gs, *_track_lateral_true);
		wt.compute_wind_vector();
		_wind_from_true.write (xf::floored_mod (1_deg * _wind_direction_smoother.process (wt.wind_from().deg(), update_dt), 360_deg));
		_wind_from_magnetic.write (xf::true_to_magnetic (*_wind_from_true, *_magnetic_declination));
		_wind_tas.write (1_kt * _wind_speed_smoother.process (wt.wind_speed().kt(), update_dt));
	}
	else
	{
		_wind_from_true.set_nil();
		_wind_from_magnetic.set_nil();
		_wind_tas.set_nil();
		_wind_direction_smoother.invalidate();
		_wind_speed_smoother.invalidate();
	}
}


void
PerformanceComputer::compute_glide_ratio()
{
	if (_speed_gs.valid() && _vertical_speed.valid())
	{
		Speed forward_speed = *_speed_gs;
		int ratio = (forward_speed > 1_kt)
			? xf::limit<int> (forward_speed / *_vertical_speed, -99, +99)
			: 0;
		_glide_ratio.write (ratio);

		if (_glide_ratio_string.configured())
		{
			std::string arr;

			if (ratio > 0)
				arr = "↑";
			else if (ratio < 0)
				arr = "↓";
			else
				arr = "";

			if (std::abs (ratio) > 0)
				_glide_ratio_string = arr + (boost::format ("%02d:1") % std::abs (ratio)).str();
			else
				_glide_ratio_string = "=";
		}
	}
	else
	{
		_glide_ratio.set_nil();
		if (_glide_ratio_string.configured())
			_glide_ratio_string.set_nil();
	}
}


void
PerformanceComputer::compute_total_energy_variometer()
{
	if (_total_energy_variometer.configured())
	{
		Time update_dt = _total_energy_variometer_computer.update_dt();

		if (_altitude_amsl_std.valid() &&
			_input_aircraft_weight.valid() &&
			_speed_ias.valid())
		{
			Weight const m = *_input_aircraft_weight;
			Acceleration const g = 9.81_mps2;
			Speed const v = *_speed_ias;
			Energy const Ep = m * g * *_altitude_amsl_std;
			Energy const Ek = 1_J * m.si_units() * v.si_units() * v.si_units() * 0.5;
			Energy const total_energy = Ep + Ek;

			// If total energy was nil (invalid), reset _prev_total_energy
			// value to the current _total_energy:
			if (_total_energy_variometer.is_nil())
				_prev_total_energy = total_energy;

			Energy const energy_diff = total_energy - _prev_total_energy;
			Power tev = energy_diff / update_dt;

			// If IAS not in valid range, continue computations but only set output to nil.
			_total_energy_variometer_smoother.process (tev.W(), update_dt);
			if (*_speed_ias > _total_energy_variometer_min_ias)
				_total_energy_variometer = 1_W * _total_energy_variometer_smoother.value();
			else
				_total_energy_variometer.set_nil();

			_prev_total_energy = total_energy;
		}
		else
		{
			_total_energy_variometer.set_nil();
			_total_energy_variometer_smoother.invalidate();
		}
	}
}


void
PerformanceComputer::compute_speeds()
{
	xf::Airframe* airframe = module_manager()->application()->airframe();

	// Vs for load factors equivalent of banking 0_deg, 5_deg and 30_deg.
	_v_s_0_deg = get_stall_ias (0_deg);
	_v_s_5_deg = get_stall_ias (5_deg);
	_v_s_30_deg = get_stall_ias (30_deg);
	// Stall speed for current bank angle:
	_v_s = get_stall_ias (std::min (60_deg, _input_bank_angle.read (60_deg)));

	// V_r:
	if (_v_s_0_deg.valid())
		_v_r = 1.15 * *_v_s_0_deg;
	else
		_v_r.set_nil();

	// V_a; since the formula is almost identical as for V_s, use V_s_0_deg:
	if (airframe && _v_s_0_deg.valid())
	{
		xf::Range<double> lf_limits = airframe->load_factor_limits();
		double max_lf = std::min (lf_limits.max(), -lf_limits.min());
		_v_a = std::sqrt (max_lf) * *_v_s_0_deg;
	}
	else
		_v_a.set_nil();

	// V_REF for landing:
	if (_v_s_0_deg.valid())
		_v_approach = 1.3 * *_v_s_0_deg;
	else
		_v_approach.set_nil();

	// V_1
	// TODO
	// V_bg (best glide)
	// TODO
	// V_br (best range powered)
	// TODO
	// V_md (minimum descent unpowered)
	// TODO
	// V_be (best endurance powered)
	// TODO
	// V_y (best rate of climb)
	// TODO
	// V_x (best angle of climb)
	// TODO
}


Optional<Speed>
PerformanceComputer::get_stall_ias (Angle const& max_bank_angle) const
{
	using std::cos;

	// Formula:
	//   V_s = sqrt((load_factor * weight) / (0.5 * air_density * wings_area * C_L_max)).

	xf::Airframe* airframe = module_manager()->application()->airframe();
	if (airframe)
	{
		if (_input_aircraft_weight.valid() && _input_air_density_static.valid())
		{
			xf::Lift const& lift = airframe->lift();
			Area wings_area = airframe->wings_area();
			Angle max_safe_aoa = lift.critical_aoa() + airframe->safe_aoa_correction();
			xf::FlapsAngle flaps_angle (_input_flaps_angle.read (0_deg));
			xf::SpoilersAngle spoilers_angle (_input_spoilers_angle.read (0_deg));
			xf::LiftCoefficient cl = airframe->get_cl (max_safe_aoa, flaps_angle, spoilers_angle);

			Acceleration load = 1.0_g / cos (max_bank_angle);
			// Note: stick to SI units:
			Speed tas = Speed::from_si_units (std::sqrt (load.si_units() * _input_aircraft_weight->si_units()
														 / (0.5 * _input_air_density_static->si_units() * wings_area.si_units() * cl)));

			return tas_to_ias (tas);
		}
		else
			return {};
	}
	else
		return {};
}


Optional<Speed>
PerformanceComputer::tas_to_ias (Speed const& tas) const
{
	if (_density_altitude.valid())
		return xf::compute_indicated_airspeed (tas, *_density_altitude);
	else
		return {};
}


void
PerformanceComputer::compute_critical_aoa()
{
	xf::Airframe* airframe = module_manager()->application()->airframe();
	if (airframe)
	{
		xf::FlapsAngle flaps_angle (_input_flaps_angle.read (0_deg));
		xf::SpoilersAngle spoilers_angle (_input_spoilers_angle.read (0_deg));

		_critical_aoa = airframe->get_critical_aoa (flaps_angle, spoilers_angle);

		if (_stall.configured())
		{
			if (_input_aoa_alpha.valid())
				_stall = *_input_aoa_alpha >= *_critical_aoa;
			else
				_stall.set_nil();
		}
	}
	else
	{
		_critical_aoa.set_nil();
		_stall.set_nil();
	}
}


void
PerformanceComputer::compute_C_L()
{
	// Formula:
	//   C_L = load_factor * weight / (0.5 * air_density * TAS^2 * wings_area),
	// where
	//   load is down acceleration (in airplane frame of reference).

	Time update_dt = _cl_computer.update_dt();
	xf::Airframe* airframe = module_manager()->application()->airframe();

	if (airframe &&
		_input_load.valid() &&
		_input_aircraft_weight.valid() &&
		_input_air_density_static.valid() &&
		_speed_tas.valid())
	{
		Force lift = *_input_load * *_input_aircraft_weight;
		Speed tas = *_speed_tas;
		Area wings_area = airframe->wings_area();
		xf::LiftCoefficient cl (lift.si_units() / (0.5 * _input_air_density_static->si_units() * tas.si_units() * tas.si_units() * wings_area.si_units()));
		_cl_smoother.process (cl, update_dt);
		_lift_coefficient = _cl_smoother.value();
	}
	else
	{
		_lift_coefficient.set_nil();
		_cl_smoother.invalidate();
	}
}


void
PerformanceComputer::compute_C_D()
{
	// TODO weź bieżące przyspieszenie wzdłuż osi y, oblicz z F=ma jaka jest siła przyspieszająca,
	// oblicz różnicę między siłą ciągu a siłą przyspieszającą i to będzie drag.
	// Ze wzoru na drag oblicz współczynnik C_D.
	//   C_D = drag / (0.5 * air_density * TAS^2 * wing_area),
}


void
PerformanceComputer::compute_estimations()
{
	xf::Airframe* airframe = module_manager()->application()->airframe();

	if (airframe &&
		_input_load.valid() &&
		_input_aircraft_weight.valid() &&
		_input_air_density_static.valid())
	{
		Force lift_force = *_input_load * *_input_aircraft_weight;
		Area wings_area = airframe->wings_area();
		xf::FlapsAngle flaps_angle (_input_flaps_angle.read (0_deg));
		xf::SpoilersAngle spoilers_angle (_input_spoilers_angle.read (0_deg));

		// Estimate IAS:
		if (_input_aoa_alpha.valid())
		{
			xf::LiftCoefficient cl = airframe->get_cl (*_input_aoa_alpha, flaps_angle, spoilers_angle);
			double si_tas = std::sqrt (lift_force.N() / (0.5 * _input_air_density_static->si_units() * wings_area.si_units() * cl));
			Speed tas = Speed::from_si_units (si_tas);
			_estimated_ias = tas_to_ias (tas);
		}
		else
			_estimated_ias.set_nil();

		// Estimate AOA:
		if (_speed_tas.valid())
		{
			Speed tas = *_speed_tas;
			xf::LiftCoefficient cl (lift_force.N() / (0.5 * _input_air_density_static->si_units() * wings_area.si_units() * tas.si_units() * tas.si_units()));
			_estimated_aoa = airframe->get_aoa_in_normal_regime (cl, flaps_angle, spoilers_angle);
		}
		else
			_estimated_aoa.set_nil();

		// Compute errors:
		if (_speed_ias.valid() && _estimated_ias.valid())
			_estimated_ias_error = *_estimated_ias - *_speed_ias;
		else
			_estimated_ias_error.set_nil();

		if (_input_aoa_alpha.valid() && _estimated_aoa.valid())
			_estimated_aoa_error = *_estimated_aoa - *_input_aoa_alpha;
		else
			_estimated_aoa_error.set_nil();
	}
	else
	{
		_estimated_ias.set_nil();
		_estimated_aoa.set_nil();
	}
}

