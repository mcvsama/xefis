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
#include "pc.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/airframe/airframe.h>
#include <xefis/support/airframe/lift.h>
#include <xefis/support/airframe/types.h>
#include <xefis/support/earth/air/air.h>
#include <xefis/support/earth/navigation/wind_triangle.h>
#include <xefis/support/nature/constants.h>

// Neutrino:
#include <neutrino/exception.h>
#include <neutrino/numeric.h>
#include <neutrino/qt/qdom.h>

// Standard:
#include <cstddef>
#include <limits>


PerformanceComputer::PerformanceComputer (xf::Airframe* airframe, std::string_view const& instance):
	PerformanceComputerIO (instance),
	_airframe (airframe)
{
	_wind_computer.set_callback (std::bind (&PerformanceComputer::compute_wind, this));
	_wind_computer.add_depending_smoothers ({
		&_wind_direction_smoother,
		&_wind_speed_smoother,
	});
	_wind_computer.observe ({
		&_io.speed_tas,
		&_io.speed_gs,
		&_io.track_lateral_true,
		&_io.orientation_heading_true,
		&_io.magnetic_declination,
	});

	_glide_ratio_computer.set_callback (std::bind (&PerformanceComputer::compute_glide_ratio, this));
	_glide_ratio_computer.observe ({
		&_io.speed_gs,
		&_io.vertical_speed,
	});

	_total_energy_variometer_computer.set_minimum_dt (50_ms);
	_total_energy_variometer_computer.set_callback (std::bind (&PerformanceComputer::compute_total_energy_variometer, this));
	_total_energy_variometer_computer.add_depending_smoothers ({
		&_total_energy_variometer_smoother,
	});
	_total_energy_variometer_computer.observe ({
		&_io.altitude_amsl_std,
		&_io.speed_ias,
	});

	_speeds_computer.set_callback (std::bind (&PerformanceComputer::compute_speeds, this));
	_speeds_computer.observe ({
		&_io.density_altitude,
		&_io.air_density_static,
		&_io.aircraft_mass,
		&_io.flaps_angle,
		&_io.spoilers_angle,
		&_io.bank_angle,
	});

	_aoa_computer.set_minimum_dt (1_ms);
	_aoa_computer.set_callback (std::bind (&PerformanceComputer::compute_critical_aoa, this));
	_aoa_computer.observe ({
		&_io.flaps_angle,
		&_io.spoilers_angle,
		&_io.aoa_alpha,
	});

	_cl_computer.set_minimum_dt (10_ms);
	_cl_computer.set_callback (std::bind (&PerformanceComputer::compute_C_L, this));
	_cl_computer.add_depending_smoothers ({
		&_cl_smoother,
	});
	_cl_computer.observe ({
		&_io.load,
		&_io.aircraft_mass,
		&_io.air_density_static,
		&_io.speed_tas,
	});

	_estimations_computer.set_minimum_dt (10_ms);
	_estimations_computer.set_callback (std::bind (&PerformanceComputer::compute_estimations, this));
	_estimations_computer.observe ({
		&_io.load,
		&_io.aircraft_mass,
		&_io.air_density_static,
		&_io.flaps_angle,
		&_io.spoilers_angle,
		&_io.speed_tas,
		&_io.aoa_alpha,
	});

	_slip_skid_computer.set_minimum_dt (10_ms);
	_slip_skid_computer.set_callback (std::bind (&PerformanceComputer::compute_slip_skid, this));
	_slip_skid_computer.observe ({
		&_io.y_acceleration,
		&_io.z_acceleration,
	});
}


void
PerformanceComputer::process (xf::Cycle const& cycle)
{
	xf::SocketObserver* computers[] = {
		// Order is important:
		&_wind_computer,
		&_glide_ratio_computer,
		&_total_energy_variometer_computer,
		&_speeds_computer,
		&_aoa_computer,
		&_cl_computer,
		&_estimations_computer,
	};

	for (auto* o: computers)
		o->process (cycle.update_time());
}


