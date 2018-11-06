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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/simulation/airfoil_shape.h>

// Local:
#include "airfoils/sim_airfoil.h"
#include "airfoils/sim_airfoil_spline.h"
#include "sim_airplane.h"


SimAirplane::SimAirplane (std::unique_ptr<SimAirplaneIO> module_io, xf::Logger const& logger, std::string_view const& instance):
	Module (std::move (module_io), instance)
{
	constexpr double kMassScaler = 1.0 / 6;

	// TODO consider creating struct PointMass
	std::vector<std::tuple<si::Mass, xf::SpaceVector<si::Length, xf::BodyFrame>>> point_masses {
		{ kMassScaler * 1_kg, { +1_m, 0_m, 0_m } },
		{ kMassScaler * 1_kg, { -1_m, 0_m, 0_m } },
		{ kMassScaler * 1_kg, { 0_m, +1_m, 0_m } },
		{ kMassScaler * 1_kg, { 0_m, -1_m, 0_m } },
		{ kMassScaler * 1_kg, { 0_m, 0_m, +1_m } },
		{ kMassScaler * 1_kg, { 0_m, 0_m, -1_m } },
		// // Forward mass:
		// { kMassScaler * 1_kg, { +0.5_m, 0_m, 0_m } },
		// // Left wing:
		// { kMassScaler * 0.2_kg, { +0.1_m, -1_m, 0_m } },
		// // Right wing:
		// { kMassScaler * 0.2_kg, { +0.1_m, +1_m, 0_m } },
		// // Left engine:
		// { kMassScaler * 1.00_kg, { +0.1_m, -0.5_m, 1_m } },
		// // Right engine:
		// { kMassScaler * 1.00_kg, { +0.1_m, +0.5_m, 1_m } },
		// // Tail:
		// { kMassScaler * 0.02_kg, { -1_m, 0_m, -0.2_m } },
		// // Left wheel:
		// //{ kMassScaler * 0.090_kg, { -0.1_m, -0.5_m, -0.05_m } },
		// // Right wheel:
		// //{ kMassScaler * 0.090_kg, { -0.1_m, +0.5_m, -0.05_m } },
	};
	auto const com = xf::move_to_center_of_gravity<xf::BodyFrame> (point_masses.begin(), point_masses.end());

	xf::SpaceVector<si::Length, xf::ECEFFrame> const
	airframe_ecef_position =
		xf::cartesian (xf::LonLatRadius (0_deg, 0_deg, xf::kEarthMeanRadius + 1_km));

	xf::SpaceVector<si::Velocity, xf::ECEFFrame> const
	airframe_ecef_velocity { 0_mps, 0_mps, 20_kt };

	xf::SpaceMatrix<double, xf::ECEFFrame, xf::BodyFrame> const
	airframe_ecef_orientation =
		xf::body_to_ecef_transform (xf::TaitBryanAngles { 0_deg, 0_deg, 0_deg }, airframe_ecef_position);

	xf::SpaceMatrix<si::MomentOfInertia, xf::BodyFrame> const
	airframe_moment_of_inertia =
		xf::moment_of_inertia<xf::BodyFrame> (point_masses.begin(), point_masses.end());

	xf::SpaceVector<si::BaseAngularVelocity, xf::ECEFFrame> const
	airframe_angular_velocity { math::zero };

	auto shape = make_airframe_shape (com);
	shape.set_mass (xf::total_mass (point_masses.begin(), point_masses.end()));
	shape.set_moment_of_inertia (airframe_moment_of_inertia);

	xf::sim::Airframe airframe (std::move (shape));
	airframe.set_position (airframe_ecef_position);
	airframe.set_velocity (airframe_ecef_velocity);
	airframe.set_orientation (airframe_ecef_orientation);
	airframe.set_angular_velocity (airframe_angular_velocity);

	_simulation = std::make_unique<xf::sim::FlightSimulation> (std::move (airframe), 240_Hz, logger.with_scope ("simulation")); // TODO want 600 Hz?
}


void
SimAirplane::process (xf::Cycle const& cycle)
{
	set_inputs();
	_simulation->evolve (cycle.update_dt(), cycle.intended_update_dt());
	set_outputs();
}


void
SimAirplane::set_inputs()
{
	_controls.elevator->control().deflection_angle = -10_deg * io.joystick_y_axis.value_or (0.0) + /* TRIM: */ 1_deg;
	_controls.aileron_L->control().deflection_angle = -10_deg * io.joystick_x_axis.value_or (0.0);
	_controls.aileron_R->control().deflection_angle = +10_deg * io.joystick_x_axis.value_or (0.0);
	_controls.engine_L->control().power = 120_W * io.joystick_throttle.value_or (0.0);
	_controls.engine_R->control().power = 120_W * io.joystick_throttle.value_or (0.0);
	_controls.rudder->control().deflection_angle = 10_deg * io.joystick_rudder.value_or (0.0);
}


