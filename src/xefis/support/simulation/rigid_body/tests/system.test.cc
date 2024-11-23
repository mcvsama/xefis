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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/transforms.h>
#include <xefis/support/math/triangulation.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/nature/mass_moments.h>
#include <xefis/support/nature/various_inertia_tensors.h>
#include <xefis/support/simulation/constraints/angular_limits_constraint.h>
#include <xefis/support/simulation/constraints/angular_servo_constraint.h>
#include <xefis/support/simulation/constraints/fixed_constraint.h>
#include <xefis/support/simulation/constraints/hinge_constraint.h>
#include <xefis/support/simulation/constraints/linear_limits_constraint.h>
#include <xefis/support/simulation/constraints/slider_constraint.h>
#include <xefis/support/simulation/rigid_body/concepts.h>
#include <xefis/support/simulation/rigid_body/impulse_solver.h>
#include <xefis/support/simulation/rigid_body/utility.h>
#include <xefis/support/simulation/rigid_body/various_shapes.h>
#include <xefis/support/simulation/evolver.h>
#include <xefis/support/ui/rigid_body_viewer.h>

// Neutrino:
#include <neutrino/qt/qutils.h>
#include <neutrino/test/dummy_qapplication.h>
#include <neutrino/test/manual_test.h>

// Standard:
#include <cstddef>
#include <memory>
#include <optional>
#include <string>


