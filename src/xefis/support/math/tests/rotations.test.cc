/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
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
#include <xefis/support/math/geometry.h>
#include <xefis/support/math/placement.h>
#include <xefis/support/math/matrix_rotations.h>
#include <xefis/support/math/quaternion_rotations.h>
#include <xefis/support/math/rotations.h>
#include <xefis/support/simulation/rigid_body/concepts.h>

// Neutrino:
#include <neutrino/test/auto_test.h>
#include <neutrino/si/si.h>
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>
#include <numbers>


namespace xf::test {
namespace {

using namespace neutrino::si;


template<math::CoordinateSystem TS = void, math::CoordinateSystem SS = void>
	auto
	random_quaternion_rotation()
	{
		using std::numbers::pi;

		auto const angle = neutrino::renormalize (rand(), Range<double> (0, RAND_MAX), Range<si::Angle> (-2_rad * pi, +2_rad * pi));
		auto const axis = SpaceVector<double, TS> { 1.0 * rand(), 1.0 * rand(), 1.0 * rand() }.normalized();
		auto const q = quaternion_rotation_about<TS, SS> (axis, angle);

		test_asserts::verify_equal_with_epsilon ("rotation quaternion is normalized", q.norm(), 1.0, 1e-9);

		return q;
	};


template<math::Scalar S = double, math::CoordinateSystem Space = void>
	auto
	random_vector()
	{
		return SpaceVector<S, Space> { S (rand()), S (rand()), S (rand()) };
	}


AutoTest t1 ("Math: rotations with Quaternion", []{
	using std::numbers::pi;

	auto const vx = SpaceLength { 1_m, 0_m, 0_m };
	auto const vy = SpaceLength { 0_m, 1_m, 0_m };
	auto const vz = SpaceLength { 0_m, 0_m, 1_m };

	test_asserts::verify_equal_with_epsilon ("x → y",
											 z_rotation_quaternion (+90_deg) * vx, vy, 1e-9_m);
	test_asserts::verify_equal_with_epsilon ("y → x",
											 z_rotation_quaternion (-90_deg) * vy, vx, 1e-9_m);
	test_asserts::verify_equal_with_epsilon ("x → y → x",
											 z_rotation_quaternion (+90_deg) * z_rotation_quaternion (-90_deg) * vx, vx, 1e-9_m);
	test_asserts::verify_equal_with_epsilon ("x → z",
											 y_rotation_quaternion (-90_deg) * vx, vz, 1e-9_m);
	test_asserts::verify_equal_with_epsilon ("z → x",
											 y_rotation_quaternion (+90_deg) * vz, vx, 1e-9_m);
	test_asserts::verify_equal_with_epsilon ("Z angle == 90_deg",
											 angle (z_rotation_quaternion (+90_deg)), 90_deg, 1e-9_deg);
	test_asserts::verify_equal_with_epsilon ("Z axis == Z",
											 normalized_axis (z_rotation_quaternion (+90_deg)), vz / 1_m, 1e-9);
	test_asserts::verify_equal_with_epsilon ("rotation_vector (90_deg rotation) is π/2",
											 to_rotation_vector (z_rotation_quaternion (+90_deg)),
											 SpaceVector<si::Angle> { 0_rad, 0_rad, 0.5_rad * pi },
											 1e-9_rad);
});


AutoTest t2 ("Math: rotations with Quaternion (angle range π…2π)", []{
	using std::numbers::pi;

	for (auto const a: { -0.9_rad * pi, +0.9_rad * pi })
	{
		auto const x = SpaceVector { 1.0 * rand(), 1.0 * rand(), 1.0 * rand() }.normalized();
		auto const q = quaternion_rotation_about (x, a);

		test_asserts::verify_equal_with_epsilon (std::format ("{} quaternion returns correct angle and always positive", a),
												 angle (q), abs (a), 1e-9_rad);
	}
});


AutoTest t3 ("Math: composing quaternion rotations", []{
	for (int i = 0; i < 1000; ++i)
	{
		auto const vec = random_vector();
		auto const q1 = random_quaternion_rotation();
		auto const q2 = random_quaternion_rotation();
		auto const q3 = random_quaternion_rotation();
		auto const epsilon = abs (vec) * 1e-14;

		test_asserts::verify_equal_with_epsilon ("(q1 * q2) * vec == q1 * (q2 * vec)",
												 (q1 * q2) * vec,
												 q1 * (q2 * vec),
												 epsilon);
		test_asserts::verify_equal_with_epsilon ("(q1 * ~q2) * vec == q1 * (~q2 * vec)",
												 (q1 * ~q2) * vec,
												 q1 * (~q2 * vec),
												 epsilon);
		test_asserts::verify_equal_with_epsilon ("((q1 * q2) * q3) * vec == (q1 * (q2 * q3)) * vec",
												 ((q1 * q2) * q3) * vec,
												 (q1 * (q2 * q3)) * vec,
												 epsilon);
		test_asserts::verify_equal_with_epsilon ("to_rotation_vector ((q1 * q2) * q3) == to_rotation_vector (q1 * (q2 * q3))",
												 to_rotation_vector ((q1 * q2) * q3),
												 to_rotation_vector (q1 * (q2 * q3)),
												 1_rad * epsilon);
	}
});


AutoTest t4 ("Math: Quaternion-Matrix compatibility", []{
	using std::numbers::pi;

	test_asserts::verify_equal_with_epsilon ("rotation_vector (90_deg rotation) is π/2",
											 to_rotation_vector (z_rotation_matrix (+90_deg)),
											 SpaceVector<si::Angle> { 0_rad, 0_rad, 0.5_rad * pi },
											 1e-9_rad);

	for (int i = 0; i < 1000; ++i)
	{
		auto const vec = random_vector();
		auto const q1 = random_quaternion_rotation();
		auto const q2 = random_quaternion_rotation();
		auto const q3 = random_quaternion_rotation();
		auto const m1 = RotationMatrix (q1);
		auto const m2 = RotationMatrix (q2);
		auto const m3 = RotationMatrix (q3);
		auto const epsilon = abs (vec) * 1e-14;

		test_asserts::verify_equal_with_epsilon ("(1)",
												 (q1 * m2) * vec,
												 q1 * (m2 * vec),
												 epsilon);
		test_asserts::verify_equal_with_epsilon ("(2)",
												 (q1 * q2) * vec,
												 m1 * (q2 * vec),
												 epsilon);
		test_asserts::verify_equal_with_epsilon ("(3)",
												 ((q1 * q2) * q3) * vec,
												 m1 * ((q2 * q3) * vec),
												 epsilon);
		test_asserts::verify_equal_with_epsilon ("(4)",
												 ((q1 * q2) * q3) * vec,
												 (m1 * (m2 * m3)) * vec,
												 epsilon);
		test_asserts::verify_equal_with_epsilon ("(5)",
												 to_rotation_vector ((q1 * q2) * m3),
												 to_rotation_vector (q1 * (q2 * m3)),
												 1_rad * epsilon);
		test_asserts::verify_equal_with_epsilon ("(6)",
												 to_rotation_vector ((q1 * q2) * q3),
												 to_rotation_vector (m1 * (m2 * m3)),
												 1_rad * epsilon);
	}
});


AutoTest t5 ("Math: random rotations fuzz", []{
	using std::numbers::pi;

	for (int i = 0; i < 1000; ++i)
	{
		auto const vec = random_vector();
		auto const a = neutrino::renormalize (rand(), Range<double> (0, RAND_MAX), Range<si::Angle> (-1_rad * pi, +1_rad * pi));
		auto const x = SpaceVector { 1.0 * rand(), 1.0 * rand(), 1.0 * rand() }.normalized();
		auto const q_rotation = quaternion_rotation_about (x, a);
		auto const m_rotation = matrix_rotation_about (x, a);
		auto const q_rotation_vector = to_rotation_vector (q_rotation);
		auto const m_rotation_vector = to_rotation_vector (m_rotation);
		// Since length of some random vectors can be very high, the required epsilon must be adjusted:
		auto const epsilon = abs (vec) * 1e-14;

		test_asserts::verify_equal_with_epsilon ("~q_rotation * (q_rotation * vec) == vec",
												 ~q_rotation * (q_rotation * vec), vec, epsilon);
		test_asserts::verify_equal_with_epsilon ("inv (q_rotation) * (q_rotation * vec) == vec",
												 math::inv (q_rotation) * (q_rotation * vec), vec, epsilon);
		test_asserts::verify_equal_with_epsilon ("-q_rotation rotates the same as +q_rotation",
												 -q_rotation * vec, +q_rotation * vec, epsilon);

		// Make sure either Qa = Qb or Qa == -Qb, since they're equivalent:
		if (!test_asserts::equal_with_epsilon (to_rotation_quaternion (to_rotation_vector (q_rotation)), q_rotation, epsilon))
		{
			test_asserts::verify_equal_with_epsilon ("to_rotation_quaternion (to_rotation_vector (q)) == q",
													 to_rotation_quaternion (to_rotation_vector (q_rotation)), -q_rotation, epsilon);
		}

		test_asserts::verify_equal_with_epsilon ("to_rotation_vector (Quaterion) == to_rotation_vector (Matrix)",
												 q_rotation_vector, m_rotation_vector, si::Angle (epsilon));

		// The axis must be the same or negated, but if it's negated then the angle must be (2 * pi - angle):
		if (test_asserts::equal_with_epsilon (normalized_axis (q_rotation), x, epsilon))
		{
			test_asserts::verify_equal_with_epsilon ("normalized_axis() is correct",
													 normalized_axis (q_rotation), x, epsilon);
			test_asserts::verify_equal_with_epsilon ("angle() is correct",
													 angle (q_rotation), a, si::Angle (epsilon));
		}
		else
		{
			test_asserts::verify_equal_with_epsilon ("normalized_axis() is correct (negated quaternion)",
													 normalized_axis (q_rotation), -x, epsilon);

			// angle() always returns positive values, so be prepared for it:
			if (a > 0_rad)
			{
				test_asserts::verify_equal_with_epsilon ("angle() is correct (negated quaternion, positive original angle)",
														 angle (q_rotation), 2_rad * pi - a, si::Angle (epsilon));
			}
			else
			{
				test_asserts::verify_equal_with_epsilon ("angle() is correct (negated quaternion, negative original angle)",
														 angle (q_rotation), -a, si::Angle (epsilon));
			}
		}
	}
});


AutoTest t6 ("Math: fixed orientation helper rotations", []{
	for (int i = 0; i < 100; ++i)
	{
		auto const position = SpaceLength<WorldSpace> { 1_m, 1_m, 1_m };
		auto const placement_1 = Placement<WorldSpace, BodyCOM> (position, x_rotation<BodyCOM, WorldSpace> (0_deg));
		auto placement_2 = Placement<WorldSpace, BodyCOM> (position, x_rotation<BodyCOM, WorldSpace> (90_deg));

		auto const q_initial_relative_rotation = relative_rotation (placement_1, placement_2);
		auto const m_initial_relative_rotation = RotationMatrix<BodyCOM, BodyCOM> (q_initial_relative_rotation);

		for (int j = 0; j < 100; ++j)
		{
			placement_2.rotate_body_frame (random_quaternion_rotation<WorldSpace, WorldSpace>());

			RotationMatrix<BodyCOM, BodyCOM> const m_current_relative_rotation = RotationMatrix (relative_rotation (placement_1, placement_2));
			auto const m_angle = angle (m_current_relative_rotation);
			auto const m_axis = normalized_axis (m_current_relative_rotation);

			RotationQuaternion<BodyCOM, BodyCOM> const q_current_relative_rotation = relative_rotation (placement_1, placement_2);
			auto const q_angle = angle (q_current_relative_rotation);
			auto const q_axis = normalized_axis (q_current_relative_rotation);

			test_asserts::verify_equal_with_epsilon ("Matrix vs Quaternion: rotation angles are the same",
													 m_angle, q_angle, 1e-9_rad);
			test_asserts::verify_equal_with_epsilon ("Matrix vs Quaternion: rotation axes are the same",
													 m_axis, q_axis, 1e-9);

			for (auto k = 0; k < 10; ++k)
			{
				auto vec = random_vector<si::Length, BodyCOM>().normalized();
				test_asserts::verify_equal_with_epsilon ("Matrix vs Quaternion: current relative rotations are the same",
														 m_current_relative_rotation * vec, q_current_relative_rotation * vec, 1e-9_m);
			}

			RotationMatrix<BodyCOM, BodyCOM> const m_body_error = ~m_initial_relative_rotation * m_current_relative_rotation;
			RotationQuaternion<BodyCOM, BodyCOM> const q_body_error = ~q_initial_relative_rotation * q_current_relative_rotation;

			SpaceVector<si::Angle, WorldSpace> const m_world_error = placement_2.body_to_base_rotation() * to_rotation_vector (m_body_error);
			SpaceVector<si::Angle, WorldSpace> const q_world_error = placement_2.body_to_base_rotation() * to_rotation_vector (q_body_error);

			test_asserts::verify_equal_with_epsilon ("Matrix relative rotations == Quaternion relative rotations",
													 m_world_error, q_world_error, 1e-9_rad);
		}
	}
});

} // namespace
} // namespace xf::test

