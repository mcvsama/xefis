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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>

// Local:
#include "airfoil_shape.h"


namespace xf::sim {

AirfoilShape::AirfoilShape (AirfoilSpline const& spline,
							si::Length wing_length,
							si::Length chord_length,
							SpaceVector<si::Length, AirfoilSplineFrame> const& pivot_position,
							LiftField const& lift_field,
							DragField const& drag_field,
							PitchingMomentField const& pitching_moment_field,
							CenterOfPressureOffsetField const& center_of_pressure_offset_field):
	_spline (spline),
	_wing_length (wing_length),
	_chord_length (chord_length),
	_pivot_position (pivot_position),
	_lift_coefficient (lift_field),
	_drag_coefficient (drag_field),
	_pitching_moment_coefficient (pitching_moment_field),
	_center_of_pressure_offset (center_of_pressure_offset_field)
{ }


si::Force
AirfoilShape::lift_force (si::Angle alpha, si::Angle beta, Reynolds re, si::Pressure dynamic_pressure, std::optional<si::Area> lifting_area) const
{
	auto const cl = _lift_coefficient (*re, wrap_angle_for_field (alpha));

	if (!lifting_area)
		lifting_area = lift_drag_areas (alpha, beta)[0];

	return cl * dynamic_pressure * *lifting_area;
}


si::Force
AirfoilShape::drag_force (si::Angle alpha, si::Angle beta, Reynolds re, si::Pressure dynamic_pressure, std::optional<si::Area> dragging_area) const
{
	auto const cd = _drag_coefficient (*re, wrap_angle_for_field (alpha));

	if (!dragging_area)
		dragging_area = lift_drag_areas (alpha, beta)[1];

	return cd * dynamic_pressure * *dragging_area;
}


si::Torque
AirfoilShape::pitching_moment (si::Angle alpha, Reynolds re, si::Pressure dynamic_pressure) const
{
	auto const cm = _pitching_moment_coefficient (*re, wrap_angle_for_field (alpha));
	auto const wing_planform = _wing_length * _chord_length;
	return cm * dynamic_pressure * wing_planform * _chord_length;
}


ForceTorque<AirfoilSplineFrame>
AirfoilShape::planar_aerodynamic_forces (Atmosphere::State<AirfoilSplineFrame> const& atm, AngleOfAttack& aoa) const
{
	aoa.alpha = atan2 (atm.wind[1], atm.wind[0]);
	aoa.beta = atan2 (atm.wind[2], atm.wind[0]);

	SpaceVector<si::Velocity, AirfoilSplineFrame> const planar_wind { atm.wind[0], atm.wind[1], 0_mps };
	si::Velocity const planar_tas = abs (planar_wind);
	si::Pressure const planar_dp = dynamic_pressure (atm.air.density, planar_tas);
	Reynolds const planar_re = reynolds_number (atm.air.density, planar_tas, _chord_length, atm.air.dynamic_viscosity);
	PlaneVector<si::Area> const areas = lift_drag_areas (aoa.alpha, aoa.beta);
	si::Force const lift = lift_force (aoa.alpha, aoa.beta, planar_re, planar_dp, areas[0]);
	si::Force const drag = drag_force (aoa.alpha, aoa.beta, planar_re, planar_dp, areas[1]);
	si::Torque const torque = pitching_moment (aoa.alpha, planar_re, planar_dp);

	// Lift force is always perpendicular to relative wind.
	// Drag is always parallel to relative wind.
	// Pitching moment is always perpendicular to lift and drag forces.

	SpaceVector<si::Length, AirfoilSplineFrame> const center_of_pressure_from_leading_edge_vec { _center_of_pressure_offset (*planar_re, wrap_angle_for_field (aoa.alpha)) * _chord_length, 0_m, 0_m };
	SpaceVector<si::Length, AirfoilSplineFrame> const center_of_pressure_vec = center_of_pressure_from_leading_edge_vec - _pivot_position;
	SpaceVector<double, AirfoilSplineFrame> const drag_direction = normalized (atm.wind);
	SpaceVector<double, AirfoilSplineFrame> const lift_direction = normalized (cross_product (SpaceVector<double, AirfoilSplineFrame> { 0.0, 0.0, +1.0 }, atm.wind));
	SpaceVector<si::Force, AirfoilSplineFrame> /*const*/ total_force_vec = drag * drag_direction + lift * lift_direction;
	SpaceVector<si::Torque, AirfoilSplineFrame> const pitching_moment_vec = { 0_Nm, 0_Nm, torque };
	Wrench<AirfoilSplineFrame> const wrench { total_force_vec, pitching_moment_vec, center_of_pressure_vec };

	return resultant_force (wrench);
}


PlaneVector<si::Area>
AirfoilShape::lift_drag_areas (si::Angle alpha, si::Angle beta) const
{
	return _spline.projected_chord_and_thickness (alpha, beta) * _chord_length * _wing_length;
}

} // namespace xf::sim

