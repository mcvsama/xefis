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
#include <xefis/config/all.h>
#include <xefis/support/math/transforms.h>
#include <xefis/support/math/triangulation.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/nature/mass_moments.h>
#include <xefis/support/nature/various_inertia_tensors.h>
#include <xefis/support/shapes/various_shapes.h>
#include <xefis/support/simulation/constraints/angular_limits_constraint.h>
#include <xefis/support/simulation/constraints/angular_servo_constraint.h>
#include <xefis/support/simulation/constraints/fixed_constraint.h>
#include <xefis/support/simulation/constraints/hinge_constraint.h>
#include <xefis/support/simulation/constraints/linear_limits_constraint.h>
#include <xefis/support/simulation/constraints/slider_constraint.h>
#include <xefis/support/simulation/rigid_body/concepts.h>
#include <xefis/support/simulation/rigid_body/impulse_solver.h>
#include <xefis/support/simulation/rigid_body/utility.h>
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
#include <vector>


namespace xf::test {
namespace {

static constexpr SpaceForce<WorldSpace>		kNoForce	{ 0_N, 0_N, 0_N };
static constexpr SpaceTorque<WorldSpace>	kNoTorque	{ 0_Nm, 0_Nm, 0_Nm };

nu::Logger g_null_logger;

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
	auto const edge_length = 1_m * std::pow ((mass / density).to_floating_point(), 1.0 / 3.0);
	return make_body_mass_moments (mass, edge_length);
}


MassMoments<BodyCOM>
make_cuboid_body_mass_moments (si::Mass const mass, SpaceLength<BodyOrigin> const& dimensions)
{
	return {
		mass,
		make_cuboid_inertia_tensor<BodyCOM> (mass, SpaceLength<> { dimensions[0], dimensions[1], dimensions[2] }),
	};
}


Placement<WorldSpace, BodyCOM>
make_pendulum_link_placement (SpaceLength<WorldSpace> const& top_anchor,
							  si::Length const half_length,
							  RotationQuaternion<WorldSpace> const& rotation)
{
	auto const body_rotation = math::coordinate_system_cast<WorldSpace, BodyCOM, WorldSpace, WorldSpace> (rotation);
	auto const orientation = Placement<WorldSpace, BodyCOM> ({ 0_m, 0_m, 0_m }, body_rotation);
	auto const com_position = top_anchor - orientation.rotate_to_base (SpaceLength<BodyCOM> { 0_m, +half_length, 0_m });

	return Placement<WorldSpace, BodyCOM> (com_position, body_rotation);
}


rigid_body::Body&
add_cuboid_body (rigid_body::System& system,
				 MassMoments<BodyCOM> const& mass_moments,
				 Placement<WorldSpace, BodyCOM> const& placement,
				 SpaceLength<BodyOrigin> const& dimensions,
				 ShapeMaterial const& material)
{
	auto& body = system.add<rigid_body::Body> (mass_moments);
	body.set_placement (placement);
	body.set_shape (make_centered_cube_shape (dimensions, material));
	return body;
}


void
run (rigid_body::System& system, rigid_body::Body* followed_body, std::function<void (si::Time dt)> apply_forces = nullptr)
{
	auto solver = rigid_body::ImpulseSolver (system, 100);
	system.set_default_baumgarte_factor (0.01);
	system.set_default_friction_factor (0.0001);

	nu::DummyQApplication app;

	Evolver evolver (0_s, 1_ms, g_null_logger, [&] (si::Time const dt) {
		if (apply_forces)
			apply_forces (dt);

		solver.evolve (dt);
	});

	QWidget w (nullptr);
	auto const lh = nu::default_line_height (&w);

	RigidBodyViewer viewer (nullptr, RigidBodyViewer::AutoFPS);
	viewer.set_rigid_body_system (&system);
	viewer.set_before_paint_callback ([&evolver] (std::optional<si::Time> const frame_duration) {
		if (frame_duration)
			evolver.evolve (*frame_duration);
		else
			evolver.evolve (1);
	});
	viewer.resize (QSize (50 * lh, 50 * lh));
	viewer.set_universe_enabled (true);

	if (followed_body)
		viewer.set_followed (*followed_body);

	viewer.set_camera_mode (RigidBodyPainter::FixedView);
	viewer.show();

	app->exec();
}


nu::ManualTest t_1 ("rigid_body::System: airplane", []{
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

	auto wing_shape = make_airfoil_shape ({ .spline = kSpline, .chord_length = 50_cm, .wing_length = 4_m, .with_bottom = true, .with_top = true });
	wing_shape.translate ({ -25_cm, 0_m, -2_m });
	auto& wing = system.add<rigid_body::Body> (make_body_mass_moments (0.1_kg));
	wing.set_shape (wing_shape);
	wing.rotate_about_body_origin (wing_to_normal_rotation);

	auto tail_h_shape = make_airfoil_shape ({ .spline = kSpline, .chord_length = 40_cm, .wing_length = 1_m, .with_bottom = true, .with_top = true });
	tail_h_shape.translate ({ 0_m, 0_m, -0.5_m });
	auto& tail_h = system.add<rigid_body::Body> (make_body_mass_moments (0.01_kg));
	tail_h.set_shape (tail_h_shape);
	tail_h.rotate_about_body_origin (wing_to_normal_rotation);
	tail_h.translate<WorldSpace> ({ 0_m, -1.5_m, 0_m });

	auto tail_v_shape = make_airfoil_shape ({ .spline = kSpline, .chord_length = 40_cm, .wing_length = 0.5_m, .with_bottom = true, .with_top = true });
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


nu::ManualTest t_2 ("rigid_body::System: fixed constraints", []{
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
		body1.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, +gravity_acceleration * 10_kg, 0_N }, kNoTorque));
		body2.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, -gravity_acceleration * 10_kg, 0_N }, kNoTorque));
	});
});


