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
#include <xefis/support/math/euler_angles.h>
#include <xefis/support/math/tait_bryan_angles.h>
#include <xefis/support/math/transforms.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/nature/mass_moments.h>
#include <xefis/support/simulation/rigid_body/concepts.h>
#include <xefis/support/simulation/rigid_body/impulse_solver.h>
#include <xefis/support/simulation/rigid_body/system.h>
#include <xefis/support/simulation/rigid_body/utility.h>
#include <xefis/support/simulation/simulation.h>

// Neutrino:
#include <neutrino/test/auto_test.h>

// Standard:
#include <cstddef>
#include <memory>
#include <string>


namespace xf::test {
namespace {

constexpr auto	kISSHeight = xf::kEarthMeanRadius + 405.5_km;

xf::Logger g_null_logger;


std::unique_ptr<rigid_body::Body>
make_iss()
{
	xf::SpaceLength<xf::ECEFSpace> const iss_ecef_position = xf::cartesian (xf::LonLatRadius (0_deg, 0_deg, kISSHeight));
	xf::RotationMatrix<rigid_body::WorldSpace> const iss_ecef_rotation = math::reframe<rigid_body::WorldSpace, rigid_body::WorldSpace> (xf::airframe_to_ecef_rotation (xf::TaitBryanAngles { 0_deg, 0_deg, 0_deg }, iss_ecef_position));
	xf::SpaceVector<si::Velocity, rigid_body::WorldSpace> const iss_velocity { 0_mps, 0_mps, 27'600_kph };
	xf::SpaceVector<si::AngularVelocity, rigid_body::WorldSpace> const iss_angular_velocity (math::zero);
	xf::MassMoments<rigid_body::BodySpace> const iss_mass_moments (419'725_kg, math::zero, math::unit);

	auto body = std::make_unique<rigid_body::Body> (iss_mass_moments);
	auto loc = body->location();
	loc.set_position (math::reframe<rigid_body::WorldSpace, void> (iss_ecef_position));
	loc.set_body_to_base_rotation (math::reframe<rigid_body::WorldSpace, rigid_body::BodySpace> (iss_ecef_rotation));
	body->set_location (loc);
	body->set_velocity_moments (VelocityMoments<rigid_body::WorldSpace> { iss_velocity, iss_angular_velocity });

	return body;
}


AutoTest t_1 ("rigid_body::System: 90-minute simulation of gravitational forces", []{
	auto rigid_body_system = rigid_body::System();
	auto rigid_body_solver = rigid_body::ImpulseSolver (rigid_body_system);
	auto const& iss = rigid_body_system.add (make_iss());
	auto const& earth = rigid_body_system.add_gravitating (rigid_body::make_earth());

	auto const real_time_limit = 10_s;
	auto const orbital_period = 92.28532_min;
	auto const interim_precision = 20_km;
	auto const final_precision = 50_m;

	SpaceLength<rigid_body::WorldSpace> const earth_initial_position { 0_m, 0_m, 0_m };
	SpaceLength<rigid_body::WorldSpace> const iss_position_1_of_4 { 0_m, 0_m, +kISSHeight };
	SpaceLength<rigid_body::WorldSpace> const iss_position_2_of_4 { -kISSHeight, 0_m, 0_m };
	SpaceLength<rigid_body::WorldSpace> const iss_position_3_of_4 { 0_m, 0_m, -kISSHeight };
	SpaceLength<rigid_body::WorldSpace> const iss_position_4_of_4 { +kISSHeight, 0_m, 0_m };

	auto simulation = Simulation (50_Hz, g_null_logger, [&] (si::Time const dt) { rigid_body_solver.evolve (dt); });

	simulation.evolve (orbital_period / 4, real_time_limit);

	test_asserts::verify_equal_with_epsilon ("ISS traveled 1/4 of distance", iss.location().position(), iss_position_1_of_4, interim_precision);
	test_asserts::verify_equal_with_epsilon ("Earth didn't travel much", earth.location().position(), earth_initial_position, 1_cm);

	simulation.evolve (orbital_period / 4, real_time_limit);

	test_asserts::verify_equal_with_epsilon ("ISS traveled 2/4 of distance", iss.location().position(), iss_position_2_of_4, interim_precision);
	test_asserts::verify_equal_with_epsilon ("Earth didn't travel much", earth.location().position(), earth_initial_position, 1_cm);

	simulation.evolve (orbital_period / 4, real_time_limit);

	test_asserts::verify_equal_with_epsilon ("ISS traveled 3/4 of distance", iss.location().position(), iss_position_3_of_4, interim_precision);
	test_asserts::verify_equal_with_epsilon ("Earth didn't travel much", earth.location().position(), earth_initial_position, 1_cm);

	simulation.evolve (orbital_period / 4, real_time_limit);

	test_asserts::verify_equal_with_epsilon ("ISS is back at its original position", iss.location().position(), iss_position_4_of_4, final_precision);
	test_asserts::verify_equal_with_epsilon ("Earth didn't travel much", earth.location().position(), earth_initial_position, 1_cm);
});

} // namespace
} // namespace xf::test