namespace xf::test {
namespace {

static constexpr SpaceVector<si::Force, WorldSpace>		kNoForce	{ 0_N, 0_N, 0_N };
static constexpr SpaceVector<si::Torque, WorldSpace>	kNoTorque	{ 0_Nm, 0_Nm, 0_Nm };

xf::Logger g_null_logger;

auto const no_rotation = kNoRotation<WorldSpace, BodyCOM>;
auto const gravity_acceleration = 9.81_mps2;

auto const kPlacement1 = Placement<WorldSpace, BodyCOM> ({ 0.0_m, 0_m, 0_m }, no_rotation);
auto const kPlacement2 = Placement<WorldSpace, BodyCOM> ({ 0.0_m, 3_m, 0_m }, no_rotation);
auto const kPlacement3 = Placement<WorldSpace, BodyCOM> ({ -0.01_m, 4_m, 0_m }, no_rotation);
auto const kPlacement3z = Placement<WorldSpace, BodyCOM> ({ -0.01_m, 4_m, 0.5_m }, no_rotation);
auto const kPlacement4 = Placement<WorldSpace, BodyCOM> ({ -0.1_m, 4.5_m, 0_m }, no_rotation);
auto const kPlacement5 = Placement<WorldSpace, BodyCOM> ({ 3_m, 0_m, 0_m }, no_rotation);
auto const kPlacement6 = Placement<WorldSpace, BodyCOM> ({ 3_m, 0_m, 1_m }, no_rotation);

auto const hinge1 = SpaceLength<BodyCOM> (0.0_m, 1.5_m, 0_m);
auto const hinge2 = SpaceLength<BodyCOM> (0.0_m, 0.5_m, 0_m);
auto const hinge3 = SpaceLength<BodyCOM> (0.0_m, 0.25_m, 0_m);


MassMoments<BodyCOM>
make_body_mass_moments (si::Mass const mass, si::Length const edge_length)
{
	return {
		mass,
		make_cuboid_inertia_tensor<BodyCOM> (mass, edge_length),
	};
}


MassMoments<BodyCOM>
make_body_mass_moments (si::Mass const mass, si::Density const density = 1000_kg / 1_m3)
{
	auto const edge_length = 1_m * std::pow ((mass / density).value(), 1.0 / 3.0);
	return make_body_mass_moments (mass, edge_length);
}


void
run (rigid_body::System& system, rigid_body::Body* followed_body, std::function<void (si::Time dt)> apply_forces = nullptr)
{
	auto solver = rigid_body::ImpulseSolver (system, 1);
	system.set_baumgarte_factor (0.5);

	neutrino::DummyQApplication app;

	Evolver evolver (1_ms, g_null_logger, [&] (si::Time const dt) {
		if (apply_forces)
			apply_forces (dt);

		solver.evolve (dt);
	});

	QWidget w (nullptr);
	auto const lh = neutrino::default_line_height (&w);

	RigidBodyViewer viewer (nullptr, 60_Hz);
	viewer.set_rigid_body_system (&system);
	viewer.set_redraw_callback ([&evolver] (std::optional<si::Time> const simulation_time) {
		if (simulation_time)
			evolver.evolve (*simulation_time);
		else
			evolver.evolve (1);
	});
	viewer.resize (QSize (50 * lh, 50 * lh));
	viewer.set_followed_body (followed_body);
	viewer.show();

	app->exec();
}


ManualTest t_1 ("rigid_body::System: airplane", []{
	static xf::AirfoilSpline const
	kSpline {
		{ 1.00,  +0.00 },
		{ 0.80,  +0.05 },
		{ 0.60,  +0.10 },
		{ 0.40,  +0.15 },
		{ 0.20,  +0.13 },
		{ 0.00,   0.00 },
		{ 0.20,  -0.13 },
		{ 0.40,  -0.15 },
		{ 0.60,  -0.10 },
		{ 0.80,  -0.05 },
		{ 1.00,  -0.00 },
	};

	rigid_body::System system;

	auto const z_minus_90_rotation = z_rotation<WorldSpace> (-90_deg);
	auto const wing_to_normal_rotation = z_minus_90_rotation * x_rotation<WorldSpace> (+90_deg);

	auto wing_shape = rigid_body::make_airfoil_shape ({ .spline = kSpline, .chord_length = 50_cm, .wing_length = 4_m, .with_bottom = true, .with_top = true });
	wing_shape.translate ({ -25_cm, 0_m, -2_m });
	auto& wing = system.add<rigid_body::Body> (make_body_mass_moments (0.1_kg));
	wing.set_shape (wing_shape);
	wing.rotate_about_body_origin (wing_to_normal_rotation);

	auto tail_h_shape = rigid_body::make_airfoil_shape ({ .spline = kSpline, .chord_length = 40_cm, .wing_length = 1_m, .with_bottom = true, .with_top = true });
	tail_h_shape.translate ({ 0_m, 0_m, -0.5_m });
	auto& tail_h = system.add<rigid_body::Body> (make_body_mass_moments (0.01_kg));
	tail_h.set_shape (tail_h_shape);
	tail_h.rotate_about_body_origin (wing_to_normal_rotation);
	tail_h.translate<WorldSpace> ({ 0_m, -1.5_m, 0_m });

	auto tail_v_shape = rigid_body::make_airfoil_shape ({ .spline = kSpline, .chord_length = 40_cm, .wing_length = 0.5_m, .with_bottom = true, .with_top = true });
	auto& tail_v = system.add<rigid_body::Body> (make_body_mass_moments (0.005_kg));
	tail_v.set_shape (tail_v_shape);
	tail_v.rotate_about_body_origin (z_minus_90_rotation);
	tail_v.translate<WorldSpace> ({ 0_m, -1.5_m, 0_m });

	system.add<rigid_body::FixedConstraint> (wing, tail_h);
	system.add<rigid_body::FixedConstraint> (tail_h, tail_v);

	for (auto* body: { &wing, &tail_h, &tail_v })
		body->rotate_about_world_origin (z_rotation<WorldSpace> (90_deg));

	run (system, &wing);
});


ManualTest t_2 ("rigid_body::System: fixed constraints", []{
	rigid_body::System system;

	auto& body1 = system.add<rigid_body::Body> (make_body_mass_moments (10_kg));
	body1.set_placement (kPlacement1);
	body1.rotate_about_world_origin (x_rotation<WorldSpace> (+90_deg));

	auto& body2 = system.add<rigid_body::Body> (make_body_mass_moments (1_kg));
	body2.set_placement (kPlacement5);
	body2.rotate_about_world_origin (y_rotation<WorldSpace> (+90_deg));

	system.add<rigid_body::FixedConstraint> (body1, body2);

	for (auto* body: { &body1, &body2 })
		body->rotate_about_world_origin (y_rotation<WorldSpace> (+90_deg));

	run (system, &body1, [&] (si::Time const) {
		body1.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, +gravity_acceleration * (10_kg + 1_kg), 0_N }, kNoTorque));
		body2.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, -gravity_acceleration * 10_kg, 0_N }, kNoTorque));
	});
});


