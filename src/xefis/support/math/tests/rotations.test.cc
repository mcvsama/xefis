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
#include <xefis/support/math/rotations.h>
#include <xefis/support/math/quaternion_rotations.h>

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
											 rotation_angle (z_rotation_quaternion (+90_deg)), 90_deg, 1e-9_deg);
	test_asserts::verify_equal_with_epsilon ("Z axis == Z",
											 rotation_axis (z_rotation_quaternion (+90_deg)), vz / 1_m, 1e-9);
	test_asserts::verify_equal_with_epsilon ("rotation_vector (90_deg rotation) is π/2",
											 to_rotation_vector (z_rotation_quaternion (+90_deg)),
											 SpaceVector<si::Angle> { 0_rad, 0_rad, 0.5_rad * pi },
											 1e-9_rad);
	// TODO 270°
});


AutoTest t2 ("Math: rotations with Matrix", []{
	using std::numbers::pi;

	test_asserts::verify_equal_with_epsilon ("rotation_vector (90_deg rotation) is π/2",
											 to_rotation_vector (z_rotation_matrix (+90_deg)),
											 SpaceVector<si::Angle> { 0_rad, 0_rad, 0.5_rad * pi },
											 1e-9_rad);
	// TODO 270°
});


AutoTest t3 ("Math: rotations: Matrix-Quaternion conversions", []{
	// TODO
});


AutoTest t4 ("Math: random rotations fuzz", []{
	using std::numbers::pi;

	for (int i = 0; i < 1000; ++i)
	{
		auto const vec = normalized (SpaceLength { 1_m * rand(), 1_m * rand(), 1_m * rand() });
		auto const angle = neutrino::renormalize (rand(), Range<double> (0, RAND_MAX), Range<si::Angle> (-2_rad * pi, +2_rad * pi));
		auto const axis = normalized (SpaceVector { 1.0 * rand(), 1.0 * rand(), 1.0 * rand() });
		auto const rotation = quaternion_rotation_about (axis, angle);
		auto const rotation_m = RotationMatrix (rotation);
		auto const rotation_vector = to_rotation_vector (rotation);
		auto const rotation_vector_m = to_rotation_vector (rotation_m);

		test_asserts::verify_equal_with_epsilon ("~rotation * (rotation * vec) == vec",
												 ~rotation * (rotation * vec), vec, 1e-9_m);
		test_asserts::verify_equal_with_epsilon ("inv (rotation) * (rotation * vec) == vec",
												 math::inv (rotation) * (rotation * vec), vec, 1e-9_m);
		test_asserts::verify_equal_with_epsilon ("-rotation rotates the same as +rotation",
												 -rotation * vec, +rotation * vec, 1e-9_m);
		test_asserts::verify_equal_with_epsilon ("to_rotation_quaternion (to_rotation_vector (q)) == q",
												 to_rotation_quaternion (to_rotation_vector (rotation)), rotation, 1e-9);
		test_asserts::verify_equal_with_epsilon ("to_rotation_vector (Quaterion) == to_rotation_vector (Matrix)",
												 rotation_vector, rotation_vector_m, 1e-9_rad);

		if (test_asserts::equal_with_epsilon (rotation_axis (rotation), axis, 1e-9))
		{
			test_asserts::verify_equal_with_epsilon ("rotation_angle() is correct",
													 rotation_angle (rotation), angle, 1e-9_rad);
		}
		else
		{
			test_asserts::verify_equal_with_epsilon ("rotation_axis() is correct",
													 rotation_axis (rotation), -axis, 1e-9);
			test_asserts::verify_equal_with_epsilon ("rotation_angle() is correct",
													 rotation_angle (rotation), -angle, 1e-9_rad);
		}
	}
});

} // namespace
} // namespace xf::test

