/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
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
#include <xefis/support/nature/constants.h>
#include <xefis/support/nature/mass_moments.h>
#include <xefis/support/nature/various_inertia_tensors.h>
#include <xefis/support/shapes/various_shapes.h>
#include <xefis/support/simulation/rigid_body/body.h>

// Neutrino:
#include <neutrino/test/auto_test.h>

// Standard:
#include <cmath>
#include <cstddef>


namespace xf::test {
namespace {

namespace rb = rigid_body;
namespace test_asserts = nu::test_asserts;


std::unique_ptr<rb::Body>
make_unit_body()
{
	auto const mass = 1_kg;
	auto const inertia = make_cuboid_inertia_tensor<BodyCOM> (mass, { 1_m, 1_m, 1_m });
	return std::make_unique<rb::Body> (MassMoments<BodyCOM> (mass, inertia));
}


// Verify origin/placement transforms:
nu::AutoTest t_origin ("rigid_body::Body: origin transforms", []{
	auto body = make_unit_body();

	// Initially COM at (0,0,0), origin also at (0,0,0):
	test_asserts::verify_equal ("initial COM", body->placement().position(), SpaceLength<WorldSpace> (0_m, 0_m, 0_m));
	test_asserts::verify_equal ("initial origin", body->origin<WorldSpace>(), SpaceLength<WorldSpace> (0_m, 0_m, 0_m));

	// Move COM:
	body->move_to (SpaceLength<WorldSpace> (1_m, 2_m, 3_m));
	test_asserts::verify_equal ("COM moved", body->placement().position(), SpaceLength<WorldSpace> (1_m, 2_m, 3_m));

	// Move origin explicitly:
	body->move_origin_to (SpaceLength<WorldSpace> (5_m, 5_m, 5_m));
	test_asserts::verify_equal ("origin moved", body->origin<WorldSpace>(), SpaceLength<WorldSpace> (5_m, 5_m, 5_m));
});


// Verify kinetic energy:
nu::AutoTest t_energy ("rigid_body::Body: kinetic energy", []{
	auto body = make_unit_body();

	// Give it some velocity:
	VelocityMoments<WorldSpace> vm { { 3_mps, 0_mps, 0_mps }, { 2_radps, 0_radps, 0_radps } };
	body->set_velocity_moments (vm);

	auto const e_trans = body->translational_kinetic_energy();
	auto const e_rot = body->rotational_kinetic_energy();

	test_asserts::verify ("translational energy positive", e_trans > 0_J);
	test_asserts::verify ("rotational energy positive", e_rot > 0_J);
});


// Verify broken flag:
nu::AutoTest t_broken ("rigid_body::Body: broken flag", []{
	auto body = make_unit_body();
	test_asserts::verify ("initially not broken", !body->broken());

	body->set_broken();
	test_asserts::verify ("body marked broken", body->broken());
});


// Verify bounding sphere radius:
nu::AutoTest t_bounding_sphere ("rigid_body::Body: bounding sphere radius", []{
	auto body = make_unit_body();

	test_asserts::verify_equal ("no shape -> zero radius", body->bounding_sphere_radius(), 0_m);

	body->set_shape (make_centered_cube_shape (2_m));
	test_asserts::verify_equal_with_epsilon ("centered cube radius", body->bounding_sphere_radius(), std::sqrt (3.0) * 1_m, 1e-12_m);

	auto origin_placement = Placement<BodyCOM, BodyOrigin>();
	origin_placement.set_position ({ 1_m, 0_m, 0_m });
	body->set_origin_placement (origin_placement);
	test_asserts::verify_equal_with_epsilon ("offset origin affects radius", body->bounding_sphere_radius(), std::sqrt (6.0) * 1_m, 1e-12_m);
});

} // namespace
} // namespace xf::test
