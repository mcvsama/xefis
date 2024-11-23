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

Body::Body (MassMoments<BodyCOM> const& mass_moments, ShapeType const shape_type):
	_shape_type (shape_type)
{
	set_mass_moments (mass_moments);
}


Body::Body (MassMomentsAtArm<BodyCOM> const& mass_moments, ShapeType const shape_type):
	_shape_type (shape_type)
{
	set_mass_moments (mass_moments);
}


void
Body::set_mass_moments (MassMomentsAtArm<BodyCOM> const& mass_moments)
{
	auto const com_position = mass_moments.center_of_mass_position();

	// We want mass moments to be viewed from the center of mass, so translate if necessary
	// (this should transform the inertia tensor accordingly):
	_mass_moments = mass_moments.centered_at_center_of_mass();

	// Move the body so that the placement().position() points to the current center of mass:
	translate<BodyCOM> (com_position);

	// Because the origin is defined as relative to center of mass, and we just moved center of mass
	// while not wanting to move origin, move the origin back:
	_origin_placement.translate_frame (-com_position);
}


void
Body::rotate_about_center_of_mass (RotationQuaternion<WorldSpace> const& rotation)
{
	_placement.rotate_body_frame (rotation);
	_velocity_moments = rotation * _velocity_moments;
	_acceleration_moments = rotation * _acceleration_moments;

	_world_space_applied_impulses.reset();
}


void
Body::rotate_about_world_origin (RotationQuaternion<WorldSpace> const& rotation)
{
	_placement.rotate_base_frame (rotation);
	_velocity_moments = rotation * _velocity_moments;
	_acceleration_moments = rotation * _acceleration_moments;

	_world_space_applied_impulses.reset();
}


void
Body::rotate_about_body_origin (RotationQuaternion<WorldSpace> const& rotation)
{
	SpaceLength<WorldSpace> const about_point = _placement.bound_transform_to_base (_origin_placement.position());

	_placement.rotate_base_frame_about (about_point, rotation);
	_velocity_moments = rotation * _velocity_moments;
	_acceleration_moments = rotation * _acceleration_moments;

	_world_space_applied_impulses.reset();
}


void
Body::move_origin_to (SpaceLength<WorldSpace> const& new_origin_position)
{
	SpaceLength<BodyCOM> const old_origin_position_in_BodyCOM = _origin_placement.position();
	SpaceLength<BodyCOM> const new_origin_position_in_BodyCOM = _placement.bound_transform_to_body (new_origin_position);
	SpaceLength<BodyCOM> const new_com_position_in_BodyCOM = new_origin_position_in_BodyCOM - old_origin_position_in_BodyCOM;
	SpaceLength<WorldSpace> const new_com_position_in_WorldSpace = _placement.bound_transform_to_base (new_com_position_in_BodyCOM);
	move_to (new_com_position_in_WorldSpace);
}


si::Energy
Body::translational_kinetic_energy() const
{
	auto const mm = mass_moments();
	auto const vm = velocity_moments<WorldSpace>();
	return 0.5 * mm.mass() * square (abs (vm.velocity()));
}


si::Energy
Body::rotational_kinetic_energy() const
{
	auto const mm = mass_moments();
	auto const vm = velocity_moments<BodyCOM>();
	return 0.5 * (~vm.angular_velocity() * mm.inertia_tensor() * vm.angular_velocity() / 1_rad / 1_rad).scalar();
}

} // namespace xf::rigid_body

