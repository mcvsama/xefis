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
#include <string>


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


nu::AutoTest t_physical_body_tests ("rigid_body::Body: Physical behavior verification", []{
	using namespace xf::rigid_body;

	auto const mass_moments = MassMoments<BodyCOM> (100_kg, make_cuboid_inertia_tensor<BodyCOM> (100_kg, { 1_m, 1_m, 1_m }));
	auto const different_mass_moments = MassMoments<BodyCOM> (200_kg, make_cuboid_inertia_tensor<BodyCOM> (200_kg, { 1_m, 1_m, 1_m }));

	// Verify mass moment storage and retrieval:
	Body test_body (mass_moments);
	test_asserts::verify_equal_with_epsilon ("Body correctly stores mass",
											 test_body.mass_moments<BodyCOM>().mass(),
											 mass_moments.mass(),
											 1e-12_kg);
	test_asserts::verify_equal_with_epsilon ("Body correctly stores inertia tensor",
											 test_body.mass_moments<BodyCOM>().inertia_tensor(),
											 mass_moments.inertia_tensor(),
											 1e-12_kgm2);

	// Check translation:
	SpaceLength<WorldSpace> translation { 10_m, 0_m, 0_m };
	test_body.translate (translation);
	test_asserts::verify_equal ("Body translates correctly", test_body.placement().position(), translation);

	// Check setting velocity:
	VelocityMoments<WorldSpace> velocity { { 5_mps, 0_mps, 0_mps }, { 0_radps, 0_radps, 0_radps } };
	test_body.set_velocity_moments (velocity);
	test_asserts::verify_equal_with_epsilon ("Body correctly stores velocity",
											 test_body.velocity_moments<WorldSpace>().velocity(),
											 velocity.velocity(),
											 1e-12_mps);
	test_asserts::verify_equal_with_epsilon ("Body correctly stores angular velocity",
											 test_body.velocity_moments<WorldSpace>().angular_velocity(),
											 velocity.angular_velocity(),
											 1e-12_radps);

	// Check impulse application (simple linear force):
	ForceMoments<WorldSpace> impulse ({ 100_N, 0_N, 0_N }, { 0_Nm, 0_Nm, 0_Nm });
	test_body.apply_impulse (impulse);
	test_asserts::verify_equal_with_epsilon ("Impulse force is applied correctly",
											 test_body.external_force_moments<WorldSpace>().force(),
											 impulse.force(),
											 1e-12_N);
	test_asserts::verify_equal_with_epsilon ("Impulse torque is applied correctly",
											 test_body.external_force_moments<WorldSpace>().torque(),
											 impulse.torque(),
											 1e-12_Nm);

	// Check reset of applied impulses:
	test_body.reset_applied_impulses();
	test_asserts::verify_equal_with_epsilon ("Applied impulse force is reset",
											 test_body.external_force_moments<WorldSpace>().force(),
											 SpaceForce<WorldSpace> { 0_N, 0_N, 0_N },
											 1e-12_N);
	test_asserts::verify_equal_with_epsilon ("Applied impulse torque is reset",
											 test_body.external_force_moments<WorldSpace>().torque(),
											 SpaceTorque<WorldSpace> { 0_Nm, 0_Nm, 0_Nm },
											 1e-12_Nm);

	// Verify rotation about center-of-mass:
	auto const rotation = quaternion_rotation_about<WorldSpace> ({ 0.0, 1.0, 0.0 }, 90_deg);
	test_body.rotate_about_center_of_mass (rotation);
	test_asserts::verify_equal_with_epsilon ("Rotation about center-of-mass updates placement",
											 to_rotation_vector (test_body.placement().body_rotation()),
											 to_rotation_vector (rotation),
											 1e-12_rad);

	// Verify impulse applied at offset position (torque generation):
	SpaceLength<WorldSpace> force_position { 1_m, 0_m, 0_m };
	test_body.apply_impulse (impulse, force_position);
	test_asserts::verify ("Impulse applied at offset generates torque", abs (test_body.external_force_moments<WorldSpace>().torque()) > 0_Nm);

	// Verify kinetic energy calculation:
	si::Energy kinetic_energy = test_body.translational_kinetic_energy() + test_body.rotational_kinetic_energy();
	test_asserts::verify ("Kinetic energy is positive", kinetic_energy > 0_J);

	// Simulate time evolution:
	si::Energy const initial_kinetic_energy = kinetic_energy;
	test_body.evolve (1_s);
	si::Energy const final_kinetic_energy = test_body.translational_kinetic_energy() + test_body.rotational_kinetic_energy();
	test_asserts::verify_equal_with_epsilon ("Kinetic energy is conserved", initial_kinetic_energy, final_kinetic_energy, 1e-12_J);

	// Verify setting new mass moments:
	test_body.set_mass_moments (different_mass_moments);
	test_asserts::verify_equal_with_epsilon ("New mass is stored correctly",
											 test_body.mass_moments<BodyCOM>().mass(),
											 different_mass_moments.mass(),
											 1e-12_kg);
	test_asserts::verify_equal_with_epsilon ("New inertia tensor is stored correctly",
											 test_body.mass_moments<BodyCOM>().inertia_tensor(),
											 different_mass_moments.inertia_tensor(),
											 1e-12_kgm2);

	// Verify breaking the body:
	test_asserts::verify ("Body is initially not broken", !test_body.broken());
	test_body.set_broken();
	test_asserts::verify ("Body is marked as broken", test_body.broken());
});


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


