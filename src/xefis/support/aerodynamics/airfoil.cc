/* vim:ts=4
 *
 * Copyleft 2008…2018  Michał Gawron
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
#include "airfoil.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/wrench.h>

// Standard:
#include <cstddef>


namespace xf {

Airfoil::Airfoil (AirfoilCharacteristics const& airfoil_characteristics,
				  si::Length const chord_length,
				  si::Length const wing_length):
	_airfoil_characteristics (airfoil_characteristics),
	_chord_length (chord_length),
	_wing_length (wing_length)
{ }


si::Force
Airfoil::lift_force (si::Angle alpha, si::Angle beta, Reynolds re, si::Pressure dynamic_pressure, std::optional<si::Area> lifting_area) const
{
	auto const cl = _airfoil_characteristics.lift_coefficient (*re, wrap_angle_for_field (alpha));

	if (!lifting_area)
		lifting_area = lift_drag_areas (alpha, beta).first;

	return cl * dynamic_pressure * *lifting_area;
}


si::Force
Airfoil::drag_force (si::Angle alpha, si::Angle beta, Reynolds re, si::Pressure dynamic_pressure, std::optional<si::Area> dragging_area) const
{
	auto const cd = _airfoil_characteristics.drag_coefficient (*re, wrap_angle_for_field (alpha));

	if (!dragging_area)
		dragging_area = lift_drag_areas (alpha, beta).second;

	return cd * dynamic_pressure * *dragging_area;
}


si::Torque
Airfoil::pitching_moment (si::Angle alpha, Reynolds re, si::Pressure dynamic_pressure) const
{
	auto const cm = _airfoil_characteristics.pitching_moment_coefficient (*re, wrap_angle_for_field (alpha));
	auto const wing_planform = _wing_length * _chord_length;
	return cm * dynamic_pressure * wing_planform * _chord_length;
}


AerodynamicForces<AirfoilSplineSpace>
Airfoil::planar_aerodynamic_forces (AtmosphereState<AirfoilSplineSpace> const& atm, AngleOfAttack& aoa) const
{
	// Wind vector will be normalized, so make sure it's not near 0:
	if (abs (atm.wind) > 1e-6_mps)
	{
		aoa.alpha = 1_rad * atan2 (atm.wind[1], atm.wind[0]);
		aoa.beta = 1_rad * atan2 (atm.wind[2], atm.wind[0]);

		SpaceVector<si::Velocity, AirfoilSplineSpace> const planar_wind { atm.wind[0], atm.wind[1], 0_mps };
		si::Velocity const	planar_tas				= abs (planar_wind);
		si::Pressure const	planar_dp				= dynamic_pressure (atm.air.density, planar_tas);
		Reynolds const		planar_re				= reynolds_number (atm.air.density, planar_tas, _chord_length, atm.air.dynamic_viscosity);
		auto const			[lift_area, drag_area]	= lift_drag_areas (aoa.alpha, aoa.beta);
		si::Force const		lift					= lift_force (aoa.alpha, aoa.beta, planar_re, planar_dp, lift_area);
		si::Force const		drag					= drag_force (aoa.alpha, aoa.beta, planar_re, planar_dp, drag_area);
		si::Torque const	torque					= pitching_moment (aoa.alpha, planar_re, planar_dp);

		// Lift force is always perpendicular to relative wind.
		// Drag is always parallel to relative wind.
		// Pitching moment is always perpendicular to lift and drag forces.

		SpaceVector<si::Length, AirfoilSplineSpace> const	cp_position			{ _airfoil_characteristics.center_of_pressure_position (*planar_re, wrap_angle_for_field (aoa.alpha)) * _chord_length, 0_m, 0_m };
		// If atm.wind is 0, normalized will be nan³, but we're guarded by the if above.
		SpaceVector<double, AirfoilSplineSpace> const		drag_direction		= normalized (atm.wind) / 1_mps;
		SpaceVector<double, AirfoilSplineSpace> const		lift_direction		= normalized (cross_product (SpaceVector<double, AirfoilSplineSpace> { 0.0, 0.0, +1.0 }, atm.wind)) / 1_mps;
		SpaceVector<si::Torque, AirfoilSplineSpace> const	pitching_moment_vec	{ 0_Nm, 0_Nm, torque };

		// TODO drag should be 3D, that is also in Z direction
		// TODO maybe lift, too?
		return {
			lift * lift_direction,
			drag * drag_direction,
			pitching_moment_vec,
			cp_position,
		};
	}
	else
	{
		aoa.alpha = 0_deg;
		aoa.beta = 0_deg;
		return {};
	}
}


AerodynamicForces<AirfoilSplineSpace>
Airfoil::aerodynamic_forces (AtmosphereState<AirfoilSplineSpace> const& atm, AngleOfAttack& aoa) const
{
	auto planar = planar_aerodynamic_forces (atm, aoa);
	planar.center_of_pressure += SpaceLength<AirfoilSplineSpace> { 0_m, 0_m, 0.5 * _wing_length };
	return planar;
}


std::pair<si::Area, si::Area>
Airfoil::lift_drag_areas (si::Angle alpha, si::Angle beta) const
{
	auto const [chord, thickness] = _airfoil_characteristics.spline().projected_chord_and_thickness (alpha, beta);
	auto const k = _chord_length * _wing_length;
	return { k * chord, k * thickness };
}


constexpr si::Angle
Airfoil::wrap_angle_for_field (si::Angle angle)
{
	return floored_mod (angle, Range<si::Angle> (-180_deg, +180_deg));
}

} // namespace xf