nu::ManualTest t_2_1 ("rigid_body::System: more fixed constraints", []{
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


nu::ManualTest t_3 ("rigid_body::System: hinge constraints", []{
	rigid_body::System system;

	auto& body1 = system.add<rigid_body::Body> (make_body_mass_moments (10_kg));
	body1.set_placement (kPlacement1);

	auto& body2 = system.add<rigid_body::Body> (make_body_mass_moments (1_kg));
	body2.set_placement (kPlacement2);

	auto& body3 = system.add<rigid_body::Body> (make_body_mass_moments (0.5_kg));
	body3.set_placement (kPlacement3z);

	auto& body4 = system.add<rigid_body::Body> (make_body_mass_moments (0.1_kg));
	body4.set_placement (kPlacement4);

	auto& h1 = system.add<rigid_body::HingePrecomputation> (hinge1, hinge1 + SpaceLength<BodyCOM> { 0_m, 0_m, +1_m }, body1, body2);
	system.add<rigid_body::HingeConstraint> (h1);

	auto& h2 = system.add<rigid_body::HingePrecomputation> (hinge2, hinge2 + SpaceLength<BodyCOM> { 0_m, 0_m, +1_m }, body2, body3);
	system.add<rigid_body::HingeConstraint> (h2);

	auto& h3 = system.add<rigid_body::HingePrecomputation> (hinge3, hinge3 + SpaceLength<BodyCOM> { 0_m, 0_m, +1_m }, body3, body4);
	system.add<rigid_body::HingeConstraint> (h3);

	run (system, &body1, [&] (si::Time const) {
		body1.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, +gravity_acceleration * (1_kg + 0.5_kg + 0.1_kg), 0_N }, kNoTorque));
		body2.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, -gravity_acceleration * 1_kg, 0_N }, kNoTorque));
		body3.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, -gravity_acceleration * 0.5_kg, 0_N }, kNoTorque));
		body4.apply_impulse (ForceMoments<WorldSpace> ({ 0_N, -gravity_acceleration * 0.1_kg, 0_N }, kNoTorque));
	});
});