// Verify origin/placement transforms:
nu::AutoTest t_rotations ("rigid_body::Body: rotations", []{
	auto body = make_unit_body();
	auto origin_placement = Placement<BodyCOM, BodyOrigin>();
	origin_placement.set_position ({ 1_m, 0_m, 0_m });
	body->set_origin_placement (origin_placement);
	body->move_to (SpaceLength<WorldSpace> (2_m, 3_m, 4_m));

	auto const rotation = z_rotation<WorldSpace> (90_deg);
	auto const inverse_rotation = z_rotation<WorldSpace> (-90_deg);

	test_asserts::verify_equal ("initial COM", body->placement().position(), SpaceLength<WorldSpace> (2_m, 3_m, 4_m));
	test_asserts::verify_equal ("initial origin", body->origin<WorldSpace>(), SpaceLength<WorldSpace> (3_m, 3_m, 4_m));

	body->rotate_about_center_of_mass (rotation);
	test_asserts::verify_equal_with_epsilon ("rotation about COM keeps COM fixed",
											 body->placement().position(),
											 SpaceLength<WorldSpace> (2_m, 3_m, 4_m),
											 1e-12_m);
	test_asserts::verify_equal_with_epsilon ("rotation about COM rotates origin around COM",
											 body->origin<WorldSpace>(),
											 SpaceLength<WorldSpace> (2_m, 4_m, 4_m),
											 1e-12_m);
	test_asserts::verify_equal_with_epsilon ("rotation about COM updates body rotation",
											 to_rotation_vector (body->placement().body_rotation()),
											 to_rotation_vector (rotation),
											 1e-12_rad);

	body->rotate_about_center_of_mass (inverse_rotation);
	test_asserts::verify_equal_with_epsilon ("inverse COM rotation restores COM",
											 body->placement().position(),
											 SpaceLength<WorldSpace> (2_m, 3_m, 4_m),
											 1e-12_m);
	test_asserts::verify_equal_with_epsilon ("inverse COM rotation restores origin",
											 body->origin<WorldSpace>(),
											 SpaceLength<WorldSpace> (3_m, 3_m, 4_m),
											 1e-12_m);
	test_asserts::verify_equal_with_epsilon ("inverse COM rotation restores body rotation",
											 to_rotation_vector (body->placement().body_rotation()),
											 SpaceVector<si::Angle, WorldSpace> { 0_rad, 0_rad, 0_rad },
											 1e-12_rad);

	body->rotate_about_body_origin (rotation);
	test_asserts::verify_equal_with_epsilon ("rotation about body origin moves COM around origin",
											 body->placement().position(),
											 SpaceLength<WorldSpace> (3_m, 2_m, 4_m),
											 1e-12_m);
	test_asserts::verify_equal_with_epsilon ("rotation about body origin keeps origin fixed",
											 body->origin<WorldSpace>(),
											 SpaceLength<WorldSpace> (3_m, 3_m, 4_m),
											 1e-12_m);
	test_asserts::verify_equal_with_epsilon ("rotation about body origin updates body rotation",
											 to_rotation_vector (body->placement().body_rotation()),
											 to_rotation_vector (rotation),
											 1e-12_rad);

	body->rotate_about_body_origin (inverse_rotation);
	test_asserts::verify_equal_with_epsilon ("inverse body-origin rotation restores COM",
											 body->placement().position(),
											 SpaceLength<WorldSpace> (2_m, 3_m, 4_m),
											 1e-12_m);
	test_asserts::verify_equal_with_epsilon ("inverse body-origin rotation restores origin",
											 body->origin<WorldSpace>(),
											 SpaceLength<WorldSpace> (3_m, 3_m, 4_m),
											 1e-12_m);
	test_asserts::verify_equal_with_epsilon ("inverse body-origin rotation restores body rotation",
											 to_rotation_vector (body->placement().body_rotation()),
											 SpaceVector<si::Angle, WorldSpace> { 0_rad, 0_rad, 0_rad },
											 1e-12_rad);

	body->rotate_about_world_origin (rotation);
	test_asserts::verify_equal_with_epsilon ("rotation about world origin rotates COM around world origin",
											 body->placement().position(),
											 SpaceLength<WorldSpace> (-3_m, 2_m, 4_m),
											 1e-12_m);
	test_asserts::verify_equal_with_epsilon ("rotation about world origin rotates origin around world origin",
											 body->origin<WorldSpace>(),
											 SpaceLength<WorldSpace> (-3_m, 3_m, 4_m),
											 1e-12_m);
	test_asserts::verify_equal_with_epsilon ("rotation about world origin updates body rotation",
											 to_rotation_vector (body->placement().body_rotation()),
											 to_rotation_vector (rotation),
											 1e-12_rad);

	body->rotate_about_world_origin (inverse_rotation);
	test_asserts::verify_equal_with_epsilon ("inverse world-origin rotation restores COM",
											 body->placement().position(),
											 SpaceLength<WorldSpace> (2_m, 3_m, 4_m),
											 1e-12_m);
	test_asserts::verify_equal_with_epsilon ("inverse world-origin rotation restores origin",
											 body->origin<WorldSpace>(),
											 SpaceLength<WorldSpace> (3_m, 3_m, 4_m),
											 1e-12_m);
	test_asserts::verify_equal_with_epsilon ("inverse world-origin rotation restores body rotation",
											 to_rotation_vector (body->placement().body_rotation()),
											 SpaceVector<si::Angle, WorldSpace> { 0_rad, 0_rad, 0_rad },
											 1e-12_rad);

	body->move_to (SpaceLength<WorldSpace> (10_m, 20_m, 30_m));
	test_asserts::verify_equal ("move_to sets COM position", body->placement().position(), SpaceLength<WorldSpace> (10_m, 20_m, 30_m));
	test_asserts::verify_equal ("move_to keeps origin offset", body->origin<WorldSpace>(), SpaceLength<WorldSpace> (11_m, 20_m, 30_m));

	origin_placement.set_position ({ 0_m, 2_m, 0_m });
	body->set_origin_placement (origin_placement);
	test_asserts::verify_equal ("set_origin_placement changes world-space origin", body->origin<WorldSpace>(), SpaceLength<WorldSpace> (10_m, 22_m, 30_m));

	body->rotate_about_center_of_mass (rotation);
	test_asserts::verify_equal_with_epsilon ("updated origin placement rotates around COM",
											 body->origin<WorldSpace>(),
											 SpaceLength<WorldSpace> (8_m, 20_m, 30_m),
											 1e-12_m);
	test_asserts::verify_equal_with_epsilon ("updated origin placement still keeps COM fixed",
											 body->placement().position(),
											 SpaceLength<WorldSpace> (10_m, 20_m, 30_m),
											 1e-12_m);
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


nu::AutoTest t_body_rotate_about_updates_rotated_state ("rigid_body::Body: rotate_about updates rotated state", []{
	auto const mass = 2_kg;
	auto const inertia_at_com = make_cuboid_inertia_tensor<BodyCOM> (mass, { 1_m, 2_m, 3_m });
	auto body = rb::Body (MassMoments<BodyCOM> (mass, inertia_at_com));

	body.move_to (SpaceLength<WorldSpace> { 1_m, 0_m, 0_m });
	body.set_velocity_moments (VelocityMoments<WorldSpace> ({ 1_mps, 0_mps, 0_mps }, { 0_radps, 0_radps, 2_radps }));
	body.set_acceleration_moments (AccelerationMoments<WorldSpace> ({ 0_mps2, 3_mps2, 0_mps2 }, { 0_radps2, 0_radps2, 4_radps2 }));

	// Prime cached world-space mass moments before rotating.
	auto const cached_world_mass_moments = body.mass_moments<WorldSpace>();
	auto const rotation = z_rotation<WorldSpace> (90_deg);

	body.rotate_about (SpaceLength<WorldSpace> { 0_m, 0_m, 0_m }, rotation);

	auto const rotated_world_mass_moments = body.mass_moments<WorldSpace>();
	auto const expected_world_mass_moments = body.placement().rotate_to_base (body.mass_moments<BodyCOM>());

	test_asserts::verify_equal_with_epsilon ("COM position rotates around the requested point",
											 body.placement().position(),
											 SpaceLength<WorldSpace> { 0_m, 1_m, 0_m },
											 1e-12_m);
	test_asserts::verify_equal_with_epsilon ("world velocity rotates with body motion",
											 body.velocity_moments<WorldSpace>().velocity(),
											 SpaceVector<si::Velocity, WorldSpace> { 0_mps, 1_mps, 0_mps },
											 1e-12_mps);
	test_asserts::verify_equal_with_epsilon ("world acceleration rotates with body motion",
											 body.acceleration_moments<WorldSpace>().acceleration(),
											 SpaceVector<si::Acceleration, WorldSpace> { -3_mps2, 0_mps2, 0_mps2 },
											 1e-12_mps2);
	test_asserts::verify ("world inertia cache is refreshed after rotation",
						  abs (cached_world_mass_moments.inertia_tensor()[0, 0] - rotated_world_mass_moments.inertia_tensor()[0, 0]) > 1e-12_kgm2);
	test_asserts::verify_equal_with_epsilon ("world inertia matches rotated body inertia",
											 rotated_world_mass_moments.inertia_tensor(),
											 expected_world_mass_moments.inertia_tensor(),
											 1e-12_kgm2);
});


nu::AutoTest t_body_specific_rotations_refresh_world_mass_moments_cache ("rigid_body::Body: specific rotations refresh world mass moments cache", []{
	auto verify_rotation = [] (std::string const& label, auto&& rotate) {
		auto const mass = 2_kg;
		auto const inertia_at_com = make_cuboid_inertia_tensor<BodyCOM> (mass, { 1_m, 2_m, 3_m });
		auto body = rb::Body (MassMoments<BodyCOM> (mass, inertia_at_com));

		auto origin_placement = Placement<BodyCOM, BodyOrigin>();
		origin_placement.set_position ({ 1_m, 0_m, 0_m });
		body.set_origin_placement (origin_placement);
		body.move_to (SpaceLength<WorldSpace> { 2_m, 3_m, 4_m });

		auto const cached_world_mass_moments = body.mass_moments<WorldSpace>();
		auto const rotation = z_rotation<WorldSpace> (90_deg);

		rotate (body, rotation);

		auto const rotated_world_mass_moments = body.mass_moments<WorldSpace>();
		auto const expected_world_mass_moments = body.placement().rotate_to_base (body.mass_moments<BodyCOM>());

		test_asserts::verify (label + ": world inertia cache is refreshed",
							  abs (cached_world_mass_moments.inertia_tensor()[0, 0] - rotated_world_mass_moments.inertia_tensor()[0, 0]) > 1e-12_kgm2);
		test_asserts::verify_equal_with_epsilon (label + ": world inertia matches rotated body inertia",
												 rotated_world_mass_moments.inertia_tensor(),
												 expected_world_mass_moments.inertia_tensor(),
												 1e-12_kgm2);
	};

	verify_rotation ("rotate_about_center_of_mass", [] (rb::Body& body, auto const& rotation) {
		body.rotate_about_center_of_mass (rotation);
	});

	verify_rotation ("rotate_about_world_origin", [] (rb::Body& body, auto const& rotation) {
		body.rotate_about_world_origin (rotation);
	});

	verify_rotation ("rotate_about_body_origin", [] (rb::Body& body, auto const& rotation) {
		body.rotate_about_body_origin (rotation);
	});
});


nu::AutoTest t_body_set_mass_moments_refreshes_world_cache ("rigid_body::Body: set_mass_moments refreshes world cache", []{
	auto const initial_mass = 2_kg;
	auto const initial_inertia = make_cuboid_inertia_tensor<BodyCOM> (initial_mass, { 1_m, 2_m, 3_m });
	auto body = rb::Body (MassMoments<BodyCOM> (initial_mass, initial_inertia));
	body.rotate_about_center_of_mass (z_rotation<WorldSpace> (90_deg));

	auto const cached_initial_world_mass_moments = body.mass_moments<WorldSpace>();

	auto const updated_mass = 3_kg;
	auto const updated_inertia = make_cuboid_inertia_tensor<BodyCOM> (updated_mass, { 4_m, 5_m, 6_m });
	body.set_mass_moments (MassMoments<BodyCOM> (updated_mass, updated_inertia));

	auto const refreshed_world_mass_moments = body.mass_moments<WorldSpace>();
	auto const expected_world_mass_moments = body.placement().rotate_to_base (body.mass_moments<BodyCOM>());

	test_asserts::verify ("direct overload replaces cached world inertia",
						  abs (cached_initial_world_mass_moments.inertia_tensor()[0, 0] - refreshed_world_mass_moments.inertia_tensor()[0, 0]) > 1e-12_kgm2);
	test_asserts::verify_equal_with_epsilon ("direct overload updates world-space mass moments",
											 refreshed_world_mass_moments.inertia_tensor(),
											 expected_world_mass_moments.inertia_tensor(),
											 1e-12_kgm2);

	auto const offset = SpaceLength<BodyCOM> { 2_m, 0_m, 0_m };
	auto const offset_inertia_at_com = make_cuboid_inertia_tensor<BodyCOM> (updated_mass, { 2_m, 3_m, 4_m });
	auto const offset_inertia_at_origin = inertia_tensor_com_to_point (updated_mass, offset_inertia_at_com, offset);
	body.set_mass_moments (MassMomentsAtArm<BodyCOM> (updated_mass, offset, offset_inertia_at_origin));

	auto const refreshed_offset_world_mass_moments = body.mass_moments<WorldSpace>();
	auto const expected_offset_world_mass_moments = body.placement().rotate_to_base (body.mass_moments<BodyCOM>());

	test_asserts::verify_equal_with_epsilon ("at-arm overload updates COM placement",
											 body.placement().position(),
											 SpaceLength<WorldSpace> { 0_m, 2_m, 0_m },
											 1e-12_m);
	test_asserts::verify_equal_with_epsilon ("at-arm overload updates world-space mass moments",
											 refreshed_offset_world_mass_moments.inertia_tensor(),
											 expected_offset_world_mass_moments.inertia_tensor(),
											 1e-12_kgm2);
});

} // namespace
} // namespace xf::test
