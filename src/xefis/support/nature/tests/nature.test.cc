/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
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
#include <xefis/support/nature/mass_moments.h>
#include <xefis/support/nature/velocity_moments.h>
#include <xefis/support/nature/wrench.h>

// Neutrino:
#include <neutrino/test/auto_test.h>

// Standard:
#include <cstddef>
#include <memory>
#include <string>


namespace xf::test {
namespace {

AutoTest t1 ("Nature: MassMoments calculations", []{
	{
		SpaceMatrix<double> const unit { math::unit };
		SpaceMatrix<si::MomentOfInertia> const moi2 {
			2_kgm2, 0_kgm2, 0_kgm2,
			0_kgm2, 2_kgm2, 0_kgm2,
			0_kgm2, 0_kgm2, 2_kgm2,
		};
		MassMoments<> const m1 (1_kg, { 0_m, 0_m, 0_m }, math::unit);
		MassMoments<> const m2 (1_kg, { 0_m, 0_m, 0_m }, math::unit);

		auto const m3 = m1 + m2;

		test_asserts::verify_equal_with_epsilon ("(0.0) mass summed correctly", m3.mass(), 2_kg, 1e-9_kg);
		test_asserts::verify_equal_with_epsilon ("(0.1) center of mass summed correctly",
												 m3.center_of_mass_position(),
												 SpaceLength<> { 0_m, 0_m, 0_m },
												 1e-9_m);
		test_asserts::verify_equal_with_epsilon ("(0.2) moment of inertia summed correctly",
												 m3.moment_of_inertia(),
												 moi2,
												 1e-9_kgm2);

		auto const m4 = unit * m1;

		test_asserts::verify_equal_with_epsilon ("(1.0) mass is unchanged", m1.mass(), m4.mass(), 1e-12_kg);
		test_asserts::verify_equal_with_epsilon ("(1.1) center of mass is unchanged",
												 m4.center_of_mass_position(),
												 SpaceLength<> { 0_m, 0_m, 0_m },
												 1e-12_m);
		test_asserts::verify_equal_with_epsilon ("(1.2) moment of inertia is unchanged",
												 m1.moment_of_inertia(),
												 m4.moment_of_inertia(),
												 1e-12_kgm2);

		// TODO m5 = z_rotation (90°)
	}

	{
		MassMoments<> const m1 (9_kg, { -2_m, 0_m, 0_m }, math::unit);
		MassMoments<> const m2 (1_kg, { +7_m, 0_m, 0_m }, math::unit);

		auto const m3 = m1 + m2;

		test_asserts::verify_equal_with_epsilon ("(2.0) mass summed correctly", m3.mass(), 10_kg, 1e-15_kg);
		test_asserts::verify_equal_with_epsilon ("(2.1) center of mass summed correctly",
												 m3.center_of_mass_position(),
												 SpaceLength<> { -1.1_m, 0_m, 0_m },
												 1e-9_m);
		// TODO verify moment of inertia at the new resulting COM
	}

	// MassMoments with offset:
	{
		MassMoments<> const m1 (9_kg, { -2_m, 0_m, 0_m }, math::unit);
		MassMoments<> const m2 (1_kg, { +7_m, 0_m, 0_m }, math::unit);
		SpaceVector<si::Length> vec { 1_m, 0_m, 0_m };

		auto const m3 = (m1 + vec) + (m2 + vec);

		test_asserts::verify_equal_with_epsilon ("(3.0) mass summed correctly", m3.mass(), 10_kg, 1e-15_kg);
		test_asserts::verify_equal_with_epsilon ("(3.1) center of mass summed correctly",
												 m3.center_of_mass_position(),
												 SpaceLength<> { -1.1_m, 0_m, 0_m } + vec,
												 1e-9_m);
		// TODO verify moment of inertia at the new resulting COM
	}

	// MassMoments with offset:
	{
		MassMoments<> const m1 (9_kg, { 0_m, 0_m, 0_m }, math::unit);
		MassMoments<> const m2 (1_kg, { 0_m, 0_m, 0_m }, math::unit);
		SpaceVector<si::Length> o1 { -2_m, 0_m, 0_m };
		SpaceVector<si::Length> o2 { +7_m, 0_m, 0_m };

		auto const m3 = (m1 + o1) + (m2 + o2);

		test_asserts::verify_equal_with_epsilon ("(4.0) mass summed correctly", m3.mass(), 10_kg, 1e-15_kg);
		test_asserts::verify_equal_with_epsilon ("(4.1) center of mass summed correctly",
												 m3.center_of_mass_position(),
												 SpaceLength<> { -1.1_m, 0_m, 0_m },
												 1e-9_m);
		// TODO verify moment of inertia at the new resulting COM
	}

	// Apply rotation to MassMoments:
	{
		// TODO
	}
});


AutoTest t2 ("Nature: VelocityMoments calculations", []{
	test_asserts::verify_equal_with_epsilon ("(0) velocities are added correctly",
											 add (VelocityMoments<> ({ -1_mps, 0_mps, 0_mps }, { 0_radps, 0_radps, 1_radps }),
												  VelocityMoments<> ({ +1_mps, 0_mps, 0_mps }, { 0_radps, 0_radps, 0_radps }),
												  SpaceLength<> { 1_m, 0_m, 0_m }).velocity(),
											 SpaceVector<si::Velocity> { 0_mps, 1_mps, 0_mps },
											 1e-12_mps);
	test_asserts::verify_equal_with_epsilon ("(1) velocities are added correctly",
											 add (VelocityMoments<> ({ -1_mps, 0_mps, 0_mps }, { 0_radps, 0_radps, 2_radps }),
												  VelocityMoments<> ({ +1_mps, 0_mps, 0_mps }, { 0_radps, 0_radps, 0_radps }),
												  SpaceLength<> { 1_m, 0_m, 0_m }).velocity(),
											 SpaceVector<si::Velocity> { 0_mps, 2_mps, 0_mps },
											 1e-12_mps);
	test_asserts::verify_equal_with_epsilon ("(2) velocities are added correctly",
											 add (VelocityMoments<> ({ -1_mps, 0_mps, 0_mps }, { 0_radps, 0_radps, 1_radps }),
												  VelocityMoments<> ({ +1_mps, 0_mps, 0_mps }, { 0_radps, 0_radps, 1_radps }),
												  SpaceLength<> { 1_m, 0_m, 0_m }).velocity(),
											 SpaceVector<si::Velocity> { 0_mps, 1_mps, 0_mps },
											 1e-12_mps);
	test_asserts::verify_equal_with_epsilon ("(3) velocities are added correctly",
											 add (VelocityMoments<> ({ -1_mps, 0_mps, 0_mps }, { 0_radps, 0_radps, 1_radps }),
												  VelocityMoments<> ({ +1_mps, -1_mps, 0_mps }, { 0_radps, 0_radps, 0_radps }),
												  SpaceLength<> { 1_m, 0_m, 0_m }).velocity(),
											 SpaceVector<si::Velocity> { 0_mps, 0_mps, 0_mps },
											 1e-12_mps);
});


AutoTest t3 ("Nature: Wrench: resultant_force()", []{
	Wrench<> w1 ({ 0_N, 1_N, 0_N }, { 0_Nm, 0_Nm, 0_Nm }, { 1_m, 0_m, 0_m });
	Wrench<> w2 ({ 0_N, 0_N, 0_N }, { 0_Nm, 0_Nm, 1_Nm }, { 2_m, 0_m, 0_m });
	Wrench<> w3 ({ 0_N, 2_N, 0_N }, { 0_Nm, 0_Nm, 1_Nm }, { 2_m, 0_m, 0_m });

	auto const rw1 = resultant_force (w1);
	auto const rw2 = resultant_force (w2);
	auto const rw3 = resultant_force (w3);

	test_asserts::verify_equal_with_epsilon ("(rw1 F)", rw1.force(), SpaceForce<> { 0_N, 1_N, 0_N }, 1e-6_N);
	test_asserts::verify_equal_with_epsilon ("(rw1 M)", rw1.torque(), SpaceTorque<> { 0_Nm, 0_Nm, 1_Nm }, 1e-6_Nm);

	test_asserts::verify_equal_with_epsilon ("(rw2 F)", rw2.force(), SpaceForce<> { 0_N, 0_N, 0_N }, 1e-6_N);
	test_asserts::verify_equal_with_epsilon ("(rw2 M)", rw2.torque(), SpaceTorque<> { 0_Nm, 0_Nm, 1_Nm }, 1e-6_Nm);

	test_asserts::verify_equal_with_epsilon ("(rw3 F)", rw3.force(), SpaceForce<> { 0_N, 2_N, 0_N }, 1e-6_N);
	test_asserts::verify_equal_with_epsilon ("(rw3 M)", rw3.torque(), SpaceTorque<> { 0_Nm, 0_Nm, 5_Nm }, 1e-6_Nm);
});


AutoTest t4 ("Nature: Wrench + offset", []{
	Wrench<> w1 ({ 0_N, 1_N, 0_N }, { 0_Nm, 0_Nm, 0_Nm }, { 1_m, 0_m, 0_m });
	Wrench<> w2 ({ 0_N, 0_N, 0_N }, { 0_Nm, 0_Nm, 1_Nm }, { 2_m, 0_m, 0_m });
	Wrench<> w3 ({ 0_N, 2_N, 0_N }, { 0_Nm, 0_Nm, 1_Nm }, { 2_m, 0_m, 0_m });

	auto const o1 = w1 + SpaceLength<> { 1_m, 0_m, 0_m };
	auto const o2 = w2 + SpaceLength<> { 1_m, 0_m, 0_m };
	auto const o3 = w3 + SpaceLength<> { 1_m, 0_m, 0_m };

	auto const rw1 = resultant_force (w1);
	auto const rw2 = resultant_force (w2);
	auto const rw3 = resultant_force (w3);

	auto const ro1 = resultant_force (o1);
	auto const ro2 = resultant_force (o2);
	auto const ro3 = resultant_force (o3);

	test_asserts::verify_equal_with_epsilon ("(rw1 F)", rw1.force(), SpaceForce<> { 0_N, 1_N, 0_N }, 1e-6_N);
	test_asserts::verify_equal_with_epsilon ("(rw1 M)", rw1.torque(), SpaceTorque<> { 0_Nm, 0_Nm, 1_Nm }, 1e-6_Nm);
	test_asserts::verify_equal_with_epsilon ("(ro1 F)", ro1.force(), SpaceForce<> { 0_N, 1_N, 0_N }, 1e-6_N);
	test_asserts::verify_equal_with_epsilon ("(ro1 M)", ro1.torque(), SpaceTorque<> { 0_Nm, 0_Nm, 2_Nm }, 1e-6_Nm);

	test_asserts::verify_equal_with_epsilon ("(rw2 F)", rw2.force(), SpaceForce<> { 0_N, 0_N, 0_N }, 1e-6_N);
	test_asserts::verify_equal_with_epsilon ("(rw2 M)", rw2.torque(), SpaceTorque<> { 0_Nm, 0_Nm, 1_Nm }, 1e-6_Nm);
	test_asserts::verify_equal_with_epsilon ("(ro2 F)", ro2.force(), SpaceForce<> { 0_N, 0_N, 0_N }, 1e-6_N);
	test_asserts::verify_equal_with_epsilon ("(ro2 M)", ro2.torque(), SpaceTorque<> { 0_Nm, 0_Nm, 1_Nm }, 1e-6_Nm);

	test_asserts::verify_equal_with_epsilon ("(rw3 F)", rw3.force(), SpaceForce<> { 0_N, 2_N, 0_N }, 1e-6_N);
	test_asserts::verify_equal_with_epsilon ("(rw3 M)", rw3.torque(), SpaceTorque<> { 0_Nm, 0_Nm, 5_Nm }, 1e-6_Nm);
	test_asserts::verify_equal_with_epsilon ("(ro3 F)", ro3.force(), SpaceForce<> { 0_N, 2_N, 0_N }, 1e-6_N);
	test_asserts::verify_equal_with_epsilon ("(ro3 M)", ro3.torque(), SpaceTorque<> { 0_Nm, 0_Nm, 7_Nm }, 1e-6_Nm);
});

} // namespace
} // namespace xf::test

