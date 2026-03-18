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
#include <xefis/support/math/rotations.h>
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
#include <numbers>


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


struct SymmetricTop
{
	rb::Group*	group;
	rb::Body*	hub;
	rb::Body*	upper_axial_mass;
	rb::Body*	lower_axial_mass;
};


MassMoments<BodyCOM>
make_lumped_mass_moments (si::Mass const mass)
{
	return {
		mass,
		make_cuboid_inertia_tensor<BodyCOM> (mass, kBodyEdgeLength),
	};
}


SymmetricTop
make_symmetric_top (rb::System& system)
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
		.hub = &hub,
		.upper_axial_mass = &upper_axial_mass,
		.lower_axial_mass = &lower_axial_mass,
	};
}


void
evolve (rb::System& system, si::Time duration, si::Time max_dt, std::function<void (si::Time)> const& before_step = {})
{
	rb::ImpulseSolver solver (system, 1);
	system.set_default_baumgarte_factor (0.5);
	system.set_default_constraint_force_mixing_factor (0.0);
	system.set_default_friction_factor (0.0);

	for (auto remaining = duration; remaining > 0_s; )
	{
		auto const dt = std::min (remaining, max_dt);
		if (before_step)
			before_step (dt);

		solver.evolve (dt);
		remaining -= dt;
	}
}


void
set_rigid_group_rotation (SymmetricTop const& top, SpaceVector<si::AngularVelocity, WorldSpace> const& angular_velocity)
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


SpaceVector<double, WorldSpace>
top_axis (SymmetricTop const& top)
{
	return normalized_direction_or_zero (top.upper_axial_mass->placement().position() - top.lower_axial_mass->placement().position());
}


si::Angle
tilt_about_x (SymmetricTop const& top)
{
	auto const axis = top_axis (top);
	return std::atan2 (axis[1], axis[2]) * 1_rad;
}


SpaceVector<si::AngularMomentum, WorldSpace>
angular_momentum (MassMoments<WorldSpace> const& mass_moments, SpaceVector<si::AngularVelocity, WorldSpace> const& angular_velocity)
{
	return mass_moments.inertia_tensor() * angular_velocity / 1_rad;
}


si::Angle
signed_rotation_about (SpaceVector<double, WorldSpace> const& initial_vector,
					   SpaceVector<double, WorldSpace> const& final_vector,
					   SpaceVector<double, WorldSpace> const& axis)
{
	auto const initial_projection = normalized_direction_or_zero (initial_vector - dot_product (initial_vector, axis) * axis);
	auto const final_projection = normalized_direction_or_zero (final_vector - dot_product (final_vector, axis) * axis);
	auto const rotation = angle_between (initial_projection, final_projection);
	auto const rotation_sign = dot_product (cross_product (initial_projection, final_projection), axis) >= 0.0 ? +1.0 : -1.0;

	return rotation_sign * rotation;
}


nu::AutoTest t_torque_induced_precession ("rigid_body::System: torque-induced precession", []{
	// <https://en.wikipedia.org/wiki/Precession#Torque-induced>

	rb::System system;
	auto const top = make_symmetric_top (system);
	auto const centered_mass_moments = top.group->mass_moments().centered_at_center_of_mass();
	auto const I3 = centered_mass_moments.inertia_tensor()[2, 2];
	auto const spin = 40_radps;
	auto const force = 6_N;
	auto const duration = 200_ms;
	auto const dt = 100_us;
	si::Torque const applied_torque = 2.0 * kAxialArm * force;
	auto const angular_momentum = I3 * spin / 1_rad;
	auto const expected_precession_rate = applied_torque * 1_rad / angular_momentum;
	si::Angle const expected_tilt = expected_precession_rate * duration;

	set_rigid_group_rotation (top, { 0_radps, 0_radps, spin });

	evolve (system, duration, dt, [&] (si::Time) {
		top.upper_axial_mass->apply_impulse (ForceMoments<WorldSpace> ({ +force, 0_N, 0_N }, { 0_Nm, 0_Nm, 0_Nm }));
		top.lower_axial_mass->apply_impulse (ForceMoments<WorldSpace> ({ -force, 0_N, 0_N }, { 0_Nm, 0_Nm, 0_Nm }));
	});

	auto const measured_tilt = tilt_about_x (top);

	test_asserts::verify_equal_with_epsilon ("Torque-induced precession rate matches tau / L for short times",
											 measured_tilt,
											 expected_tilt,
											 0.04_rad);
});


nu::AutoTest t_torque_free_precession ("rigid_body::System: torque-free precession", []{
	// <https://en.wikipedia.org/wiki/Precession#Torque-free>

	rb::System system;
	auto const top = make_symmetric_top (system);
	auto const centered_mass_moments = top.group->mass_moments().centered_at_center_of_mass();
	auto const I1 = centered_mass_moments.inertia_tensor()[0, 0];
	auto const I3 = centered_mass_moments.inertia_tensor()[2, 2];
	auto const initial_angular_velocity = SpaceVector<si::AngularVelocity, WorldSpace> { 3_radps, 0_radps, 12_radps };
	auto const duration = 200_ms;
	auto const dt = 100_us;
	auto const initial_axis = top_axis (top);
	auto const initial_angular_momentum = angular_momentum (centered_mass_moments, initial_angular_velocity);
	auto const angular_momentum_direction = normalized_direction_or_zero (initial_angular_momentum);
	auto const expected_precession_rate = abs (initial_angular_momentum) * 1_rad / I1;
	si::Angle const expected_phase = expected_precession_rate * duration;

	set_rigid_group_rotation (top, initial_angular_velocity);

	evolve (system, duration, dt);

	auto const measured_phase = signed_rotation_about (initial_axis, top_axis (top), angular_momentum_direction);

	test_asserts::verify ("Symmetric top really has distinct axial inertia", abs (I3 - I1) > 0.01_kgm2);
	test_asserts::verify_equal_with_epsilon ("Torque-free precession rate matches the space-frame symmetric-top formula",
											 measured_phase,
											 expected_phase,
											 0.08_rad);
});

} // namespace
} // namespace xf::test
