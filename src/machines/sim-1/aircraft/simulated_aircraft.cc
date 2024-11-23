/* vim:ts=4
 *
 * Copyleft 2008…2023  Michał Gawron
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
#include "machine.h"
#include "data_center.h"
#include "models.h"
#include "simulation.h"

// Machines:
#include <machines/sim-1/common/airfoils/control_surface_airfoil.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/math/lonlat_radius.h>
#include <xefis/support/math/placement.h>
#include <xefis/support/nature/various_inertia_tensors.h>
#include <xefis/support/simulation/constraints/angular_limits_constraint.h>
#include <xefis/support/simulation/constraints/angular_servo_constraint.h>
#include <xefis/support/simulation/constraints/angular_spring_constraint.h>
#include <xefis/support/simulation/constraints/fixed_constraint.h>
#include <xefis/support/simulation/constraints/hinge_constraint.h>
#include <xefis/support/simulation/devices/angular_servo.h>
#include <xefis/support/simulation/devices/prandtl_tube.h>
#include <xefis/support/simulation/devices/wing.h>
#include <xefis/support/simulation/rigid_body/concepts.h>
#include <xefis/support/simulation/rigid_body/group.h>
#include <xefis/support/simulation/rigid_body/various_shapes.h>

// Neutrino:
#include <neutrino/math/math.h>

// Standard:
#include <cstddef>


namespace sim1::aircraft {

SimulatedAircraft
make_aircraft (xf::rigid_body::System& rigid_body_system, Models& models)
{
	namespace rb = xf::rigid_body;
	using xf::BodyCOM;
	using xf::WorldSpace;

	// Aircraft coordinate system:
	//   X - front
	//   Y - right wing
	//   Z - down

	auto const kDihedral = 0_deg;
	auto const kWingletAngle = 2_deg;
	auto const kFoamDensity = 3 * 0.12_kg / (3'600'000 * 1_mm * 1_mm * 1_mm);
	auto const kFuselageWidth = 20_cm;
	auto const kWingLength = 2_m;
	auto const kWingletLength = 50_cm;
	auto const kAileronLength = 1.2_m;
	auto const kTailHorizontalStabilizerChord = 20_cm;
	auto const kTailHorizontalStabilizerLength = 1_m;
	auto const kTailVerticalStabilizerChord = 20_cm;
	auto const kTailVerticalStabilizerLength = 50_cm;
	auto const kElevatorChord = 20_cm;
	auto const kElevatorLength = 1_m;
	auto const kRudderChord = 20_cm;
	auto const kRudderLength = 50_cm;

	auto const main_wing_airfoil_spline = xf::AirfoilSpline (sim1::control_surface_airfoil::kSpline);
	auto const main_wing_airfoil_characteristics =
		xf::AirfoilCharacteristics (main_wing_airfoil_spline,
									sim1::control_surface_airfoil::kLiftField,
									sim1::control_surface_airfoil::kDragField,
									sim1::control_surface_airfoil::kPitchingMomentField,
									sim1::control_surface_airfoil::kCenterOfPressureOffsetField);

	auto const main_wing_airfoil = xf::Airfoil (main_wing_airfoil_characteristics, 50_cm, kWingLength);
	auto const winglet_airfoil = xf::Airfoil (main_wing_airfoil_characteristics, 50_cm, kWingletLength);
	auto const aileron_airfoil = xf::Airfoil (main_wing_airfoil_characteristics, 20_cm, kAileronLength);
	auto const tail_h_airfoil = xf::Airfoil (main_wing_airfoil_characteristics, kTailHorizontalStabilizerChord, kTailHorizontalStabilizerLength);
	auto const elevator_airfoil = xf::Airfoil (main_wing_airfoil_characteristics, kElevatorChord, kElevatorLength);
	auto const tail_v_airfoil = xf::Airfoil (main_wing_airfoil_characteristics, kTailVerticalStabilizerChord, kTailVerticalStabilizerLength);
	auto const rudder_airfoil = xf::Airfoil (main_wing_airfoil_characteristics, kRudderChord, kRudderLength);

	auto const x_versor = xf::SpaceVector<double, BodyCOM> (1, 0, 0);
	auto const z_versor = xf::SpaceVector<double, BodyCOM> (0, 0, 1);
	auto const z_wing_length = z_versor * main_wing_airfoil.wing_length();
	auto const x_wing_chord = x_versor * main_wing_airfoil.chord_length();
	auto const z_aileron_length = z_versor * aileron_airfoil.wing_length();
	auto const z_elevator_length = z_versor * elevator_airfoil.wing_length();
	auto const z_rudder_length = z_versor * rudder_airfoil.wing_length();
	auto const wing_to_normal_rotation = xf::x_rotation<WorldSpace> (+90_deg) * xf::z_rotation<WorldSpace> (+180_deg);

	auto aircraft_group = rigid_body_system.make_group();

	// Fuselage

	auto const fuselage_mass = 2_kg;
	auto& fuselage = aircraft_group.add<rb::Body> (xf::MassMoments<BodyCOM> (fuselage_mass, xf::make_cuboid_inertia_tensor<BodyCOM> (fuselage_mass, { 100_cm, 100_cm, 100_cm })));
	fuselage.set_label ("fuselage");
	// Move it a bit to the front:
	fuselage.translate ({ +1.0_m, 0_m, 0_m });

	// Wing L

	auto& wing_l = aircraft_group.add<xf::sim::Wing> (main_wing_airfoil, kFoamDensity);
	wing_l.set_label ("wing L");
	wing_l.rotate_about_body_origin (wing_to_normal_rotation);
	// Move to the left:
	wing_l.translate ({ 0_m, -0.5 * kFuselageWidth, 0_m });
	wing_l.rotate_about_body_origin (xf::x_rotation<WorldSpace> (+kDihedral)); // TODO make possible to rotate about world origin by giving rotation in BodyOrigin space

	// Wing R

	auto& wing_r = aircraft_group.add<xf::sim::Wing> (main_wing_airfoil, kFoamDensity);
	wing_r.set_label ("wing R");
	wing_r.rotate_about_body_origin (wing_to_normal_rotation);
	// Move to the right:
	wing_r.translate ({ 0_m, +0.5 * kFuselageWidth + kWingLength, 0_m });
	wing_r.rotate_about_body_origin (xf::x_rotation<WorldSpace> (-kDihedral));

	// Winglet L

	auto& winglet_l = aircraft_group.add<xf::sim::Wing> (winglet_airfoil, kFoamDensity);
	winglet_l.set_label ("wing L/winglet L");
	winglet_l.rotate_about_body_origin (wing_to_normal_rotation);
	winglet_l.move_origin_to (wing_l.origin<WorldSpace>());
	winglet_l.translate ({ 0_m, -kWingLength, 0_m });
	winglet_l.rotate_about (xf::SpaceLength<WorldSpace> { 0_m, -0.5 * kFuselageWidth - main_wing_airfoil.wing_length(), 0_m }, xf::x_rotation<WorldSpace> (+kWingletAngle));
	winglet_l.rotate_about_world_origin (xf::x_rotation<WorldSpace> (+kDihedral));

	// Winglet R

	auto& winglet_r = aircraft_group.add<xf::sim::Wing> (winglet_airfoil, kFoamDensity);
	winglet_r.set_label ("wing R/winglet R");
	winglet_r.rotate_about_body_origin (wing_to_normal_rotation);
	winglet_r.move_origin_to (wing_r.origin<WorldSpace>());
	winglet_r.translate ({ 0_m, +kWingletLength, 0_m });
	winglet_r.rotate_about (xf::SpaceLength<WorldSpace> { 0_m, +0.5 * kFuselageWidth + main_wing_airfoil.wing_length(), 0_m }, xf::x_rotation<WorldSpace> (-kWingletAngle));
	winglet_r.rotate_about_world_origin (xf::x_rotation<WorldSpace> (-kDihedral));

	// Aileron L

	auto& aileron_l = aircraft_group.add<xf::sim::Wing> (aileron_airfoil, kFoamDensity);
	aileron_l.set_label ("wing L/aileron L");
	aileron_l.rotate_about_body_origin (wing_to_normal_rotation);
	aileron_l.move_origin_to (wing_l.origin<WorldSpace>() + xf::SpaceLength<WorldSpace> {
		-main_wing_airfoil.chord_length(),
		-main_wing_airfoil.wing_length() + aileron_airfoil.wing_length(),
		0_m,
	});
	aileron_l.rotate_about_world_origin (xf::x_rotation<WorldSpace> (+kDihedral));

	// Aileron R

	auto& aileron_r = aircraft_group.add<xf::sim::Wing> (aileron_airfoil, kFoamDensity);
	aileron_r.set_label ("wing R/aileron R");
	aileron_r.rotate_about_body_origin (wing_to_normal_rotation);
	aileron_r.move_origin_to (wing_r.origin<WorldSpace>() + xf::SpaceLength<WorldSpace> {
		-main_wing_airfoil.chord_length(),
		0_m,
		0_m,
	});
	aileron_r.rotate_about_world_origin (xf::x_rotation<WorldSpace> (-kDihedral));

	// Tail horizontal

	auto& tail_h = aircraft_group.add<xf::sim::Wing> (tail_h_airfoil, kFoamDensity);
	tail_h.set_label ("tail/stabilizer/horizontal");
	tail_h.rotate_about_body_origin (wing_to_normal_rotation);
	// Center to [0, 0, 0]:
	tail_h.translate ({ 0_m, 0.5 * kTailHorizontalStabilizerLength, 0_m });
	// Move to the tail:
	tail_h.translate ({ -1.5_m, 0_m, 0_m });
	// A bit of negative lift on the tail for longitudal stability:
	tail_h.rotate_about_body_origin (xf::y_rotation<WorldSpace> (-5_deg));

	// Elevator

	auto& elevator = aircraft_group.add<xf::sim::Wing> (elevator_airfoil, kFoamDensity);
	elevator.set_label ("tail/elevator");
	elevator.rotate_about_body_origin (wing_to_normal_rotation);
	elevator.move_origin_to (tail_h.origin<WorldSpace>());
	elevator.translate ({ -tail_h_airfoil.chord_length(), 0_m, 0_m });

	// Tail vertical

	auto& tail_v = aircraft_group.add<xf::sim::Wing> (tail_v_airfoil, kFoamDensity);
	tail_v.set_label ("tail/stabilizer/vertical");
	tail_v.rotate_about_body_origin (xf::y_rotation<WorldSpace> (-180_deg));
	tail_v.move_origin_to (tail_h.origin<WorldSpace>());
	tail_v.translate ({ 0_m, -0.5 * tail_h_airfoil.wing_length(), 0_m });

	// Rudder

	auto& rudder = aircraft_group.add<xf::sim::Wing> (rudder_airfoil, kFoamDensity);
	rudder.set_label ("tail/rudder");
	rudder.rotate_about_body_origin (xf::y_rotation<WorldSpace> (-180_deg));
	rudder.move_origin_to (tail_v.origin<WorldSpace>());
	rudder.translate ({ -tail_v_airfoil.chord_length(), 0_m, 0_m });

	// Prandtl tube

	auto& prandtl_tube = aircraft_group.add<xf::sim::PrandtlTube> (models.standard_atmosphere, xf::sim::PrandtlTubeParameters { .mass = 0.1_kg, .length = 20_cm, .diameter = 1_cm }); // TODO mass = 25 g
	prandtl_tube.set_label ("Prandtl tube");
	prandtl_tube.move_origin_to (wing_l.origin<WorldSpace>() + xf::SpaceLength<WorldSpace> (0_m, -0.75 * main_wing_airfoil.wing_length(), 0_m));

	// Constraints

	auto& wing_l_hinge = rigid_body_system.add<rb::HingePrecalculation> (fuselage, wing_l, wing_l.origin<BodyCOM>(), wing_l.origin<BodyCOM>() - x_wing_chord);
	auto& wing_r_hinge = rigid_body_system.add<rb::HingePrecalculation> (fuselage, wing_r, wing_r.origin<BodyCOM>() + z_wing_length, wing_r.origin<BodyCOM>() + z_wing_length - x_wing_chord);

	auto& winglet_l_hinge = rigid_body_system.add<rb::HingePrecalculation> (wing_l, winglet_l, winglet_l.origin<BodyCOM>(), winglet_l.origin<BodyCOM>() + x_wing_chord);
	auto& winglet_r_hinge = rigid_body_system.add<rb::HingePrecalculation> (wing_r.origin<BodyCOM>(), wing_r.origin<BodyCOM>() + x_wing_chord, wing_r, winglet_r);

	auto& aileron_l_hinge = rigid_body_system.add<rb::HingePrecalculation> (wing_l, aileron_l, aileron_l.origin<BodyCOM>(), aileron_l.origin<BodyCOM>() + z_aileron_length);
	auto& aileron_r_hinge = rigid_body_system.add<rb::HingePrecalculation> (wing_r, aileron_r, aileron_r.origin<BodyCOM>(), aileron_r.origin<BodyCOM>() + z_aileron_length);

	auto& elevator_hinge = rigid_body_system.add<rb::HingePrecalculation> (tail_h, elevator, elevator.origin<BodyCOM>(), elevator.origin<BodyCOM>() + z_elevator_length);
	auto& rudder_hinge = rigid_body_system.add<rb::HingePrecalculation> (tail_v, rudder, rudder.origin<BodyCOM>(), rudder.origin<BodyCOM>() + z_rudder_length);

	// Servo limits:
	rigid_body_system.add<rb::AngularLimitsConstraint> (aileron_l_hinge, xf::Range { -60_deg, +60_deg });
	rigid_body_system.add<rb::AngularLimitsConstraint> (aileron_r_hinge, xf::Range { -60_deg, +60_deg });
	rigid_body_system.add<rb::AngularLimitsConstraint> (elevator_hinge, xf::Range { -60_deg, +60_deg });
	rigid_body_system.add<rb::AngularLimitsConstraint> (rudder_hinge, xf::Range { -60_deg, +60_deg });

	auto& aileron_l_servo_constraint = rigid_body_system.add (rb::make_standard_servo_constraint (aileron_l_hinge, 3));
	auto& aileron_r_servo_constraint = rigid_body_system.add (rb::make_standard_servo_constraint (aileron_r_hinge, 3));
	auto& elevator_servo_constraint = rigid_body_system.add (rb::make_standard_servo_constraint (elevator_hinge, 3));
	auto& rudder_servo_constraint = rigid_body_system.add (rb::make_standard_servo_constraint (rudder_hinge, 3));

	auto& aileron_l_servo = aircraft_group.add (xf::sim::make_standard_servo (aileron_l_servo_constraint, 3));
	aileron_l_servo.set_label ("wing L/aileron L/servo");
	aileron_l_servo.move_to (wing_l.placement().position());

	auto& aileron_r_servo = aircraft_group.add (xf::sim::make_standard_servo (aileron_r_servo_constraint, 3));
	aileron_r_servo.set_label ("wing R/aileron R/servo");
	aileron_r_servo.move_to (wing_r.placement().position());

	auto& elevator_servo = aircraft_group.add (xf::sim::make_standard_servo (elevator_servo_constraint, 3));
	elevator_servo.set_label ("tail/elevator/servo");
	elevator_servo.move_to (tail_h.placement().position());

	auto& rudder_servo = aircraft_group.add (xf::sim::make_standard_servo (rudder_servo_constraint, 3));
	rudder_servo.set_label ("tail/rudder/servo");
	rudder_servo.move_to (tail_v.placement().position());

	aileron_l_servo.set_orientation (xf::sim::ServoOrientation::Reversed);
	elevator_servo.set_orientation (xf::sim::ServoOrientation::Reversed);

	aileron_l_servo.constraint().set_voltage (6_V);
	aileron_r_servo.constraint().set_voltage (6_V);
	elevator_servo.constraint().set_voltage (6_V);
	rudder_servo.constraint().set_voltage (6_V);

	auto& prandtl_tube_fixed_constraint = rigid_body_system.add<rb::FixedConstraint> (wing_l, prandtl_tube);

	std::vector<rb::Constraint*> constraints = {
		&rigid_body_system.add<rb::AngularSpringConstraint> (wing_l_hinge, rb::angular_spring_function (30_Nm / 1_deg)),
		&rigid_body_system.add<rb::AngularSpringConstraint> (wing_r_hinge, rb::angular_spring_function (30_Nm / 1_deg)),
		&rigid_body_system.add<rb::HingeConstraint> (wing_l_hinge),
		&rigid_body_system.add<rb::HingeConstraint> (wing_r_hinge),
		&rigid_body_system.add<rb::FixedConstraint> (fuselage, tail_h),
		&rigid_body_system.add<rb::FixedConstraint> (tail_h, tail_v),
		&rigid_body_system.add<rb::AngularSpringConstraint> (winglet_l_hinge, rb::angular_spring_function (10_Nm / 1_deg)),
		&rigid_body_system.add<rb::AngularSpringConstraint> (winglet_r_hinge, rb::angular_spring_function (10_Nm / 1_deg)),
		&rigid_body_system.add<rb::HingeConstraint> (winglet_l_hinge),
		&rigid_body_system.add<rb::HingeConstraint> (winglet_r_hinge),
		&rigid_body_system.add<rb::HingeConstraint> (aileron_l_hinge),
		&rigid_body_system.add<rb::HingeConstraint> (aileron_r_hinge),
		&rigid_body_system.add<rb::HingeConstraint> (elevator_hinge),
		&rigid_body_system.add<rb::HingeConstraint> (rudder_hinge),
		&prandtl_tube_fixed_constraint,
		&rigid_body_system.add<rb::FixedConstraint> (wing_l, aileron_l_servo),
		&rigid_body_system.add<rb::FixedConstraint> (wing_r, aileron_r_servo),
		&rigid_body_system.add<rb::FixedConstraint> (tail_h, elevator_servo),
		&rigid_body_system.add<rb::FixedConstraint> (tail_v, rudder_servo),
	};

	// TODO Virtual sensors mounted on rigid bodies should "fail" as in stop reporting any data if the body breaks off the main fuselage or something.
	for (auto& constraint: constraints)
		constraint->set_breaking_force_torque (10'000_N, 1'000_Nm);

	rigid_body_system.set_baumgarte_factor (0.3);
	rigid_body_system.set_constraint_force_mixing_factor (1e-3);
	prandtl_tube_fixed_constraint.set_baumgarte_factor (0.6);

	return SimulatedAircraft {
		.rigid_group		= std::move (aircraft_group),
		.primary_body		= fuselage,
		.aileron_l_servo	= aileron_l_servo,
		.aileron_r_servo	= aileron_r_servo,
		.elevator_servo		= elevator_servo,
		.rudder_servo		= rudder_servo,
	};
}

} // namespace sim1::aircraft

