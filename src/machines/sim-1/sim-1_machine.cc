/* vim:ts=4
 *
 * Copyleft 2008…2017  Michał Gawron
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
#include "sim-1_machine.h"

// Xefis:
#include <machines/sim-1/airfoils/control_surface_airfoil.h>
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/math/lonlat_radius.h>
#include <xefis/support/math/tait_bryan_angles.h>
#include <xefis/support/math/position_rotation.h>
#include <xefis/support/simulation/constraints/angular_servo_constraint.h>
#include <xefis/support/simulation/constraints/angular_spring_constraint.h>
#include <xefis/support/simulation/constraints/fixed_constraint.h>
#include <xefis/support/simulation/constraints/hinge_constraint.h>
#include <xefis/support/simulation/devices/wing.h>
#include <xefis/support/simulation/rigid_body/frames.h>
#include <xefis/support/simulation/rigid_body/group.h>
#include <xefis/support/simulation/rigid_body/utility.h>
#include <xefis/support/simulation/rigid_body/various_shapes.h>
#include <xefis/xefis_machine.h>

// Neutrino:
#include <neutrino/math/math.h>

// Standard:
#include <cstddef>
#include <thread>


namespace rb = xf::rigid_body;


Sim1Machine::Sim1Machine (xf::Xefis& xefis):
	Machine (xefis),
	_logger (xefis.logger())
{
	_rigid_body_system.set_atmosphere_model (&_standard_atmosphere);
	auto aircraft = construct_aircraft();

	auto const kXFrontZUpToStandardAircraftFrameRotation = xf::x_rotation<rb::WorldSpace> (180_deg) * xf::z_rotation<rb::WorldSpace> (-90_deg);

	// Transform to standard aircraft coordinates (X-front, Y-right, Z-down):
	aircraft.rigid_group.rotate_about_world_origin (kXFrontZUpToStandardAircraftFrameRotation);

	auto const location = xf::LonLatRadius (0_deg, 45_deg, xf::kEarthMeanRadius + 11_km);
	auto const tait_bryan_angles = xf::TaitBryanAngles (-45_deg, 0_deg, 0_deg);
	aircraft.rigid_group.rotate_about_world_origin (math::reframe<rb::WorldSpace, rb::WorldSpace> (airframe_to_ecef_rotation (tait_bryan_angles, location)));
	aircraft.rigid_group.translate (math::reframe<rb::WorldSpace, void> (xf::cartesian (location)));

	auto& earth = _rigid_body_system.add_gravitational (rb::make_earth());

	_rigid_body_solver.set_baumgarte_factor (0.8);

	_simulation.emplace (300_Hz, _logger, [&] (si::Time const dt) {
		_rigid_body_solver.evolve (dt);
		_electrical_network_solver.evolve (dt);
	});

	QWidget w (nullptr);
	auto const lh = neutrino::default_line_height (&w);
	auto tt = 0_s;

	_rigid_body_viewer.emplace (_rigid_body_system, QSize (50 * lh, 50 * lh), xf::RigidBodyViewer::AutoFPS, [&, aircraft_copy = aircraft] (si::Time const dt) {
		_simulation->evolve (dt, 1_s);

		auto const angle = 60_deg * std::sin (tt / 1_s);
		std::cout << angle << "\n";
		aircraft_copy.aileron_l_servo->set_setpoint (+angle);
		aircraft_copy.aileron_r_servo->set_setpoint (-angle);

		tt += dt;
	});
	_rigid_body_viewer->set_followed (aircraft.center_body);
	_rigid_body_viewer->set_planet (&earth);
	_rigid_body_viewer->show();
}


Aircraft
Sim1Machine::construct_aircraft()
{
	auto const kDihedral = 0_deg;
	auto const kWingletAngle = 30_deg;
	auto const kFoamDensity = 0.12_kg / (3600000 * 1_mm * 1_mm * 1_mm);

	auto const z_minus_90_rotation = xf::z_rotation<rb::WorldSpace> (-90_deg);
	auto const wing_to_normal_rotation = z_minus_90_rotation * xf::x_rotation<rb::WorldSpace> (+90_deg);

	auto const main_wing_airfoil_spline = xf::AirfoilSpline (sim1::control_surface_airfoil::kSpline);
	auto const main_wing_airfoil_characteristics =
		xf::AirfoilCharacteristics (main_wing_airfoil_spline,
									sim1::control_surface_airfoil::kLiftField,
									sim1::control_surface_airfoil::kDragField,
									sim1::control_surface_airfoil::kPitchingMomentField,
									sim1::control_surface_airfoil::kCenterOfPressureOffsetField);

	auto const main_wing_airfoil = xf::Airfoil (main_wing_airfoil_characteristics, 50_cm, 2_m);
	auto const winglet_airfoil = xf::Airfoil (main_wing_airfoil_characteristics, 50_cm, 50_cm);
	auto const aileron_airfoil = xf::Airfoil (main_wing_airfoil_characteristics, 15_cm, 80_cm);

	auto aircraft_group = _rigid_body_system.make_group();

	// Fuselage

	auto& fuselage = aircraft_group.add<rb::Body> (xf::MassMoments<rb::BodySpace> { 1_kg, math::zero, math::unit });
	fuselage.translate ({ 0_m, +0.8_m, 0_m });
	auto const fuselage_width = 20_cm;

	// Wing L

	auto& wing_l = aircraft_group.add<xf::sim::Wing> (main_wing_airfoil, kFoamDensity);
	wing_l.rotate_about_center_of_mass (wing_to_normal_rotation);
	wing_l.translate ({ -0.5 * fuselage_width - 0.5 * main_wing_airfoil.wing_length(), 0_m, 0_m });
	wing_l.rotate_about_world_origin (xf::y_rotation<rb::WorldSpace> (+kDihedral));

	// Wing R

	auto& wing_r = aircraft_group.add<xf::sim::Wing> (main_wing_airfoil, kFoamDensity);
	wing_r.rotate_about_center_of_mass (wing_to_normal_rotation);
	{
		auto wing_r_origin = wing_r.origin<rb::BodySpace>();
		wing_r_origin[2] *= -1;
		wing_r.set_origin_at (wing_r_origin);
	}
	wing_r.translate ({ +0.5 * fuselage_width + 0.5 * main_wing_airfoil.wing_length(), 0_m, 0_m });
	wing_r.rotate_about_world_origin (xf::y_rotation<rb::WorldSpace> (-kDihedral));

	// Winglet L

	auto& winglet_l = aircraft_group.add<xf::sim::Wing> (winglet_airfoil, kFoamDensity);
	winglet_l.rotate_about_center_of_mass (wing_to_normal_rotation);
	winglet_l.translate ({ -0.5 * fuselage_width - main_wing_airfoil.wing_length() - 0.5 * winglet_airfoil.wing_length(), 0_m, 0_m });
	winglet_l.rotate_about (xf::SpaceLength<rb::WorldSpace> { -0.5 * fuselage_width - main_wing_airfoil.wing_length(), 0_m, 0_m }, xf::y_rotation<rb::WorldSpace> (+kWingletAngle));
	winglet_l.rotate_about_world_origin (xf::y_rotation<rb::WorldSpace> (+kDihedral));

	// Winglet R

	auto& winglet_r = aircraft_group.add<xf::sim::Wing> (winglet_airfoil, kFoamDensity);
	winglet_r.rotate_about_center_of_mass (wing_to_normal_rotation);
	{
		auto winglet_r_origin = winglet_r.origin<rb::BodySpace>();
		winglet_r_origin[2] *= -1;
		winglet_r.set_origin_at (winglet_r_origin);
	}
	winglet_r.translate ({ +0.5 * fuselage_width + main_wing_airfoil.wing_length() + 0.5 * winglet_airfoil.wing_length(), 0_m, 0_m });
	winglet_r.rotate_about (xf::SpaceLength<rb::WorldSpace> { +0.5 * fuselage_width + main_wing_airfoil.wing_length(), 0_m, 0_m }, xf::y_rotation<rb::WorldSpace> (-kWingletAngle));
	winglet_r.rotate_about_world_origin (xf::y_rotation<rb::WorldSpace> (-kDihedral));

	// Aileron L

	// FIXME This is needed so we don't have too much difference between body inertias:
	auto const aileron_multiplier = 10;

	auto& aileron_l = aircraft_group.add<xf::sim::Wing> (aileron_airfoil, aileron_multiplier * kFoamDensity);
	aileron_l.rotate_about_center_of_mass (wing_to_normal_rotation);
	aileron_l.move_origin_to (wing_l.origin<rb::WorldSpace>() + xf::SpaceLength<rb::WorldSpace> {
		-main_wing_airfoil.wing_length() + aileron_airfoil.wing_length(),
		-main_wing_airfoil.chord_length(),
		0_m,
	});
	aileron_l.rotate_about_world_origin (xf::y_rotation<rb::WorldSpace> (+kDihedral));

	// Aileron R

	auto& aileron_r = aircraft_group.add<xf::sim::Wing> (aileron_airfoil, aileron_multiplier * kFoamDensity);
	aileron_r.rotate_about_center_of_mass (wing_to_normal_rotation);
	aileron_r.move_origin_to (wing_r.origin<rb::WorldSpace>() + xf::SpaceLength<rb::WorldSpace> {
		+main_wing_airfoil.wing_length(),
		-main_wing_airfoil.chord_length(),
		0_m,
	});
	aileron_r.rotate_about_world_origin (xf::y_rotation<rb::WorldSpace> (-kDihedral));

	// TODO Spoilers, perhaps?

	// Tail horizontal

	auto const tail_h_airfoil = xf::Airfoil (main_wing_airfoil_characteristics, 40_cm, 1_m);
	auto& tail_h = aircraft_group.add<xf::sim::Wing> (tail_h_airfoil, kFoamDensity);
	tail_h.rotate_about_center_of_mass (wing_to_normal_rotation);
	tail_h.translate ({ 0_m, -1.5_m, 0_m });

	// A bit of negative lift on the tail for longitudal stability:
	tail_h.rotate_about_center_of_mass (xf::x_rotation<rb::WorldSpace> (-2_deg));

	// Tail vertical

	auto const tail_v_airfoil = xf::Airfoil (main_wing_airfoil_characteristics, 40_cm, 0.5_m);
	auto& tail_v = aircraft_group.add<xf::sim::Wing> (tail_v_airfoil, kFoamDensity);
	tail_v.rotate_about_center_of_mass (z_minus_90_rotation);
	tail_v.translate ({ 0_m, -1.5_m, 0.25_m });

	// Constraints

	auto const y_1_m = xf::SpaceLength<rb::BodySpace> { 0_m, 1_m, 0_m };
	auto const x_1_m = xf::SpaceLength<rb::BodySpace> { 1_m, 0_m, 0_m };
	auto const x_wing_chord = xf::SpaceLength<rb::BodySpace> { main_wing_airfoil.chord_length(), 0_m, 0_m };
	auto const z_w = xf::SpaceLength<rb::BodySpace> { 0_m, 0_m, main_wing_airfoil.wing_length() };

	auto& wing_l_hinge = _rigid_body_system.add<rb::HingePrecalculation> (fuselage, wing_l, wing_l.origin<rb::BodySpace>(), wing_l.origin<rb::BodySpace>() + y_1_m);
	auto& wing_r_hinge = _rigid_body_system.add<rb::HingePrecalculation> (fuselage, wing_r, wing_r.origin<rb::BodySpace>(), wing_r.origin<rb::BodySpace>() + y_1_m);

	auto& winglet_l_hinge = _rigid_body_system.add<rb::HingePrecalculation> (wing_l.origin<rb::BodySpace>() + z_w, wing_l.origin<rb::BodySpace>() + z_w + x_1_m, wing_l, winglet_l);
	auto& winglet_r_hinge = _rigid_body_system.add<rb::HingePrecalculation> (wing_r.origin<rb::BodySpace>() - z_w, wing_r.origin<rb::BodySpace>() - z_w + x_1_m, wing_r, winglet_r);

	auto& aileron_l_hinge = _rigid_body_system.add<rb::HingePrecalculation> (wing_l.origin<rb::BodySpace>() + x_wing_chord, wing_l.origin<rb::BodySpace>() + x_wing_chord + z_w, wing_l, aileron_l);
	auto& aileron_r_hinge = _rigid_body_system.add<rb::HingePrecalculation> (wing_r.origin<rb::BodySpace>() + x_wing_chord, wing_r.origin<rb::BodySpace>() + x_wing_chord + z_w, wing_r, aileron_r);

	auto& c1   = _rigid_body_system.add<rb::AngularSpringConstraint> (wing_l_hinge, rb::angular_spring_function (3_Nm / 1_deg));
	auto& c2   = _rigid_body_system.add<rb::AngularSpringConstraint> (wing_r_hinge, rb::angular_spring_function (3_Nm / 1_deg));
	auto& c3   = _rigid_body_system.add<rb::HingeConstraint> (wing_l_hinge);
	auto& c4   = _rigid_body_system.add<rb::HingeConstraint> (wing_r_hinge);
	auto& c5   = _rigid_body_system.add<rb::FixedConstraint> (fuselage, tail_h);
	auto& c6   = _rigid_body_system.add<rb::FixedConstraint> (tail_h, tail_v);
	auto& c7   = _rigid_body_system.add<rb::AngularSpringConstraint> (winglet_l_hinge, rb::angular_spring_function (3_Nm / 1_deg));
	auto& c8   = _rigid_body_system.add<rb::AngularSpringConstraint> (winglet_r_hinge, rb::angular_spring_function (3_Nm / 1_deg));
	auto& c9   = _rigid_body_system.add<rb::HingeConstraint> (winglet_l_hinge);
	auto& c10  = _rigid_body_system.add<rb::HingeConstraint> (winglet_r_hinge);
	auto& c11  = _rigid_body_system.add<rb::HingeConstraint> (aileron_l_hinge);
	auto& c11s = _rigid_body_system.add (rb::make_standard_9gram_servo_constraint (aileron_l_hinge));
	auto& c12  = _rigid_body_system.add<rb::HingeConstraint> (aileron_r_hinge);
	auto& c12s = _rigid_body_system.add (rb::make_standard_9gram_servo_constraint (aileron_r_hinge));

	c11s.set_voltage (6_V);
	c12s.set_voltage (6_V);

	for (auto& constraint: std::vector<rb::Constraint*> { &c1, &c2, &c3, &c4, &c5, &c6, &c7, &c8, &c9, &c10, &c11, &c12 })
		constraint->set_breaking_force_torque (1000'000_N, 500'000_Nm);

	return {
		std::move (aircraft_group),
		&fuselage,
		&c11s,
		&c12s,
	};
}


std::unique_ptr<xf::Machine>
xefis_machine (xf::Xefis& xefis)
{
	return std::make_unique<Sim1Machine> (xefis);
}

