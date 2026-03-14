/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/nature/mass_moments.h>
#include <xefis/support/nature/various_inertia_tensors.h>
#include <xefis/support/shapes/various_shapes.h>
#include <xefis/support/simulation/rigid_body/body.h>
#include <xefis/support/simulation/rigid_body/system.h>

// Neutrino:
#include <neutrino/test/auto_test.h>

// Standard:
#include <cmath>


namespace xf::test {
namespace {

namespace rb = rigid_body;
namespace test_asserts = nu::test_asserts;


MassMoments<BodyCOM>
make_mass_moments (si::Mass const mass = 1_kg)
{
	return {
		mass,
		make_cuboid_inertia_tensor<BodyCOM> (mass, { 1_m, 1_m, 1_m }),
	};
}


nu::AutoTest t_group_bounding_sphere ("rigid_body::Group: bounding sphere radius", []{
	auto system = rb::System();
	auto& group = system.make_group();

	auto& left = group.add<rb::Body> (make_mass_moments());
	left.set_shape (make_centered_cube_shape (2_m));
	left.move_to (SpaceLength<WorldSpace> { 0_m, 0_m, 0_m });

	auto& right = group.add<rb::Body> (make_mass_moments());
	right.set_shape (make_centered_cube_shape (2_m));
	right.move_to (SpaceLength<WorldSpace> { 10_m, 0_m, 0_m });

	test_asserts::verify_equal_with_epsilon ("radius is around group COM",
											 group.bounding_sphere_radius(),
											 5_m + std::sqrt (3.0) * 1_m,
											 1e-12_m);
});


nu::AutoTest t_group_bounding_sphere_without_shapes ("rigid_body::Group: bounding sphere radius includes shapeless bodies", []{
	auto system = rb::System();
	auto& group = system.make_group();

	auto& left = group.add<rb::Body> (make_mass_moments());
	left.move_to (SpaceLength<WorldSpace> { 0_m, 0_m, 0_m });

	auto& right = group.add<rb::Body> (make_mass_moments());
	right.move_to (SpaceLength<WorldSpace> { 10_m, 0_m, 0_m });

	test_asserts::verify_equal_with_epsilon ("shapeless bodies still contribute by COM distance",
											 group.bounding_sphere_radius(),
											 5_m,
											 1e-12_m);
});


nu::AutoTest t_group_mass_moments_include_parallel_axis ("rigid_body::Group: mass moments include displacement from world origin", []{
	auto system = rb::System();
	auto& group = system.make_group();

	auto const body_mass = 2_kg;
	auto const body_inertia_at_com = make_cuboid_inertia_tensor<BodyCOM> (body_mass, { 1_m, 1_m, 1_m });
	auto& body = group.add<rb::Body> (MassMoments<BodyCOM> (body_mass, body_inertia_at_com));
	body.move_to (SpaceLength<WorldSpace> { 3_m, 0_m, 0_m });

	auto const group_mass_moments = group.mass_moments();
	auto const expected_inertia_at_world_origin =
		inertia_tensor_com_to_point<WorldSpace> (body_mass, body.mass_moments<WorldSpace>().inertia_tensor(), body.placement().position());

	test_asserts::verify_equal_with_epsilon ("group mass matches body mass",
											 group_mass_moments.mass(),
											 body_mass,
											 1e-12_kg);
	test_asserts::verify_equal_with_epsilon ("group COM matches body COM",
											 group_mass_moments.center_of_mass_position(),
											 SpaceLength<WorldSpace> { 3_m, 0_m, 0_m },
											 1e-12_m);
	test_asserts::verify_equal_with_epsilon ("group inertia tensor includes parallel-axis contribution",
											 group_mass_moments.inertia_tensor(),
											 expected_inertia_at_world_origin,
											 1e-12_kgm2);
});

} // namespace
} // namespace xf::test