void
SimAirplane::set_outputs()
{
	auto const& airframe = _simulation->airframe();

	// Position:
	si::Length const altitude_amsl = abs (airframe.position()) - xf::kEarthMeanRadius;
	// Real velocities:
	xf::SpaceVector<si::Velocity, xf::BodyFrame> const airframe_velocity = airframe.ecef_to_body_transform() * airframe.velocity();
	xf::SpaceVector<si::Velocity, xf::NEDFrame> const ned_velocity = body_to_ned_transform (airframe.position(), airframe.orientation()) * airframe_velocity;
	xf::SpaceVector<si::Velocity, xf::NEDFrame> const ground_velocity { ned_velocity[0], ned_velocity[1], 0_mps };
	si::Velocity const true_airspeed = abs (airframe.velocity());
	// Atmosphere:
	xf::sim::Atmosphere::State<xf::BodyFrame> const atmstate = _simulation->complete_atmosphere_state_at ({ math::zero });
	si::Pressure const pressure_total = atmstate.air.pressure + xf::dynamic_pressure (atmstate.air.density, true_airspeed);
	// Real orientation:
	xf::TaitBryanAngles const tba_orientation = xf::tait_bryan_angles (airframe.orientation(), airframe.position());
	// Real track:
	xf::SpaceVector<double, xf::ECEFFrame> x = airframe.velocity() / 1_mps;
	xf::SpaceVector<double, xf::ECEFFrame> y = airframe.position() / 1_m;
	xf::SpaceVector<double, xf::ECEFFrame> z = cross_product (x, y);
	xf::SpaceMatrix<double, xf::ECEFFrame, void> const velocity_matrix_1 {{ x, y, z }};
	xf::SpaceMatrix<double, xf::ECEFFrame, void> const velocity_matrix_2 (orthogonalized (velocity_matrix_1));
	xf::SpaceMatrix<double, xf::ECEFFrame, xf::BodyFrame> const velocity_matrix (velocity_matrix_2.array());
	xf::TaitBryanAngles const real_track = xf::tait_bryan_angles (velocity_matrix, airframe.position());
	// AOA:
	auto const aoa_l = _controls.wing_L->control().angle_of_attack;
	auto const aoa_r = _controls.wing_R->control().angle_of_attack;
	// Position on Earth:
	auto const earth_position = polar (airframe.position());
	// IMU:
	xf::SpaceVector<si::Force, xf::BodyFrame> const body_forces = airframe.ecef_to_body_transform() * _simulation->airframe_forces().force();

	// Set output properties:
	io.real_cas = sqrt (2 * (pressure_total - atmstate.air.pressure) / atmstate.air.density);
	io.real_ground_speed = abs (ground_velocity);
	io.real_vertical_speed = ned_velocity[2];
	io.real_sat = atmstate.air.temperature;
	io.real_orientation_pitch = tba_orientation.pitch();
	io.real_orientation_roll = tba_orientation.roll();
	io.real_orientation_heading_true = tba_orientation.yaw();
	io.real_track_lateral_true = real_track.yaw();
	io.real_track_vertical = real_track.pitch();
	io.real_altitude_amsl = altitude_amsl;
	io.real_altitude_agl = io.real_altitude_amsl;
	io.real_aoa_alpha = std::min (aoa_l.alpha, aoa_r.alpha);
	io.real_aoa_alpha_maximum = 0.5_rad; // TODO unhardcode/compute from airfoil
	io.real_aoa_beta = 0.5 * (aoa_l.beta + aoa_r.beta);
	io.real_position_longitude = earth_position.lon();
	io.real_position_latitude = earth_position.lat();
	io.real_slip_skid = atan2 (body_forces[1], -body_forces[2]);

	// TODO
	io.requested_engine_left_power = _controls.engine_L->control().power;
	io.engine_left_power = _controls.engine_L->control().power;
	io.engine_left_thrust = _controls.engine_L->control().thrust;
	io.requested_engine_right_power = _controls.engine_R->control().power;
	io.engine_right_power = _controls.engine_R->control().power;
	io.engine_right_thrust = _controls.engine_R->control().thrust;
}


