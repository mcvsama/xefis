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
#include <xefis/support/math/geometry.h>
#include <xefis/support/nature/mass_moments_at_arm.h>
#include <xefis/support/nature/mass_moments.h>
#include <xefis/support/nature/various_inertia_tensors.h>
#include <xefis/support/simulation/constraints/fixed_constraint.h>
#include <xefis/support/simulation/rigid_body/group.h>
#include <xefis/support/simulation/rigid_body/impulse_solver.h>
#include <xefis/support/simulation/rigid_body/system.h>

// Neutrino:
#include <neutrino/test/auto_test.h>

// Standard:
#include <algorithm>
#include <array>
#include <cmath>
#include <functional>


namespace xf::test {
namespace {

namespace rb = rigid_body;
namespace test_asserts = nu::test_asserts;

constexpr auto kHubMass = 0.1_kg;
constexpr auto kAxialMass = 0.2_kg;
constexpr auto kEquatorialMass = 0.5_kg;
constexpr auto kBodyEdgeLength = 5_cm;
constexpr auto kAxialArm = 50_cm;
constexpr auto kEquatorialArm = 40_cm;
constexpr uint32_t kSolverIterations = 1;
constexpr uint32_t kConstrainedTopSolverIterations = 20;


struct OneBodySymmetricTop
{
	rb::Body*	body;
};


struct MultiBodySymmetricTop
{
	rb::Group*	group;
	rb::Body*	upper_axial_mass;
	rb::Body*	lower_axial_mass;
};


struct EquivalentPrecessionPhase
{
	si::Angle	multi_body_phase;
	si::Angle	one_body_phase;
};


[[nodiscard]]
MassMoments<BodyCOM>
make_lumped_mass_moments (si::Mass const mass)
{
	return {
		mass,
		make_cuboid_inertia_tensor<BodyCOM> (mass, kBodyEdgeLength),
	};
}


[[nodiscard]]
MassMomentsAtArm<BodyCOM>
make_lumped_mass_moments_at_arm (si::Mass const mass, SpaceLength<BodyCOM> const& position)
{
	auto const inertia_tensor_at_com = make_cuboid_inertia_tensor<BodyCOM> (mass, kBodyEdgeLength);
	auto const inertia_tensor_at_origin = inertia_tensor_com_to_point (mass, inertia_tensor_at_com, position);

	return { mass, position, inertia_tensor_at_origin };
}


[[nodiscard]]
OneBodySymmetricTop
make_one_body_symmetric_top (rb::System& system)
{
	auto const top_mass_moments = compute_mass_moments_at_arm<BodyCOM> ({
		make_lumped_mass_moments_at_arm (kHubMass, { 0_m, 0_m, 0_m }),
		make_lumped_mass_moments_at_arm (kAxialMass, { 0_m, 0_m, +kAxialArm }),
		make_lumped_mass_moments_at_arm (kAxialMass, { 0_m, 0_m, -kAxialArm }),
		make_lumped_mass_moments_at_arm (kEquatorialMass, { +kEquatorialArm, 0_m, 0_m }),
		make_lumped_mass_moments_at_arm (kEquatorialMass, { -kEquatorialArm, 0_m, 0_m }),
		make_lumped_mass_moments_at_arm (kEquatorialMass, { 0_m, +kEquatorialArm, 0_m }),
		make_lumped_mass_moments_at_arm (kEquatorialMass, { 0_m, -kEquatorialArm, 0_m }),
	}).centered_at_center_of_mass();
	auto& body = system.add<rb::Body> (top_mass_moments);

	return {
		.body = &body,
	};
}


[[nodiscard]]
MultiBodySymmetricTop
make_multi_body_symmetric_top (rb::System& system)
{
	auto& group = system.make_group ("precession top");

	auto& hub = group.add<rb::Body> (make_lumped_mass_moments (kHubMass));
	auto& upper_axial_mass = group.add<rb::Body> (make_lumped_mass_moments (kAxialMass));
	auto& lower_axial_mass = group.add<rb::Body> (make_lumped_mass_moments (kAxialMass));
	auto& x_positive = group.add<rb::Body> (make_lumped_mass_moments (kEquatorialMass));
	auto& x_negative = group.add<rb::Body> (make_lumped_mass_moments (kEquatorialMass));
	auto& y_positive = group.add<rb::Body> (make_lumped_mass_moments (kEquatorialMass));
	auto& y_negative = group.add<rb::Body> (make_lumped_mass_moments (kEquatorialMass));

	upper_axial_mass.move_to ({ 0_m, 0_m, +kAxialArm });
	lower_axial_mass.move_to ({ 0_m, 0_m, -kAxialArm });
	x_positive.move_to ({ +kEquatorialArm, 0_m, 0_m });
	x_negative.move_to ({ -kEquatorialArm, 0_m, 0_m });
	y_positive.move_to ({ 0_m, +kEquatorialArm, 0_m });
	y_negative.move_to ({ 0_m, -kEquatorialArm, 0_m });

	auto const bodies = std::array {
		&hub,
		&upper_axial_mass,
		&lower_axial_mass,
		&x_positive,
		&x_negative,
		&y_positive,
		&y_negative,
	};

	for (auto i = std::size_t { 0 }; i < bodies.size(); ++i)
		for (auto j = i + 1; j < bodies.size(); ++j)
			system.add<rb::FixedConstraint> (*bodies[i], *bodies[j]);

	return {
		.group = &group,
		.upper_axial_mass = &upper_axial_mass,
		.lower_axial_mass = &lower_axial_mass,
	};
}


struct EquivalentSymmetricTops
{
	rb::System				multi_body_system;
	MultiBodySymmetricTop	multi_body_top;
	rb::System				one_body_system;
	OneBodySymmetricTop		one_body_top;