void
PerformanceComputer::compute_wind()
{
	if (_io.speed_tas && _io.speed_gs && _io.track_lateral_true && _io.orientation_heading_true && _io.magnetic_declination)
	{
		si::Time update_dt = _wind_computer.update_dt();

		xf::WindTriangle wt;
		wt.set_air_vector (*_io.speed_tas, *_io.orientation_heading_true);
		wt.set_ground_vector (*_io.speed_gs, *_io.track_lateral_true);
		wt.compute_wind_vector();
		_io.wind_from_true = xf::floored_mod (_wind_direction_smoother (wt.wind_from(), update_dt), 360_deg);
		_io.wind_from_magnetic = xf::true_to_magnetic (*_io.wind_from_true, *_io.magnetic_declination);
		_io.wind_tas = _wind_speed_smoother (wt.wind_speed(), update_dt);
	}
	else
	{
		_io.wind_from_true = xf::nil;
		_io.wind_from_magnetic = xf::nil;
		_io.wind_tas = xf::nil;
		_wind_direction_smoother.invalidate();
		_wind_speed_smoother.invalidate();
	}
}


void
PerformanceComputer::compute_glide_ratio()
{
	if (_io.speed_gs && _io.vertical_speed)
	{
		si::Velocity forward_speed = *_io.speed_gs;
		int ratio = (forward_speed > 1_kt)
			? xf::clamped<int> (forward_speed / *_io.vertical_speed, -99, +99)
			: 0;
		_io.glide_ratio = ratio;

		if (_io.glide_ratio_string.use_count() > 0)
		{
			std::string_view arr;

			if (ratio > 0)
				arr = "↑";
			else if (ratio < 0)
				arr = "↓";
			else
				arr = "";

			if (std::abs (ratio) > 0)
				_io.glide_ratio_string = arr + (boost::format ("%02d:1") % std::abs (ratio)).str();
			else
				_io.glide_ratio_string = "=";
		}
	}
	else
	{
		_io.glide_ratio = xf::nil;
		_io.glide_ratio_string = xf::nil;
	}
}


void
PerformanceComputer::compute_total_energy_variometer()
{
	if (_io.total_energy_variometer.use_count() > 0)
	{
		si::Time update_dt = _total_energy_variometer_computer.update_dt();

		if (_io.altitude_amsl_std && _io.aircraft_mass && _io.speed_ias)
		{
			si::Mass const m = *_io.aircraft_mass;
			si::Acceleration const g = xf::kStdGravitationalAcceleration;
			si::Velocity const v = *_io.speed_ias;
			si::Energy const Ep = m * g * *_io.altitude_amsl_std;
			si::Energy const Ek = m * v * v * 0.5;
			si::Energy const total_energy = Ep + Ek;

			// If total energy was nil (invalid), reset _prev_total_energy
			// value to the current _total_energy:
			if (_io.total_energy_variometer.is_nil())
				_prev_total_energy = total_energy;

			si::Energy const energy_diff = total_energy - _prev_total_energy;
			si::Power tev = energy_diff / update_dt;

			// If IAS not in valid range, continue computations but only set output to nil.
			_total_energy_variometer_smoother (tev, update_dt);

			if (*_io.speed_ias > *_io.tev_min_ias)
				_io.total_energy_variometer = _total_energy_variometer_smoother.value();
			else
				_io.total_energy_variometer = xf::nil;

			_prev_total_energy = total_energy;
		}
		else
		{
			_io.total_energy_variometer = xf::nil;
			_total_energy_variometer_smoother.invalidate();
		}
	}
}


