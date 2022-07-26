/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
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
#include "wing.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/geometry/triangle.h>
#include <xefis/support/geometry/triangulation.h>
#include <xefis/support/simulation/rigid_body/various_shapes.h>

// Standard:
#include <cstddef>


namespace xf::sim {

Wing::Wing (Airfoil const& airfoil, si::Density const material_density):
	Body (calculate_body_space_mass_moments (airfoil, material_density)),
	_airfoil (airfoil)
{
	auto const com = last_center_of_mass();
	set_origin_at (-com);
	_com_to_planar_origin = -com;

	auto shape = rigid_body::make_airfoil_shape (_airfoil.spline(), _airfoil.chord_length(), _airfoil.wing_length(), true, {});
	shape.translate (-com);
	set_shape (shape);
}


void
Wing::update_external_forces (AtmosphereModel const* atmosphere)
{
	if (atmosphere)
	{
		// TODO Create special UnitMatrix<TF, SF> which only changes the frame, and operator* that doesn't really compute anything
		// Rotations:
		auto const world_to_ecef = RotationMatrix<ECEFSpace, rigid_body::WorldSpace> (math::unit);
		auto const body_to_airfoil_spline = RotationMatrix<AirfoilSplineSpace, rigid_body::BodySpace> (math::unit);
		auto const world_to_body = location().base_to_body_rotation();
		// ECEF → WorldSpace → BodySpace → AirfoilSplineSpace:
		RotationMatrix<AirfoilSplineSpace, ECEFSpace> ecef_to_spline = body_to_airfoil_spline * world_to_body * ~world_to_ecef;

		auto const body_position_in_ecef = world_to_ecef * location().position();
		auto const body_velocity_in_ecef = world_to_ecef * velocity_moments<rigid_body::WorldSpace>().velocity();
		// FIXME take velocity_moments().angular_velocity() into account

		auto const air = atmosphere->air_at (body_position_in_ecef);
		auto const ecef_wind = atmosphere->wind_at (body_position_in_ecef) - body_velocity_in_ecef;
		auto const spline_wind = ecef_to_spline * ecef_wind;
		auto const atmosphere_state = AtmosphereState<AirfoilSplineSpace> { air, spline_wind };
		AngleOfAttack aoa;

		// Center of pressure Wrench:
		auto const spline_aeroforces_at_origin = _airfoil.aerodynamic_forces (atmosphere_state, aoa);
		auto const body_aeroforces_at_origin = ~body_to_airfoil_spline * spline_aeroforces_at_origin;

		// Compute 'at COM' values:
		_lift_force = body_aeroforces_at_origin.lift;
		_drag_force = body_aeroforces_at_origin.drag;
		_pitching_moment = body_aeroforces_at_origin.pitching_moment;
		_center_of_pressure = body_aeroforces_at_origin.center_of_pressure + _com_to_planar_origin;

		apply_force (ForceMoments<rigid_body::BodySpace> (_lift_force, _pitching_moment), _center_of_pressure);
		apply_force (ForceMoments<rigid_body::BodySpace> (_drag_force, math::zero), _center_of_pressure);
	}
}

} // namespace xf::sim