	EquivalentSymmetricTops():
		multi_body_top (make_multi_body_symmetric_top (multi_body_system)),
		one_body_top (make_one_body_symmetric_top (one_body_system))
	{ }
};


void
evolve (rb::System& system,
		si::Time duration,
		si::Time max_dt,
		uint32_t const solver_iterations = kSolverIterations,
		double const friction_factor = 0.0,
		std::function<void (si::Time)> const& before_step = {},
		std::function<void (si::Time)> const& after_step = {})
{
	rb::ImpulseSolver solver (system, solver_iterations);
	system.set_default_baumgarte_factor (0.5);
	system.set_default_constraint_force_mixing_factor (0.0);
	system.set_default_friction_factor (friction_factor);

	for (auto remaining = duration; remaining > 0_s; )
	{
		auto const dt = std::min (remaining, max_dt);
		if (before_step)
			before_step (dt);

		solver.evolve (dt);

		if (after_step)
			after_step (dt);

		remaining -= dt;
	}
}


void
set_top_rotation (OneBodySymmetricTop const& top, SpaceVector<si::AngularVelocity, WorldSpace> const& angular_velocity)
{
	top.body->set_velocity_moments (VelocityMoments<WorldSpace> {
		{ 0_mps, 0_mps, 0_mps },
		angular_velocity,
	});
}


void
set_top_rotation (MultiBodySymmetricTop const& top, SpaceVector<si::AngularVelocity, WorldSpace> const& angular_velocity)
{
	auto const center_of_mass = top.group->mass_moments().center_of_mass_position();

	for (auto* body: top.group->bodies())
	{
		auto const arm = body->placement().position() - center_of_mass;
		body->set_velocity_moments (VelocityMoments<WorldSpace> {
			tangential_velocity (angular_velocity, arm),
			angular_velocity,
		});
	}
}


[[nodiscard]]
SpaceVector<double, WorldSpace>
top_axis (OneBodySymmetricTop const& top)
{
	return top.body->placement().rotate_to_base (SpaceVector<double, BodyCOM> { 0.0, 0.0, 1.0 });
}


[[nodiscard]]
SpaceVector<double, WorldSpace>
top_axis (MultiBodySymmetricTop const& top)
{
	return normalized_direction_or_zero (top.upper_axial_mass->placement().position() - top.lower_axial_mass->placement().position());
}


[[nodiscard]]
si::Angle
tilt_about_x (OneBodySymmetricTop const& top)
{
	auto const axis = top_axis (top);
	return std::atan2 (axis[1], axis[2]) * 1_rad;
}


template<math::CoordinateSystem Space>
	[[nodiscard]]
	SpaceVector<si::AngularMomentum, Space>
	angular_momentum (MassMoments<Space> const& mass_moments, SpaceVector<si::AngularVelocity, Space> const& angular_velocity)
	{
		return mass_moments.inertia_tensor() * angular_velocity / 1_rad;
	}


[[nodiscard]]
si::Angle
signed_rotation_about (SpaceVector<double, WorldSpace> const& initial_vector,
					   SpaceVector<double, WorldSpace> const& final_vector,
					   SpaceVector<double, WorldSpace> const& axis)
{
	auto const initial_projection = normalized_direction_or_zero (initial_vector - dot_product (initial_vector, axis) * axis);
	auto const final_projection = normalized_direction_or_zero (final_vector - dot_product (final_vector, axis) * axis);
	auto const sine = dot_product (cross_product (initial_projection, final_projection), axis);
	auto const cosine = dot_product (initial_projection, final_projection);

	return std::atan2 (sine, cosine) * 1_rad;
}


template<class Top>
	[[nodiscard]]
	si::Angle
	accumulated_rotation_about (rb::System& system,
								Top const& top,
								SpaceVector<double, WorldSpace> const& axis,
								si::Time duration,
								si::Time max_dt,
								uint32_t const solver_iterations = kSolverIterations,
								double const friction_factor = 0.0)
	{
		auto accumulated_phase = 0_rad;
		auto previous_axis = top_axis (top);

		evolve (system, duration, max_dt, solver_iterations, friction_factor, {}, [&] (si::Time) {
			auto const current_axis = top_axis (top);
			accumulated_phase += signed_rotation_about (previous_axis, current_axis, axis);
			previous_axis = current_axis;
		});

		return accumulated_phase;
	}


[[nodiscard]]
EquivalentPrecessionPhase
measure_equivalent_torque_free_precession_phase(
	EquivalentSymmetricTops& tops,
	SpaceVector<si::AngularVelocity, WorldSpace> const& initial_angular_velocity,
	SpaceVector<double, WorldSpace> const& angular_momentum_direction,
	si::Time duration,
	si::Time dt,
	double const friction_factor = 0.0
) {
	set_top_rotation (tops.multi_body_top, initial_angular_velocity);
	set_top_rotation (tops.one_body_top, initial_angular_velocity);

	return {
		.multi_body_phase = accumulated_rotation_about (
			tops.multi_body_system,
			tops.multi_body_top,
			angular_momentum_direction,
			duration,
			dt,
			kConstrainedTopSolverIterations,
			friction_factor
		),
		.one_body_phase = accumulated_rotation_about (
			tops.one_body_system,
			tops.one_body_top,
			angular_momentum_direction,
			duration,
			dt,
			kSolverIterations,
			friction_factor
		),
	};
}


nu::AutoTest t_1 ("rigid_body::System: torque-induced precession", []{
	// <https://en.wikipedia.org/wiki/Precession#Torque-induced>

	rb::System system;
	auto const top = make_one_body_symmetric_top (system);
	auto const centered_mass_moments = top.body->mass_moments<BodyCOM>();
	auto const I3 = centered_mass_moments.inertia_tensor()[2, 2];
	auto const spin = 40_radps;
	auto const force = 6_N;
	auto const duration = 200_ms;
	auto const dt = 10_us;
	si::Torque const applied_torque = 2.0 * kAxialArm * force;
	auto const angular_momentum = I3 * spin / 1_rad;
	auto const expected_precession_rate = applied_torque * 1_rad / angular_momentum;
	si::Angle const expected_tilt = expected_precession_rate * duration;

	set_top_rotation (top, { 0_radps, 0_radps, spin });

	evolve (system, duration, dt, kSolverIterations, 0.0, [&] (si::Time) {
		top.body->apply_impulse (ForceMoments<WorldSpace> ({ +force, 0_N, 0_N }, { 0_Nm, 0_Nm, 0_Nm }), SpaceLength<BodyCOM> { 0_m, 0_m, +kAxialArm });
		top.body->apply_impulse (ForceMoments<WorldSpace> ({ -force, 0_N, 0_N }, { 0_Nm, 0_Nm, 0_Nm }), SpaceLength<BodyCOM> { 0_m, 0_m, -kAxialArm });
	});

	auto const measured_tilt = tilt_about_x (top);

	test_asserts::verify_equal_with_epsilon ("Torque-induced precession rate matches tau / L for short times",
											 measured_tilt,
											 expected_tilt,
											 0.04_rad);
});


nu::AutoTest t_2 ("rigid_body::System: torque-free precession", []{
	// <https://en.wikipedia.org/wiki/Precession#Torque-free>

	rb::System system;
	auto const top = make_one_body_symmetric_top (system);
	auto const centered_mass_moments = top.body->mass_moments<BodyCOM>();
	auto const I1 = centered_mass_moments.inertia_tensor()[0, 0];
	auto const I3 = centered_mass_moments.inertia_tensor()[2, 2];
	auto const initial_angular_velocity = SpaceVector<si::AngularVelocity, WorldSpace> { 3_radps, 0_radps, 12_radps };
	auto const duration = 200_ms;
	auto const dt = 10_us;
	auto const initial_angular_momentum = angular_momentum (top.body->mass_moments<WorldSpace>(), initial_angular_velocity);
	auto const angular_momentum_direction = normalized_direction_or_zero (initial_angular_momentum);
	auto const expected_precession_rate = abs (initial_angular_momentum) * 1_rad / I1;
	si::Angle const expected_phase = expected_precession_rate * duration;

	set_top_rotation (top, initial_angular_velocity);

	auto const measured_phase = accumulated_rotation_about (system, top, angular_momentum_direction, duration, dt);

	test_asserts::verify ("Symmetric top really has distinct axial inertia", abs (I3 - I1) > 0.01_kgm2);
	test_asserts::verify_equal_with_epsilon ("Torque-free precession rate matches the space-frame symmetric-top formula",
											 measured_phase,
											 expected_phase,
											 0.08_rad);
});


nu::AutoTest t_3 ("rigid_body::System: constrained torque-free precession matches the equivalent composite body", []{
	EquivalentSymmetricTops tops;
	auto const initial_angular_velocity = SpaceVector<si::AngularVelocity, WorldSpace> { 3_radps, 0_radps, 12_radps };
	auto const duration = 100_ms;
	auto const dt = 50_us;
	auto const multi_body_mass_moments = tops.multi_body_top.group->mass_moments().centered_at_center_of_mass();
	auto const one_body_mass_moments = tops.one_body_top.body->mass_moments<WorldSpace>();
	auto const angular_momentum_direction =
		normalized_direction_or_zero (angular_momentum (one_body_mass_moments, initial_angular_velocity));
	auto const measured_phase = measure_equivalent_torque_free_precession_phase (
		tops,
		initial_angular_velocity,
		angular_momentum_direction,
		duration,
		dt
	);

	test_asserts::verify_equal_with_epsilon ("constrained and composite tops use the same total inertia tensor",
											 multi_body_mass_moments.inertia_tensor(),
											 one_body_mass_moments.inertia_tensor(),
											 1e-12_kgm2);
	test_asserts::verify_equal_with_epsilon ("fixed constraints preserve the same torque-free precession phase as the equivalent single body",
											 measured_phase.multi_body_phase,
											 measured_phase.one_body_phase,
											 0.08_rad);
});


nu::AutoTest t_4 ("rigid_body::System: constraint friction does not damp free torque-free precession", []{
	EquivalentSymmetricTops tops;
	auto const initial_angular_velocity = SpaceVector<si::AngularVelocity, WorldSpace> { 3_radps, 0_radps, 12_radps };
	auto const duration = 100_ms;
	auto const dt = 50_us;
	auto const friction_factor = 1.0;
	auto const angular_momentum_direction =
		normalized_direction_or_zero (angular_momentum (tops.one_body_top.body->mass_moments<WorldSpace>(), initial_angular_velocity));
	auto const measured_phase = measure_equivalent_torque_free_precession_phase(
		tops,
		initial_angular_velocity,
		angular_momentum_direction,
		duration,
		dt,
		friction_factor
	);

	test_asserts::verify_equal_with_epsilon ("constraint friction leaves the constrained top on the same torque-free precession path as the equivalent composite body",
											 measured_phase.multi_body_phase,
											 measured_phase.one_body_phase,
											 0.08_rad);
});

} // namespace
} // namespace xf::test