ManualTest t_2_1 ("rigid_body::System: more fixed constraints", []{
	rigid_body::System system;

	auto& body1 = system.add<rigid_body::Body> (make_body_mass_moments (10_kg));
	body1.set_placement (kPlacement1);

	auto& body2 = system.add<rigid_body::Body> (make_body_mass_moments (1_kg));
	body2.set_placement (kPlacement2);

	auto& body3 = system.add<rigid_body::Body> (make_body_mass_moments (0.5_kg));
	body3.set_placement (kPlacement3);

	auto& body4 = system.add<rigid_body::Body> (make_body_mass_moments (0.1_kg));
	body4.set_placement (kPlacement4);

	auto& body5 = system.add<rigid_body::Body> (make_body_mass_moments (0.2_kg));
	body5.set_placement (kPlacement5);

	auto& body6 = system.add<rigid_body::Body> (make_body_mass_moments (0.2_kg));
	body6.set_placement (kPlacement6);

	system.add<rigid_body::FixedConstraint> (body1, body2);
	system.add<rigid_body::FixedConstraint> (body2, body3);
	system.add<rigid_body::FixedConstraint> (body1, body2);
	system.add<rigid_body::FixedConstraint> (body1, body5);
	system.add<rigid_body::FixedConstraint> (body5, body6);

	run (system, &body1, [&] (si::Time const) {
		body1.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, +gravity_acceleration * (1_kg + 0.5_kg + 0.1_kg + 0.2_kg + 0.2_kg), 0_N }, kNoTorque));
		body2.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, -gravity_acceleration * 1_kg, 0_N }, kNoTorque));
		body3.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, -gravity_acceleration * 0.5_kg, 0_N }, kNoTorque));
		body4.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, -gravity_acceleration * 0.1_kg, 0_N }, kNoTorque));
		body5.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, -gravity_acceleration * 0.2_kg, 0_N }, kNoTorque));
		body6.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, -gravity_acceleration * 0.2_kg, 0_N }, kNoTorque));
	});
});


ManualTest t_3 ("rigid_body::System: hinge constraints", []{
	rigid_body::System system;

	auto& body1 = system.add<rigid_body::Body> (make_body_mass_moments (10_kg));
	body1.set_placement (kPlacement1);

	auto& body2 = system.add<rigid_body::Body> (make_body_mass_moments (1_kg));
	body2.set_placement (kPlacement2);

	auto& body3 = system.add<rigid_body::Body> (make_body_mass_moments (0.5_kg));
	body3.set_placement (kPlacement3z);

	auto& body4 = system.add<rigid_body::Body> (make_body_mass_moments (0.1_kg));
	body4.set_placement (kPlacement4);

	auto& h1 = system.add<rigid_body::HingePrecalculation> (hinge1, hinge1 + SpaceLength<BodyCOM> { 0_m, 0_m, +1_m }, body1, body2);
	system.add<rigid_body::HingeConstraint> (h1);

	auto& h2 = system.add<rigid_body::HingePrecalculation> (hinge2, hinge2 + SpaceLength<BodyCOM> { 0_m, 0_m, +1_m }, body2, body3);
	system.add<rigid_body::HingeConstraint> (h2);

	auto& h3 = system.add<rigid_body::HingePrecalculation> (hinge3, hinge3 + SpaceLength<BodyCOM> { 0_m, 0_m, +1_m }, body3, body4);
	system.add<rigid_body::HingeConstraint> (h3);

	run (system, &body1, [&] (si::Time const) {
		body1.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, +gravity_acceleration * (1_kg + 0.5_kg + 0.1_kg), 0_N }, kNoTorque));
		body2.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, -gravity_acceleration * 1_kg, 0_N }, kNoTorque));
		body3.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, -gravity_acceleration * 0.5_kg, 0_N }, kNoTorque));
		body4.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, -gravity_acceleration * 0.1_kg, 0_N }, kNoTorque));
	});
});


