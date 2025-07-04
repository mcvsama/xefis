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
#include "group.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>
#include <ranges>


namespace xf::rigid_body {

void
Group::rotate_about_world_origin (RotationQuaternion<WorldSpace> const& rotation)
{
	for (auto& body: _bodies)
		body->rotate_about_world_origin (rotation);
}


void
Group::rotate_about (SpaceLength<WorldSpace> const& about_point, RotationQuaternion<WorldSpace> const& rotation)
{
	for (auto& body: _bodies)
		body->rotate_about (about_point, rotation);
}


void
Group::translate (SpaceLength<WorldSpace> const& translation)
{
	for (auto& body: _bodies)
		body->translate (translation);
}


si::Energy
Group::translational_kinetic_energy() const
{
	si::Energy e = 0_J;

	for (auto const& body: _bodies)
		e += body->translational_kinetic_energy();

	return e;
}


si::Energy
Group::rotational_kinetic_energy() const
{
	si::Energy e = 0_J;

	for (auto const& body: _bodies)
		e += body->rotational_kinetic_energy();

	return e;
}


MassMomentsAtArm<WorldSpace>
Group::mass_moments() const
{
	auto const get_mass_moments_from_body = [](auto& body_ptr) -> MassMomentsAtArm<WorldSpace> {
		auto const mm_at_com = body_ptr->template mass_moments<WorldSpace>();
		return MassMomentsAtArm<WorldSpace> (mm_at_com.mass(), body_ptr->placement().position(), mm_at_com.inertia_tensor());
	};

	return compute_mass_moments_at_arm<WorldSpace> (_bodies | std::views::transform (get_mass_moments_from_body));
}

} // namespace xf::rigid_body