void
PerformanceComputer::compute_speeds()
{
	// Vs for load factors equivalent of banking 0_deg, 5_deg and 30_deg.
	_io.v_s_0_deg = get_stall_ias (0_deg);
	_io.v_s_5_deg = get_stall_ias (5_deg);
	_io.v_s_30_deg = get_stall_ias (30_deg);
	// Stall speed for current bank angle:
	_io.v_s = get_stall_ias (std::min<si::Angle> (60_deg, _io.bank_angle.value_or (60_deg)));

	// V_r:
	if (_io.v_s_0_deg)
		_io.v_r = 1.15 * *_io.v_s_0_deg;
	else
		_io.v_r = xf::nil;

	// V_a; since the formula is almost identical as for V_s, use V_s_0_deg:
	if (_airframe && _io.v_s_0_deg)
	{
		xf::Range<double> lf_limits = _airframe->load_factor_limits();
		double max_lf = std::min (lf_limits.max(), -lf_limits.min());
		_io.v_a = std::sqrt (max_lf) * *_io.v_s_0_deg;
	}
	else
		_io.v_a = xf::nil;

	// V_REF for landing:
	if (_io.v_s_0_deg)
		_io.v_approach = 1.3 * *_io.v_s_0_deg;
	else
		_io.v_approach = xf::nil;

	// Rest:
	compute_speeds_vbg();

	// V_1
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


inline void
PerformanceComputer::compute_speeds_vbg()
{
	// V-bg - "best glide" - speed for best unpowered range.
	//
	// Find AOA and IAS for which lift/drag is at maximum
	// (equivalent to C_L/C_D).

	if (_airframe && _io.flaps_angle && _io.spoilers_angle)
	{
		xf::FlapsAngle flaps_angle (*_io.flaps_angle);
		xf::SpoilersAngle spoilers_angle (*_io.spoilers_angle);

		xf::Range<si::Angle> aoa_range = _airframe->get_defined_aoa_range();

		std::optional<si::Angle> v_bg_aoa;
		double v_bg_ratio = std::numeric_limits<double>::max();

		for (si::Angle aoa = aoa_range.min(); aoa < aoa_range.max(); aoa += 0.25_deg)
		{
			double cl = _airframe->get_cl (aoa, flaps_angle, spoilers_angle);
			double cd = _airframe->get_cd (aoa, flaps_angle, spoilers_angle);
			double new_ratio = cl / cd;

			if (new_ratio < v_bg_ratio)
			{
				v_bg_ratio = new_ratio;
				v_bg_aoa = aoa;
			}
		}

		if (v_bg_aoa)
		{
			auto tas = aoa_to_tas_now (*v_bg_aoa);

			if (tas)
				_io.v_bg = tas_to_ias (*tas);
			else
				_io.v_bg = xf::nil;
		}
		else
			_io.v_bg = xf::nil;
	}
	else
		_io.v_bg = xf::nil;
}


std::optional<si::Velocity>
PerformanceComputer::get_stall_ias (si::Angle const max_bank_angle) const
{
	// Formula:
	//   V_s = sqrt((load_factor * weight) / (0.5 * air_density * wings_area * C_L_max)).

	if (_airframe)
	{
		xf::FlapsAngle flaps_angle (_io.flaps_angle.value_or (0_deg));
		xf::SpoilersAngle spoilers_angle (_io.spoilers_angle.value_or (0_deg));
		si::Angle max_safe_aoa = _airframe->get_max_safe_aoa (flaps_angle, spoilers_angle);
		si::Acceleration load = 1_g / si::cos (max_bank_angle);

		auto tas = aoa_to_tas_now (max_safe_aoa, load);

		if (tas)
			return tas_to_ias (*tas);
		else
			return { };
	}
	else
		return { };
}


std::optional<si::Velocity>
PerformanceComputer::tas_to_ias (si::Velocity const tas) const
{
	if (_io.density_altitude)
		return xf::indicated_airspeed (tas, *_io.density_altitude);
	else
		return { };
}


void
PerformanceComputer::compute_critical_aoa()
{
	if (_airframe)
	{
		xf::FlapsAngle flaps_angle (_io.flaps_angle.value_or (0_deg));
		xf::SpoilersAngle spoilers_angle (_io.spoilers_angle.value_or (0_deg));

		_io.critical_aoa = _airframe->get_critical_aoa (flaps_angle, spoilers_angle);

		if (_io.aoa_alpha)
			_io.stall = *_io.aoa_alpha >= *_io.critical_aoa;
		else
			_io.stall = xf::nil;
	}
	else
	{
		_io.critical_aoa = xf::nil;
		_io.stall = xf::nil;
	}
}


void
PerformanceComputer::compute_C_L()
{
	// Formula:
	//   C_L = load_factor * weight / (0.5 * air_density * TAS^2 * wings_area),
	// where
	//   load is down acceleration (in airplane frame of reference).

	si::Time update_dt = _cl_computer.update_dt();

	if (_airframe && _io.load && _io.aircraft_mass && _io.air_density_static && _io.speed_tas)
	{
		si::Force lift = *_io.load * *_io.aircraft_mass;
		si::Velocity tas = *_io.speed_tas;
		si::Area wings_area = _airframe->wings_area();
		xf::LiftCoefficient cl (lift / (0.5 * (*_io.air_density_static) * tas * tas * wings_area));
		_cl_smoother.process (cl, update_dt);
		_io.lift_coefficient = _cl_smoother.value();
	}
	else
	{
		_io.lift_coefficient = xf::nil;
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
	if (_airframe && _io.load && _io.aircraft_mass && _io.air_density_static)
	{
		si::Force lift_force = *_io.load * *_io.aircraft_mass;
		si::Area wings_area = _airframe->wings_area();
		xf::FlapsAngle flaps_angle (_io.flaps_angle.value_or (0_deg));
		xf::SpoilersAngle spoilers_angle (_io.spoilers_angle.value_or (0_deg));

		// Estimate IAS:
		if (_io.aoa_alpha)
		{
			xf::LiftCoefficient cl = _airframe->get_cl (*_io.aoa_alpha, flaps_angle, spoilers_angle);
			si::Velocity tas = sqrt (lift_force / (0.5 * (*_io.air_density_static) * wings_area * cl));
			_io.estimated_ias = tas_to_ias (tas);
		}
		else
			_io.estimated_ias = xf::nil;

		// Estimate AOA:
		if (_io.speed_tas)
		{
			si::Velocity tas = *_io.speed_tas;
			xf::LiftCoefficient cl (lift_force / (0.5 * (*_io.air_density_static) * tas * tas * wings_area));
			_io.estimated_aoa = _airframe->get_aoa_in_normal_regime (cl, flaps_angle, spoilers_angle);
		}
		else
			_io.estimated_aoa = xf::nil;

		// Compute errors:
		if (_io.speed_ias && _io.estimated_ias)
			_io.estimated_ias_error = *_io.estimated_ias - *_io.speed_ias;
		else
			_io.estimated_ias_error = xf::nil;

		if (_io.aoa_alpha && _io.estimated_aoa)
			_io.estimated_aoa_error = *_io.estimated_aoa - *_io.aoa_alpha;
		else
			_io.estimated_aoa_error = xf::nil;
	}
	else
	{
		_io.estimated_ias = xf::nil;
		_io.estimated_aoa = xf::nil;
	}
}


void
PerformanceComputer::compute_slip_skid()
{
	if (_io.y_acceleration && _io.z_acceleration)
		_io.slip_skid = 1_rad * si::atan2 (*_io.y_acceleration, -*_io.z_acceleration);
	else
		_io.slip_skid = xf::nil;
}


inline std::optional<si::Velocity>
PerformanceComputer::aoa_to_tas_now (si::Angle const aoa, std::optional<si::Acceleration> const load) const
{
	if (_airframe && _io.load && _io.aircraft_mass && _io.air_density_static && _io.flaps_angle && _io.spoilers_angle)
	{
		si::Area wings_area = _airframe->wings_area();
		xf::FlapsAngle flaps_angle (*_io.flaps_angle);
		xf::SpoilersAngle spoilers_angle (*_io.spoilers_angle);
		xf::LiftCoefficient cl = _airframe->get_cl (aoa, flaps_angle, spoilers_angle);
		si::Acceleration xload = load ? *load : *_io.load;
		si::Force lift = xload * *_io.aircraft_mass;
		// Result is TAS:
		return sqrt (lift / (0.5 * (*_io.air_density_static) * wings_area * cl));
	}
	else
		return { };
}