nu::ManualTest t_4 ("rigid_body::System: multiple constraints", []{
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

	auto& h1 = system.add<rigid_body::HingePrecomputation> (hinge1, hinge1 + SpaceLength<BodyCOM> { 0_m, 0_m, +1_m }, body1, body2);
	system.add<rigid_body::HingeConstraint> (h1);
	system.add<rigid_body::AngularLimitsConstraint> (h1, -90_deg, +90_deg);

	auto& h2 = system.add<rigid_body::HingePrecomputation> (hinge2, hinge2 + SpaceLength<BodyCOM> { 0_m, 0_m, +1_m }, body2, body3);
	system.add<rigid_body::HingeConstraint> (h2);
	system.add<rigid_body::AngularLimitsConstraint> (h2, -90_deg, +90_deg);
	auto& servo = system.add (make_standard_9gram_servo_constraint (h2, 10.0f));
	servo.set_efficiency (0.8);
	servo.set_voltage (6_V);
	servo.set_setpoint (45_deg);

	auto& s1 = system.add<rigid_body::SliderPrecomputation> (body3, body4, SpaceVector<double, WorldSpace> { 1.0, 0.0, 0.0 });
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


nu::ManualTest t_5 ("rigid_body::System: intermediate axis of rotation", []{
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


nu::ManualTest t_6 ("rigid_body::System: 3-layer 3D pendulums", []{
	rigid_body::System system;

	struct WeightedBody
	{
		rigid_body::Body*	body;
		si::Mass			mass;
	};

	struct CounterweightedBody
	{
		rigid_body::Body*	body;
		si::Mass			supported_mass;
	};

	auto constexpr kSupportMass = 1000_kg;
	auto constexpr kLink1Mass = 8_kg;
	auto constexpr kLink2Mass = 5_kg;
	auto constexpr kLink3Mass = 3_kg;

	auto const support_dimensions = SpaceLength<BodyOrigin> { 70_cm, 25_cm, 70_cm };
	auto const link_1_dimensions = SpaceLength<BodyOrigin> { 18_cm, 120_cm, 18_cm };
	auto const link_2_dimensions = SpaceLength<BodyOrigin> { 16_cm, 100_cm, 16_cm };
	auto const link_3_dimensions = SpaceLength<BodyOrigin> { 14_cm, 80_cm, 14_cm };

	auto constexpr kSupportHalfHeight = 12.5_cm;
	auto constexpr kLink1HalfLength = 60_cm;
	auto constexpr kLink2HalfLength = 50_cm;

	std::vector<WeightedBody> weighted_bodies;
	weighted_bodies.reserve (9);

	std::vector<CounterweightedBody> counterweighted_bodies;
	counterweighted_bodies.reserve (3);

	auto const add_pendulum =
		[&] (SpaceLength<WorldSpace> const& support_position,
			 RotationQuaternion<WorldSpace> const& link_1_rotation,
			 RotationQuaternion<WorldSpace> const& link_2_rotation,
			 RotationQuaternion<WorldSpace> const& link_3_rotation,
			 SpaceVector<double, WorldSpace> const& hinge_axis_1,
			 SpaceVector<double, WorldSpace> const& hinge_axis_2,
			 SpaceVector<double, WorldSpace> const& hinge_axis_3)
	{
		auto& support = add_cuboid_body (
			system,
			make_cuboid_body_mass_moments (kSupportMass, support_dimensions),
			Placement<WorldSpace, BodyCOM> (support_position, no_rotation),
			support_dimensions,
			kGreyMatte);

		auto const anchor_1 = support_position + SpaceLength<WorldSpace> { 0_m, -kSupportHalfHeight, 0_m };

		auto const link_1_placement = make_pendulum_link_placement (anchor_1, kLink1HalfLength, link_1_rotation);
		auto& link_1 = add_cuboid_body (
			system,
			make_cuboid_body_mass_moments (kLink1Mass, link_1_dimensions),
			link_1_placement,
			link_1_dimensions,
			kWhiteMatte);

		auto const anchor_2 = link_1_placement.rotate_translate_to_base (SpaceLength<BodyCOM> { 0_m, -kLink1HalfLength, 0_m });
		auto const link_2_placement = make_pendulum_link_placement (anchor_2, kLink2HalfLength, link_2_rotation);
		auto& link_2 = add_cuboid_body (
			system,
			make_cuboid_body_mass_moments (kLink2Mass, link_2_dimensions),
			link_2_placement,
			link_2_dimensions,
			kGreyMatte);

		auto const anchor_3 = link_2_placement.rotate_translate_to_base (SpaceLength<BodyCOM> { 0_m, -kLink2HalfLength, 0_m });
		auto const link_3_placement = make_pendulum_link_placement (anchor_3, 40_cm, link_3_rotation);
		auto& link_3 = add_cuboid_body (
			system,
			make_cuboid_body_mass_moments (kLink3Mass, link_3_dimensions),
			link_3_placement,
			link_3_dimensions,
			kWhiteMatte);

		auto& h1 = system.add<rigid_body::HingePrecomputation> (
			support,
			anchor_1,
			anchor_1 + hinge_axis_1.normalized() * 1_m,
			link_1);
		system.add<rigid_body::HingeConstraint> (h1);

		auto& h2 = system.add<rigid_body::HingePrecomputation> (
			link_1,
			anchor_2,
			anchor_2 + hinge_axis_2.normalized() * 1_m,
			link_2);
		system.add<rigid_body::HingeConstraint> (h2);

		auto& h3 = system.add<rigid_body::HingePrecomputation> (
			link_2,
			anchor_3,
			anchor_3 + hinge_axis_3.normalized() * 1_m,
			link_3);
		system.add<rigid_body::HingeConstraint> (h3);

		weighted_bodies.push_back ({ &link_1, kLink1Mass });
		weighted_bodies.push_back ({ &link_2, kLink2Mass });
		weighted_bodies.push_back ({ &link_3, kLink3Mass });
		counterweighted_bodies.push_back ({ &support, kLink1Mass + kLink2Mass + kLink3Mass });
	};

	add_pendulum (
		{ -4_m, 4_m, 0_m },
		z_rotation<WorldSpace> (+18_deg) * x_rotation<WorldSpace> (-10_deg),
		z_rotation<WorldSpace> (+9_deg) * x_rotation<WorldSpace> (+7_deg),
		z_rotation<WorldSpace> (-12_deg) * x_rotation<WorldSpace> (+6_deg),
		SpaceVector<double, WorldSpace> { +0.05, +0.01, 1.0 },
		SpaceVector<double, WorldSpace> { -0.07, +0.03, 1.0 },
		SpaceVector<double, WorldSpace> { +0.04, -0.05, 1.0 });

	add_pendulum (
		{ 0_m, 4_m, 0_m },
		z_rotation<WorldSpace> (-16_deg) * x_rotation<WorldSpace> (+11_deg),
		z_rotation<WorldSpace> (+13_deg) * x_rotation<WorldSpace> (-8_deg),
		z_rotation<WorldSpace> (+7_deg) * x_rotation<WorldSpace> (+9_deg),
		SpaceVector<double, WorldSpace> { -0.06, -0.01, 1.0 },
		SpaceVector<double, WorldSpace> { +0.03, +0.06, 1.0 },
		SpaceVector<double, WorldSpace> { -0.05, +0.04, 1.0 });

	add_pendulum (
		{ +4_m, 4_m, 0_m },
		z_rotation<WorldSpace> (+12_deg) * x_rotation<WorldSpace> (+13_deg),
		z_rotation<WorldSpace> (-15_deg) * x_rotation<WorldSpace> (-6_deg),
		z_rotation<WorldSpace> (+10_deg) * x_rotation<WorldSpace> (-9_deg),
		SpaceVector<double, WorldSpace> { +0.02, +0.08, 1.0 },
		SpaceVector<double, WorldSpace> { -0.08, -0.02, 1.0 },
		SpaceVector<double, WorldSpace> { +0.06, +0.05, 1.0 });

	run (system, nullptr, [&] (si::Time const) {
		for (auto const& [ body, mass ]: weighted_bodies)
			body->apply_impulse (ForceMoments<WorldSpace> ({ 0_N, -gravity_acceleration * mass, 0_N }, kNoTorque));

		for (auto const& [ body, supported_mass ]: counterweighted_bodies)
			body->apply_impulse (ForceMoments<WorldSpace> ({ 0_N, +gravity_acceleration * supported_mass, 0_N }, kNoTorque));
	});
});

} // namespace
} // namespace xf::test