xf::sim::BodyShape
SimAirplane::make_airframe_shape (xf::SpaceVector<si::Length, xf::BodyFrame> const& center_of_mass)
{
	si::Angle const dihedral = 10_deg;
	si::Angle const sweep_angle = 0_deg;

	xf::sim::AirfoilShape const wing_shape {
		sim_airfoil::kSpline,
		1_m,
		15_cm,
		{ 0_m, 0_m, 0_m },
		sim_airfoil::kLiftField,
		sim_airfoil::kDragField,
		sim_airfoil::kPitchingMomentField,
		sim_airfoil::kCenterOfPressureOffsetField,
	};

	xf::sim::AirfoilShape const aileron_shape {
		sim_airfoil::kSpline,
		20_cm,
		5_cm,
		{ 0_m, 0_m, 0_m },
		sim_airfoil::kLiftField,
		sim_airfoil::kDragField,
		sim_airfoil::kPitchingMomentField,
		sim_airfoil::kCenterOfPressureOffsetField
	};

	xf::sim::AirfoilShape const elevator_shape {
		sim_airfoil::kSpline,
		20_cm,
		5_cm,
		{ 0_m, 0_m, 0_m },
		sim_airfoil::kLiftField,
		sim_airfoil::kDragField,
		sim_airfoil::kPitchingMomentField,
		sim_airfoil::kCenterOfPressureOffsetField
	};

	xf::sim::AirfoilShape const rudder_shape {
		sim_airfoil::kSpline,
		20_cm,
		5_cm,
		{ 0_m, 0_m, 0_m },
		sim_airfoil::kLiftField,
		sim_airfoil::kDragField,
		sim_airfoil::kPitchingMomentField,
		sim_airfoil::kCenterOfPressureOffsetField
	};

	xf::SpaceMatrix<si::MomentOfInertia, xf::PartFrame> const unit_moi { math::unit };
	auto const std_moi = unit_moi / 20;

	auto wing_L = std::make_unique<xf::sim::Airfoil> (
		wing_shape,
		xf::SpaceVector<si::Length, xf::BodyFrame> { -0.25 * wing_shape.chord_length(), -0.5_m, 0_m },
		0.1_kg,
		std_moi
	);
	wing_L->set_mount_rotation (xf::x_rotation<xf::BodyFrame> (+dihedral) * xf::z_rotation<xf::BodyFrame> (-sweep_angle) * xf::sim::airfoil_shape_to_body_rotation_for_wing());

	auto wing_R = std::make_unique<xf::sim::Airfoil> (
		wing_shape,
		xf::SpaceVector<si::Length, xf::BodyFrame> { -0.25 * wing_shape.chord_length(), +0.5_m, 0_m },
		0.1_kg,
		std_moi
	);
	wing_R->set_mount_rotation (xf::x_rotation<xf::BodyFrame> (-dihedral) * xf::z_rotation<xf::BodyFrame> (+sweep_angle) * xf::sim::airfoil_shape_to_body_rotation_for_wing());

	auto aileron_L = std::make_unique<xf::sim::Airfoil> (
		aileron_shape,
		xf::SpaceVector<si::Length, xf::BodyFrame> { -15_cm, -0.8_m, 0_m },
		0.01_kg,
		std_moi
	);
	aileron_L->set_mount_rotation (xf::x_rotation<xf::BodyFrame> (+dihedral) * xf::z_rotation<xf::BodyFrame> (-sweep_angle) * xf::sim::airfoil_shape_to_body_rotation_for_wing());

	auto aileron_R = std::make_unique<xf::sim::Airfoil> (
		aileron_shape,
		xf::SpaceVector<si::Length, xf::BodyFrame> { -15_cm, +0.8_m, 0_m },
		0.01_kg,
		std_moi
	);
	aileron_R->set_mount_rotation (xf::x_rotation<xf::BodyFrame> (-dihedral) * xf::z_rotation<xf::BodyFrame> (+sweep_angle) * xf::sim::airfoil_shape_to_body_rotation_for_wing());

	auto elevator = std::make_unique<xf::sim::Airfoil> (
		elevator_shape,
		xf::SpaceVector<si::Length, xf::BodyFrame> { -2_m, 0_m, 0_m },
		0.01_kg,
		std_moi
	);
	elevator->set_mount_rotation (xf::sim::airfoil_shape_to_body_rotation_for_wing());

	auto rudder = std::make_unique<xf::sim::Airfoil> (
		rudder_shape,
		xf::SpaceVector<si::Length, xf::BodyFrame> { -2_m, -0.2_m, 0.0_m },
		0.01_kg,
		std_moi
	);
	rudder->set_mount_rotation (xf::sim::airfoil_shape_to_body_rotation_for_rudder());

	auto engine_L = std::make_unique<xf::sim::Engine> (
		xf::SpaceVector<si::Length, xf::BodyFrame> { 10_cm, -30_cm, 1_cm },
		0.1_kg,
		std_moi
	);
	engine_L->set_mount_rotation (math::unit);

	auto engine_R = std::make_unique<xf::sim::Engine> (
		xf::SpaceVector<si::Length, xf::BodyFrame> { 10_cm, +30_cm, 1_cm },
		0.1_kg,
		std_moi
	);
	engine_R->set_mount_rotation (math::unit);

	xf::sim::BodyShape airframe_shape;

	_controls.wing_L	= &airframe_shape.add (std::move (wing_L));
	_controls.wing_R	= &airframe_shape.add (std::move (wing_R));
	_controls.aileron_L	= &airframe_shape.add (std::move (aileron_L));
	_controls.aileron_R	= &airframe_shape.add (std::move (aileron_R));
	_controls.elevator	= &airframe_shape.add (std::move (elevator));
	_controls.rudder	= &airframe_shape.add (std::move (rudder));
	_controls.engine_L	= &airframe_shape.add (std::move (engine_L));
	_controls.engine_R	= &airframe_shape.add (std::move (engine_R));

	return airframe_shape;
}

