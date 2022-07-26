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
#include "body.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf::rigid_body {

void
Body::rotate_about_center_of_mass (RotationMatrix<WorldSpace> const& rotation)
{
	_location.rotate_body_frame (rotation);
	_velocity_moments = rotation * _velocity_moments;
	_acceleration_moments = rotation * _acceleration_moments;

	_world_space_mass_moments.reset();
	_world_space_applied_forces.reset();
}


void
Body::rotate_about_world_origin (RotationMatrix<WorldSpace> const& rotation)
{
	_location.rotate_base_frame (rotation);
	_velocity_moments = rotation * _velocity_moments;
	_acceleration_moments = rotation * _acceleration_moments;

	_world_space_mass_moments.reset();
	_world_space_applied_forces.reset();
}


void
Body::rotate_about_body_origin (RotationMatrix<WorldSpace> const& rotation)
{
	auto const about_point = _location.bound_transform_to_base (_origin_position);

	_location.rotate_base_frame_about (about_point, rotation);
	_velocity_moments = rotation * _velocity_moments;
	_acceleration_moments = rotation * _acceleration_moments;

	_world_space_mass_moments.reset();
	_world_space_applied_forces.reset();
}


si::Energy
Body::kinetic_energy() const
{
	auto const mm = mass_moments<WorldSpace>();
	auto const vm = velocity_moments<WorldSpace>();
	auto const twice_linear_energy = mm.mass() * square (abs (vm.velocity()));
	auto const twice_angular_energy = ~vm.angular_velocity() * mm.moment_of_inertia() * vm.angular_velocity() / 1_rad / 1_rad;
	return 0.5 * (twice_linear_energy + twice_angular_energy.scalar());
}

} // namespace xf::rigid_body

