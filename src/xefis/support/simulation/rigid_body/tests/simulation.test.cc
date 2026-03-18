/* vim:ts=4
 *
 * Copyleft 2018  Michał Gawron
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
#include <xefis/support/math/tait_bryan_angles.h>
#include <xefis/support/nature/acceleration_moments.h>
#include <xefis/support/math/transforms.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/nature/mass_moments.h>
#include <xefis/support/nature/various_inertia_tensors.h>
#include <xefis/support/simulation/constraints/angular_motor_constraint.h>
#include <xefis/support/simulation/constraints/fixed_constraint.h>
#include <xefis/support/simulation/constraints/hinge_constraint.h>
#include <xefis/support/simulation/rigid_body/concepts.h>
#include <xefis/support/simulation/rigid_body/impulse_solver.h>
#include <xefis/support/simulation/rigid_body/system.h>
#include <xefis/support/simulation/rigid_body/utility.h>
#include <xefis/support/simulation/evolver.h>

// Neutrino:
#include <neutrino/test/auto_test.h>

// Standard:
#include <cstddef>
#include <memory>
#include <string>


namespace xf::test {
namespace {

namespace test_asserts = nu::test_asserts;

constexpr auto	kISSHeight = kEarthMeanRadius + 405.5_km;

nu::Logger g_null_logger;


MassMoments<BodyCOM>
unit_cuboid_mass_moments()
{
	return {
		1_kg,
		make_cuboid_inertia_tensor<BodyCOM> (1_kg, 1_m),
	};
}


MassMoments<BodyCOM>
gyroscopic_test_mass_moments()
{
	return {
		1_kg,
		make_cuboid_inertia_tensor<BodyCOM> (1_kg, { 1_m, 2_m, 3_m }),
	};
}


VelocityMoments<WorldSpace>
gyroscopic_test_velocity_moments()
{
	return { { 0_mps, 0_mps, 0_mps }, { 20_radps, 10_radps, 30_radps } };
}


std::unique_ptr<rigid_body::Body>
make_iss()
{
	SpaceLength<ECEFSpace> const iss_ecef_position = to_cartesian (si::LonLatRadius (0_deg, 0_deg, kISSHeight));
	RotationQuaternion<WorldSpace> const iss_ecef_rotation =
		math::coordinate_system_cast<WorldSpace, WorldSpace, ECEFSpace, AirframeSpace> (
			airframe_to_ecef_rotation (TaitBryanAngles { 0_deg, 0_deg, 0_deg }, iss_ecef_position)
		);
	SpaceVector<si::Velocity, WorldSpace> const iss_velocity { 0_mps, 0_mps, 27'600_kph };
	SpaceVector<si::AngularVelocity, WorldSpace> const iss_angular_velocity (math::zero);
	MassMoments<BodyCOM> const iss_mass_moments (419'725_kg, math::identity);

	auto body = std::make_unique<rigid_body::Body> (iss_mass_moments);
	auto pl = body->placement();
	pl.set_position (math::coordinate_system_cast<WorldSpace, void, ECEFSpace, void> (iss_ecef_position));
	pl.set_body_rotation (math::coordinate_system_cast<WorldSpace, BodyCOM, WorldSpace, WorldSpace> (iss_ecef_rotation));
	body->set_placement (pl);
	body->set_velocity_moments (VelocityMoments<WorldSpace> { iss_velocity, iss_angular_velocity });

	return body;
}


class TwoBodySystem
{
  public:
	// Ctor
	explicit
	TwoBodySystem (uint32_t const iterations, double const friction_factor):
		solver (system, iterations)
	{
		auto const mass_moments = MassMoments<BodyCOM> {
			1_kg,
			make_cuboid_inertia_tensor<BodyCOM> (1_kg, 1_m),
		};

		this->body_1 = &system.add<rigid_body::Body> (mass_moments);
		this->body_2 = &system.add<rigid_body::Body> (mass_moments);

		this->body_1->move_to ({ -0.5_m, 0_m, 0_m });
		this->body_2->move_to ({ +0.5_m, 0_m, 0_m });

		this->system.add<rigid_body::FixedConstraint> (*body_1, *body_2);
		this->system.set_default_baumgarte_factor (0.0);
		this->system.set_default_constraint_force_mixing_factor (0.0);
		this->system.set_default_friction_factor (friction_factor);
	}

	rigid_body::System			system;
	rigid_body::ImpulseSolver	solver;
	rigid_body::Body*			body_1;
	rigid_body::Body*			body_2;
};


class ThreeBodyFixedConstraintSystem
{
  public:
	explicit
	ThreeBodyFixedConstraintSystem (uint32_t const iterations, MassMoments<BodyCOM> const& mass_moments = unit_cuboid_mass_moments()):
		solver (system, iterations)
	{
		this->body_1 = &system.add<rigid_body::Body> (mass_moments);
		this->body_2 = &system.add<rigid_body::Body> (mass_moments);
		this->body_3 = &system.add<rigid_body::Body> (mass_moments);

		this->body_1->move_to ({ -1_m, 0_m, 0_m });
		this->body_2->move_to ({  0_m, 0_m, 0_m });
		this->body_3->move_to ({ +1_m, 0_m, 0_m });

		this->system.add<rigid_body::FixedConstraint> (*body_1, *body_2);
		this->system.add<rigid_body::FixedConstraint> (*body_2, *body_3);
		this->system.set_default_baumgarte_factor (0.0);
		this->system.set_default_constraint_force_mixing_factor (0.0);
		this->system.set_default_friction_factor (0.0);
	}

	rigid_body::System			system;
	rigid_body::ImpulseSolver	solver;
	rigid_body::Body*			body_1;
	rigid_body::Body*			body_2;
	rigid_body::Body*			body_3;
};


si::Velocity
fixed_constraint_relative_speed_after_step (double const friction_factor)
{
	auto system = TwoBodySystem (1, friction_factor);
	system.body_1->set_velocity_moments (VelocityMoments<WorldSpace> ({ -1_mps, 0_mps, 0_mps }, { 0_radps, 0_radps, 0_radps }));
	system.body_2->set_velocity_moments (VelocityMoments<WorldSpace> ({ +1_mps, 0_mps, 0_mps }, { 0_radps, 0_radps, 0_radps }));
	system.solver.evolve (1_ms);
	return abs (system.body_2->velocity_moments<WorldSpace>().velocity() - system.body_1->velocity_moments<WorldSpace>().velocity());
}


si::Velocity
fixed_constraint_relative_speed_under_load_after_step (double const friction_factor, uint32_t iterations)
{
	auto system = TwoBodySystem (iterations, friction_factor);
	system.body_1->apply_impulse (ForceMoments<WorldSpace> ({ -1_N, 0_N, 0_N }, { 0_Nm, 0_Nm, 0_Nm }));
	system.body_2->apply_impulse (ForceMoments<WorldSpace> ({ +1_N, 0_N, 0_N }, { 0_Nm, 0_Nm, 0_Nm }));
	system.solver.evolve (1_ms);
	return abs (system.body_2->velocity_moments<WorldSpace>().velocity() - system.body_1->velocity_moments<WorldSpace>().velocity());
}


si::Velocity
fixed_constraint_chain_first_link_relative_speed_after_step (uint32_t const iterations)
{
	auto system = ThreeBodyFixedConstraintSystem (iterations);
	system.body_3->apply_impulse (ForceMoments<WorldSpace> ({ +1_N, 0_N, 0_N }, { 0_Nm, 0_Nm, 0_Nm }));
	system.solver.evolve (1_ms);
	return abs (system.body_2->velocity_moments<WorldSpace>().velocity() - system.body_1->velocity_moments<WorldSpace>().velocity());
}


si::Angle
motor_angle_after_motion (double const system_baumgarte_factor, double const system_friction_factor)
{
	auto system = rigid_body::System();
	auto solver = rigid_body::ImpulseSolver (system, 100);

	auto const mass_moments = MassMoments<BodyCOM> {
		1_kg,
		make_cuboid_inertia_tensor<BodyCOM> (1_kg, 1_m),
	};

	auto& body_1 = system.add<rigid_body::Body> (mass_moments);
	auto& body_2 = system.add<rigid_body::Body> (mass_moments);

	body_1.move_to ({ 0_m, 0_m, 0_m });
	body_2.move_to ({ 0_m, 1_m, 0_m });

	auto& hinge = system.add<rigid_body::HingePrecomputation> (
		body_1,
		body_2,
		SpaceLength<BodyCOM> { 0_m, -0.5_m, 0_m },
		SpaceLength<BodyCOM> { 0_m, -0.5_m, 1_m }
	);
	system.add<rigid_body::HingeConstraint> (hinge);

	system.add<rigid_body::AngularMotorConstraint> (hinge, 180_deg / 1_s, 2_Nm);

	system.set_default_baumgarte_factor (system_baumgarte_factor);
	system.set_default_constraint_force_mixing_factor (0.0);
	system.set_default_friction_factor (system_friction_factor);

	for (auto i = 0; i < 250; ++i)
		solver.evolve (1_ms);

	return hinge.data().angle;
}


nu::AutoTest t_1 ("rigid_body::Constraint: factors inherit and can revert to system defaults", []{
	auto system = rigid_body::System();

	auto const mass_moments = unit_cuboid_mass_moments();

	auto& body_1 = system.add<rigid_body::Body> (mass_moments);
	auto& body_2 = system.add<rigid_body::Body> (mass_moments);

	system.set_default_baumgarte_factor (0.125);
	system.set_default_constraint_force_mixing_factor (0.25);
	system.set_default_friction_factor (0.5);

	auto& constraint = system.add (std::make_unique<rigid_body::FixedConstraint> (body_1, body_2));

	test_asserts::verify_equal_with_epsilon ("newly attached constraint inherits Baumgarte factor",
											 constraint.baumgarte_factor(),
											 0.125,
											 1e-12);
	test_asserts::verify_equal_with_epsilon ("newly attached constraint inherits CFM factor",
											 constraint.constraint_force_mixing_factor(),
											 0.25,
											 1e-12);
	test_asserts::verify_equal_with_epsilon ("newly attached constraint inherits friction factor",
											 constraint.friction_factor(),
											 0.5,
											 1e-12);

	system.set_default_baumgarte_factor (0.375);
	system.set_default_constraint_force_mixing_factor (0.625);
	system.set_default_friction_factor (0.75);

	test_asserts::verify_equal_with_epsilon ("inherited Baumgarte factor tracks later system changes",
											 constraint.baumgarte_factor(),
											 0.375,
											 1e-12);
	test_asserts::verify_equal_with_epsilon ("inherited CFM factor tracks later system changes",
											 constraint.constraint_force_mixing_factor(),
											 0.625,
											 1e-12);
	test_asserts::verify_equal_with_epsilon ("inherited friction factor tracks later system changes",
											 constraint.friction_factor(),
											 0.75,
											 1e-12);

	constraint.set_baumgarte_factor (0.2);
	constraint.set_constraint_force_mixing_factor (0.3);
	constraint.set_friction_factor (0.4);

	system.set_default_baumgarte_factor (0.8);
	system.set_default_constraint_force_mixing_factor (0.9);
	system.set_default_friction_factor (1.0);

	test_asserts::verify_equal_with_epsilon ("explicit Baumgarte override wins over system default",
											 constraint.baumgarte_factor(),
											 0.2,
											 1e-12);
	test_asserts::verify_equal_with_epsilon ("explicit CFM override wins over system default",
											 constraint.constraint_force_mixing_factor(),
											 0.3,
											 1e-12);
	test_asserts::verify_equal_with_epsilon ("explicit friction override wins over system default",
											 constraint.friction_factor(),
											 0.4,
											 1e-12);

	constraint.set_baumgarte_factor (std::nullopt);
	constraint.set_constraint_force_mixing_factor (std::nullopt);
	constraint.set_friction_factor (std::nullopt);

	test_asserts::verify_equal_with_epsilon ("Baumgarte factor can return to system default inheritance",
											 constraint.baumgarte_factor(),
											 0.8,
											 1e-12);
	test_asserts::verify_equal_with_epsilon ("CFM factor can return to system default inheritance",
											 constraint.constraint_force_mixing_factor(),
											 0.9,
											 1e-12);
	test_asserts::verify_equal_with_epsilon ("friction factor can return to system default inheritance",
											 constraint.friction_factor(),
											 1.0,
											 1e-12);
});


nu::AutoTest t_2 ("rigid_body::System: 90-minute simulation of gravitational forces", []{
	auto rigid_body_system = rigid_body::System();
	auto rigid_body_solver = rigid_body::ImpulseSolver (rigid_body_system);
	auto const& iss = rigid_body_system.add (make_iss());
	auto const& earth = rigid_body_system.add_gravitating (rigid_body::make_earth());

	auto const orbital_period = 92.28532_min;
	auto const interim_precision = 20_km;
	auto const final_precision = 50_m;

	SpaceLength<WorldSpace> const earth_initial_position { 0_m, 0_m, 0_m };
	SpaceLength<WorldSpace> const iss_position_1_of_4 { 0_m, 0_m, +kISSHeight };
	SpaceLength<WorldSpace> const iss_position_2_of_4 { -kISSHeight, 0_m, 0_m };
	SpaceLength<WorldSpace> const iss_position_3_of_4 { 0_m, 0_m, -kISSHeight };
	SpaceLength<WorldSpace> const iss_position_4_of_4 { +kISSHeight, 0_m, 0_m };

	auto evolver = Evolver (0_s, 1 / 50_Hz, g_null_logger, [&] (si::Time const dt) { rigid_body_solver.evolve (dt); });

	evolver.evolve (orbital_period / 4);

	test_asserts::verify_equal_with_epsilon ("ISS traveled 1/4 of distance", iss.placement().position(), iss_position_1_of_4, interim_precision);
	test_asserts::verify_equal_with_epsilon ("Earth didn't travel much", earth.placement().position(), earth_initial_position, 1_cm);

	evolver.evolve (orbital_period / 4);

	test_asserts::verify_equal_with_epsilon ("ISS traveled 2/4 of distance", iss.placement().position(), iss_position_2_of_4, interim_precision);
	test_asserts::verify_equal_with_epsilon ("Earth didn't travel much", earth.placement().position(), earth_initial_position, 1_cm);

	evolver.evolve (orbital_period / 4);

	test_asserts::verify_equal_with_epsilon ("ISS traveled 3/4 of distance", iss.placement().position(), iss_position_3_of_4, interim_precision);
	test_asserts::verify_equal_with_epsilon ("Earth didn't travel much", earth.placement().position(), earth_initial_position, 1_cm);

	evolver.evolve (orbital_period / 4);

	test_asserts::verify_equal_with_epsilon ("ISS is back at its original position", iss.placement().position(), iss_position_4_of_4, final_precision);
	test_asserts::verify_equal_with_epsilon ("Earth didn't travel much", earth.placement().position(), earth_initial_position, 1_cm);
});


nu::AutoTest t_3 ("rigid_body::Constraint: friction factor damps velocity correction", []{
	auto const relative_speed_without_friction = fixed_constraint_relative_speed_after_step (0.0);
	auto const relative_speed_with_full_friction = fixed_constraint_relative_speed_after_step (1.0);

	test_asserts::verify_equal_with_epsilon ("without friction the violating relative velocity is fully corrected",
											 relative_speed_without_friction,
											 0_mps,
											 1e-9_mps);
	test_asserts::verify_equal_with_epsilon ("with full friction the velocity correction is suppressed",
											 relative_speed_with_full_friction,
											 2_mps,
											 1e-9_mps);
});


nu::AutoTest t_4 ("rigid_body::Constraint: friction factor does not weaken external load support", []{
	for (auto iterations: { 1u, 10u, 100u, 1000u })
	{
		auto const relative_speed_without_friction = fixed_constraint_relative_speed_under_load_after_step (0.0, iterations);
		auto const relative_speed_with_full_friction = fixed_constraint_relative_speed_under_load_after_step (1.0, iterations);

		test_asserts::verify_equal_with_epsilon ("without friction the fixed constraint cancels the applied separating load",
												 relative_speed_without_friction,
												 0_mps,
												 1e-9_mps);
		test_asserts::verify_equal_with_epsilon ("full friction still lets the fixed constraint hold against external load from rest",
												 relative_speed_with_full_friction,
												 0_mps,
												 1e-9_mps);
	}
});


nu::AutoTest t_5 ("rigid_body::Constraint: friction factor does not redamp external load on later solver iterations", []{
	for (auto iterations: { 1u, 10u, 100u, 1000u })
	{
		auto const relative_speed_with_partial_friction = fixed_constraint_relative_speed_under_load_after_step (0.5, iterations);

		test_asserts::verify_equal_with_epsilon ("partial friction still lets the fixed constraint hold against external load after repeated solver iterations",
												 relative_speed_with_partial_friction,
												 0_mps,
												 1e-9_mps);
	}
});


nu::AutoTest t_6 ("rigid_body::Constraint: gyroscopic acceleration is recomputed from the frame-start velocity", []{
	auto system = ThreeBodyFixedConstraintSystem (1, gyroscopic_test_mass_moments());
	si::Time const dt = 10_ms;

	system.body_1->set_velocity_moments (gyroscopic_test_velocity_moments());
	system.body_2->set_velocity_moments (gyroscopic_test_velocity_moments());
	system.body_3->set_velocity_moments (gyroscopic_test_velocity_moments());

	auto const initial_mass_moments = system.body_2->mass_moments<WorldSpace>();
	auto const initial_velocity = system.body_2->velocity_moments<WorldSpace>();

	system.solver.evolve (dt);

	auto const& iter = system.body_2->iteration();
	auto const total_acceleration = system.body_2->acceleration_moments<WorldSpace>();
	auto const expected_acceleration = compute_acceleration_moments (
		iter.all_force_moments(),
		initial_mass_moments,
		initial_velocity
	);

	test_asserts::verify ("the middle body exercised the iterative constraint path",
						  iter.velocity_moments_updated && iter.acceleration_moments.has_value());
	test_asserts::verify_equal_with_epsilon ("constraint iterations keep linear acceleration tied to the frame-start state",
											 total_acceleration.acceleration(),
											 expected_acceleration.acceleration(),
											 1e-12_mps2);
	test_asserts::verify_equal_with_epsilon ("constraint iterations keep angular acceleration tied to the frame-start state",
											 total_acceleration.angular_acceleration(),
											 expected_acceleration.angular_acceleration(),
											 1e-12_radps2);
});


nu::AutoTest t_7 ("rigid_body::Constraint: undamped external angular step excludes gyroscopic acceleration", []{
	auto system = ThreeBodyFixedConstraintSystem (1, gyroscopic_test_mass_moments());
	si::Time const dt = 10_ms;

	system.body_1->set_velocity_moments (gyroscopic_test_velocity_moments());
	system.body_2->set_velocity_moments (gyroscopic_test_velocity_moments());
	system.body_3->set_velocity_moments (gyroscopic_test_velocity_moments());

	auto const initial_mass_moments = system.body_2->mass_moments<WorldSpace>();
	auto const initial_velocity = system.body_2->velocity_moments<WorldSpace>();

	system.solver.evolve (dt);

	auto const& iter = system.body_2->iteration();
	auto const expected_external_acceleration = compute_acceleration_moments (
		iter.external_force_moments,
		initial_mass_moments,
		initial_velocity
	);
	auto const expected_external_torque_only_acceleration = compute_acceleration_moments (
		iter.external_force_moments,
		initial_mass_moments
	);
	auto const expected_external_angular_step =
		dt * expected_external_torque_only_acceleration.angular_acceleration() / 1_rad;

	test_asserts::verify ("test setup produces non-zero gyroscopic angular acceleration in the constraint predictor path",
						  abs (expected_external_acceleration.angular_acceleration()) > 1e-12_radps2);
	test_asserts::verify_equal_with_epsilon ("constraint predictor keeps linear external step tied to the frame-start state",
											 iter.external_impulses_over_mass,
											 dt * expected_external_torque_only_acceleration.acceleration(),
											 1e-12_mps);
	test_asserts::verify_equal_with_epsilon ("constraint predictor keeps the undamped angular load channel torque-only",
											 iter.external_angular_impulses_over_inertia_tensor,
											 expected_external_angular_step,
											 1e-12 / 1_s);
});


nu::AutoTest t_8 ("rigid_body::Body: acceleration except gravity keeps non-gravitational angular acceleration", []{
	auto system = rigid_body::System();
	auto solver = rigid_body::ImpulseSolver (system, 1);
	si::Time const dt = 10_ms;

	auto& body = system.add<rigid_body::Body> (gyroscopic_test_mass_moments());
	body.set_velocity_moments (gyroscopic_test_velocity_moments());

	solver.evolve (dt);

	auto const total_acceleration = body.acceleration_moments<WorldSpace>();
	auto const acceleration_except_gravity = body.acceleration_moments_except_gravity<WorldSpace>();

	test_asserts::verify ("test setup produces non-zero torque-free gyroscopic angular acceleration",
						  abs (total_acceleration.angular_acceleration()) > 1e-12_radps2);
	test_asserts::verify_equal_with_epsilon ("without gravity the body reports the same specific linear acceleration as total acceleration",
											 acceleration_except_gravity.acceleration(),
											 total_acceleration.acceleration(),
											 1e-12_mps2);
	test_asserts::verify_equal_with_epsilon ("without gravity the body keeps the same angular acceleration",
											 acceleration_except_gravity.angular_acceleration(),
											 total_acceleration.angular_acceleration(),
											 1e-12_radps2);
});


nu::AutoTest t_9 ("rigid_body::Constraint: solver iterations propagate corrections across a constraint chain", []{
	si::Velocity previous_relative_speed = 10_mps;

	for (auto iterations: { 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 20u, 1000u })
	{
		auto const context = std::format ("{} iterations", iterations);
		auto const relative_speed = fixed_constraint_chain_first_link_relative_speed_after_step (iterations);

		test_asserts::verify ("many iterations reduce the first-link residual in a coupled constraint chain; " + context,
							  relative_speed < previous_relative_speed);
		test_asserts::verify_equal_with_epsilon ("many iterations can propagate the load correction back to the first link",
												 relative_speed,
												 0_mps,
												 1e-3_mps / iterations);

		previous_relative_speed = relative_speed;
	}
});


nu::AutoTest t_10 ("rigid_body::AngularMotorConstraint: motor drive still moves with zero Baumgarte", []{
	auto const angle_with_zero_baumgarte = motor_angle_after_motion (0.0, 0.0);

	test_asserts::verify ("with zero Baumgarte the motor still rotates the hinge toward its commanded velocity",
						  angle_with_zero_baumgarte > 10_deg);
});


nu::AutoTest t_11 ("rigid_body::AngularMotorConstraint: motor drive ignores solver friction damping", []{
	auto const angle_without_friction = motor_angle_after_motion (0.0, 0.0);
	auto const angle_with_full_friction = motor_angle_after_motion (0.0, 1.0);

	test_asserts::verify_equal_with_epsilon ("solver friction does not change the motor's driven hinge motion",
											 angle_with_full_friction,
											 angle_without_friction,
											 1_deg);
});

} // namespace
} // namespace xf::test