ManualTest t_4 ("rigid_body::System: multiple constraints", []{
	rigid_body::System system;

	auto& body1 = system.add<rigid_body::Body> (make_body_mass_moments (10_kg));
	body1.set_placement (kPlacement1);

	auto& body2 = system.add<rigid_body::Body> (make_body_mass_moments (1_kg));
	body2.set_placement (kPlacement2);

	auto& body3 = system.add<rigid_body::Body> (make_body_mass_moments (0.5_kg));
	body3.set_placement (kPlacement3);

	auto& body4 = system.add<rigid_body::Body> (make_body_mass_moments (0.1_kg));
	body4.set_placement (kPlacement4);

	auto& body5 = system.add<rigid_body::Body> (make_body_mass_moments (1_kg));
	body5.set_placement (kPlacement5);

	auto& body6 = system.add<rigid_body::Body> (make_body_mass_moments (0.1_kg));
	body6.set_placement (kPlacement6);

	auto& h1 = system.add<rigid_body::HingePrecalculation> (hinge1, hinge1 + SpaceLength<BodyCOM> { 0_m, 0_m, +1_m }, body1, body2);
	system.add<rigid_body::HingeConstraint> (h1);
	system.add<rigid_body::AngularLimitsConstraint> (h1, -90_deg, +90_deg);

	auto& h2 = system.add<rigid_body::HingePrecalculation> (hinge2, hinge2 + SpaceLength<BodyCOM> { 0_m, 0_m, +1_m }, body2, body3);
	system.add<rigid_body::HingeConstraint> (h2);
	system.add<rigid_body::AngularLimitsConstraint> (h2, -90_deg, +90_deg);
	auto& servo = system.add (make_standard_9gram_servo_constraint (h2, 10.0f));
	servo.set_efficiency (0.8);
	servo.set_voltage (6_V);
	servo.set_setpoint (45_deg);

	auto& s1 = system.add<rigid_body::SliderPrecalculation> (body3, body4, SpaceVector<double, WorldSpace> { 1.0, 0.0, 0.0 });
	system.add<rigid_body::SliderConstraint> (s1);
	system.add<rigid_body::LinearLimitsConstraint> (s1, -0.5_m, +0.5_m);

	system.add<rigid_body::FixedConstraint> (body1, body5);
	system.add<rigid_body::FixedConstraint> (body5, body6);

	run (system, &body1, [&] (si::Time const) {
		body1.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, +gravity_acceleration * (1_kg + 0.5_kg + 0.1_kg), 0_N }, kNoTorque));
		body2.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, -gravity_acceleration * 1_kg, 0_N }, kNoTorque));
		body3.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, -gravity_acceleration * 0.5_kg, 0_N }, kNoTorque));
		body4.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, -gravity_acceleration * 0.1_kg, 0_N }, kNoTorque));
	});
});


ManualTest t_5 ("rigid_body::System: intermediate axis of rotation", []{
	rigid_body::System system;

	auto const add_variant = [&system] (SpaceLength<WorldSpace> const& position_offset) -> rigid_body::Body&
	{
		auto const j = 1.5_m;

		auto& body_00 = system.add<rigid_body::Body> (make_body_mass_moments (20_kg));
		body_00.set_placement (Placement<WorldSpace, BodyCOM> (position_offset + SpaceLength<WorldSpace> { 0_m, 0_m, 0_m }, no_rotation));

		auto& body_0m = system.add<rigid_body::Body> (make_body_mass_moments (20_kg));
		body_0m.set_placement (Placement<WorldSpace, BodyCOM> (position_offset + SpaceLength<WorldSpace> { 0_m, -j, 0_m }, no_rotation));

		auto& body_0p = system.add<rigid_body::Body> (make_body_mass_moments (20_kg));
		body_0p.set_placement (Placement<WorldSpace, BodyCOM> (position_offset + SpaceLength<WorldSpace> { 0_m, +j, 0_m }, no_rotation));

		auto& body_p0 = system.add<rigid_body::Body> (make_body_mass_moments (20_kg));
		body_p0.set_placement (Placement<WorldSpace, BodyCOM> (position_offset + SpaceLength<WorldSpace> { +j, 0_m, 0_m }, no_rotation));

		system.add<rigid_body::FixedConstraint> (body_00, body_0m);
		system.add<rigid_body::FixedConstraint> (body_00, body_0p);
		system.add<rigid_body::FixedConstraint> (body_00, body_p0);

		return body_00;
	};

	auto const k = 2.5_m;
	auto& body_ox = add_variant ({ +k, -k, 0_m }); // Unstable
	auto& body_oy = add_variant ({ -k, +k, 0_m }); // Stable
	auto& body_oz = add_variant ({ -k, -k, 0_m }); // Stable

	si::Time total_t = 0_s;

	run (system, nullptr, [&] (si::Time const dt) {
		// Apply torque for a while:
		if (total_t < 0.7_s)
		{
			body_ox.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, 0_N, 0_N }, { 1000_Nm, 0_Nm, 0_Nm }));
			body_oy.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, 0_N, 0_N }, { 0_Nm, 1000_Nm, 0_Nm }));
			body_oz.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, 0_N, 0_N }, { 0_Nm, 0_Nm, 1000_Nm }));
		}

		total_t += dt;
	});
});

} // namespace
} // namespace xf::test

