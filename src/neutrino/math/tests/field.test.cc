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

// Standard:
#include <cstddef>

// Neutrino:
#include <neutrino/math/math.h>
#include <neutrino/math/field.h>
#include <neutrino/test/test.h>


namespace neutrino::test {
namespace {

RuntimeTest t1 ("Field<1 argument, 1 value>", []{
	Field<double, double> field {
		{ 0.0, 0.0 },
		{ 1.0, 10.0 },
		{ 0.0, 0.0 },
		{ 2.0, 20.0 },
		{ 3.0, 10.0 },
		{ 4.0, 0.0 },
	};

	test_asserts::verify ("domain().min() is correct", field.domain().min() == 0.0);
	test_asserts::verify ("domain().max() is correct", field.domain().max() == 4.0);

	test_asserts::verify ("codomain().min() is correct", field.codomain().min() == 0.0);
	test_asserts::verify ("codomain().max() is correct", field.codomain().max() == 20.0);

	// TODO test_asserts::verify_equal_with_epsilon ("average ({ -2.0, 2.0 }) is correct", field.average ({ -2.0, 2.0 }), 0.0, 0.001);
});


RuntimeTest t2 ("Field<2 arguments, 1 value>", []{
	Field<double, si::Angle, si::Time> field {
		{
			0.0,
			{
				{ 0.0_deg, 0.0_s },
				{ 1.0_deg, 10.0_s },
				{ 2.0_deg, 20.0_s },
				{ 3.0_deg, 30.0_s },
				// 4.0_deg would extrapolate to 30.0_s.
			},
		},
		{
			1.0,
			{
				// 0.0_deg would extrapolate to 100.0_s.
				{ 1.0_deg, 100.0_s },
				{ 2.0_deg, 200.0_s },
				{ 3.0_deg, 300.0_s },
				{ 4.0_deg, 400.0_s },
			},
		}
	};

	test_asserts::verify_equal_with_epsilon ("min_argument() == 0.0", field.min_argument(), 0.0, 0.000001);
	test_asserts::verify_equal_with_epsilon ("max_argument() == 1.0", field.max_argument(), 1.0, 0.000001);

	test_asserts::verify ("min_argument (-1.0) == null", !field.min_argument (-1.0));
	test_asserts::verify ("max_argument (-1.0) == null", !field.max_argument (-1.0));

	test_asserts::verify_equal_with_epsilon ("min_argument (0.0) == 0°", field.min_argument (0.0).value_or (-1_deg), 0_deg, 0.000001_deg);
	test_asserts::verify_equal_with_epsilon ("max_argument (0.0) == 3°", field.max_argument (0.0).value_or (-1_deg), 3_deg, 0.000001_deg);

	test_asserts::verify_equal_with_epsilon ("min_argument (1.0) == 1°", field.min_argument (1.0).value_or (-1_deg), 1_deg, 0.000001_deg);
	test_asserts::verify_equal_with_epsilon ("max_argument (1.0) == 4°", field.max_argument (1.0).value_or (-1_deg), 4_deg, 0.000001_deg);

	test_asserts::verify ("returned value for outside of domain is std::nullopt", !field.value (100.0, 0.0_deg));

	test_asserts::verify_equal_with_epsilon ("value (0.0, 1.0°) == 0 s", field.value (0.0, 1.0_deg).value_or (-1.0_s), 10_s, 0.000001_s);
	test_asserts::verify_equal_with_epsilon ("value (0.0, 3.0°) == 30 s", field.value (0.0, 3.0_deg).value_or (-1.0_s), 30_s, 0.000001_s);

	test_asserts::verify_equal_with_epsilon ("value (1.0, 1.0°) == 100 s", field.value (1.0, 1.0_deg).value_or (-1.0_s), 100_s, 0.000001_s);
	test_asserts::verify_equal_with_epsilon ("value (1.0, 3.0°) == 300 s", field.value (1.0, 3.0_deg).value_or (-1.0_s), 300_s, 0.000001_s);

	test_asserts::verify_equal_with_epsilon ("extrapolated_value (0.0, 0°) == 0 s", field.extrapolated_value (0.0, 0.0_deg), 0_s, 0.000001_s);
	test_asserts::verify_equal_with_epsilon ("extrapolated_value (1.0, 0°) == 100 s", field.extrapolated_value (1.0, 0.0_deg), 100_s, 0.000001_s);

	test_asserts::verify_equal_with_epsilon ("extrapolated_value (0.5, 0.0°) == 50 s", field.extrapolated_value (0.5, 0.0_deg), 50_s, 0.000001_s);
	test_asserts::verify_equal_with_epsilon ("extrapolated_value (0.5, 2.0°) == 110 s", field.extrapolated_value (0.5, 2.0_deg), 110_s, 0.000001_s);
	test_asserts::verify_equal_with_epsilon ("extrapolated_value (0.5, 4.0°) == 215 s", field.extrapolated_value (0.5, 4.0_deg), 215_s, 0.000001_s);

	// 2D search:
	test_asserts::verify_equal_with_epsilon ("min_value() == 0_s", field.min_value(), 0.0_s, 0.000001_s);

	// 1D search:
	test_asserts::verify_equal_with_epsilon ("min_value (0.0) == 10_s", field.min_value (0.0).value_or (-1_s), 10_s, 0.000001_s); // TODO ensure arguments are {0.0, 1.0_deg}
	test_asserts::verify_equal_with_epsilon ("min_value (0.5) == 50_s", field.min_value (0.5).value_or (-1_s), 55_s, 0.000001_s);
	test_asserts::verify_equal_with_epsilon ("min_value (1.0) == 100_s", field.min_value (1.0).value_or (-1_s), 100_s, 0.000001_s);

	// Test if arguments are properly renormalized:
	{
		auto const point = field.min_value_point (0.5);
		test_asserts::verify ("value is inside domain", !!point);
		test_asserts::verify_equal_with_epsilon ("min_value_point (0.5) is at argument<0> 0.5", std::get<0> (point->arguments), 0.5, 0.000001);
		test_asserts::verify_equal_with_epsilon ("min_value_point (0.5) is at argument<1> 1_deg", std::get<1> (point->arguments), 1_deg, 0.000001_deg);
	}
});


RuntimeTest t3 ("Field<3 arguments, 1 value>", []{
	Field<double, si::Angle, si::Time, si::Length> field {
		{
			0.0,
			{
				{
					0.0_deg,
					{
						{ 1.0_s, 10.0_m },
						{ 2.0_s, 20.0_m },
						{ 3.0_s, 30.0_m },
					},
				},
				{
					10.0_deg,
					{
						{ 1.0_s, -10.0_m },
						{ 2.0_s, -20.0_m },
						{ 3.0_s, -30.0_m },
					},
				},
			},
		},
		{
			1.0,
			{
				{
					0.0_deg,
					{
						{ 2.0_s, 200.0_m },
						{ 3.0_s, 300.0_m },
						{ 4.0_s, 400.0_m },
					},
				},
				{
					10.0_deg,
					{
						{ 2.0_s, -200.0_m },
						{ 3.0_s, -300.0_m },
						{ 4.0_s, -400.0_m },
					},
				},
			},
		},
		{
			2.0,
			{
				{
					0.0_deg,
					{
						{ 0.0_s, -100.0_m },
						{ 1.0_s, -70.0_m },
						{ 2.0_s, 0.0_m },
					},
				},
				{
					10.0_deg,
					{
						{ 0.0_s, 0.0_m },
						{ 1.0_s, -70.0_m },
						{ 2.0_s, -100.0_m },
					},
				},
			},
		},
	};

	// 3D search:
	test_asserts::verify_equal_with_epsilon ("min_value() == -400_m", field.min_value(), -400_m, 0.000001_m);

	// 2D search:
	test_asserts::verify_equal_with_epsilon ("min_value (0.0) == -30_m", field.min_value (0.0).value_or (-1_m), -30_m, 0.000001_m);
	test_asserts::verify_equal_with_epsilon ("min_value (0.5) == -215_m", field.min_value (0.5).value_or (-1_m), -215_m, 0.000001_m);

	// 1D search:
	test_asserts::verify_equal_with_epsilon ("min_value (0.0, 0°) == 10_m", field.min_value (0.0, 0_deg).value_or (-1_m), 10_m, 0.000001_m);
	test_asserts::verify_equal_with_epsilon ("min_value (0.0, 5°) == 0_m", field.min_value (0.0, 5_deg).value_or (-1_m), 0_m, 0.000001_m);
	test_asserts::verify_equal_with_epsilon ("min_value (0.0, 10°) == -30_m", field.min_value (0.0, 10_deg).value_or (-1_m), -30_m, 0.000001_m);
	test_asserts::verify_equal_with_epsilon ("min_value (0.5, 0°) == 105_m", field.min_value (0.5, 0_deg).value_or (-1_m), 105_m, 0.000001_m);
	test_asserts::verify_equal_with_epsilon ("min_value (1.0, 0°) == 200_m", field.min_value (1.0, 0_deg).value_or (-1_m), 200_m, 0.000001_m);
	test_asserts::verify_equal_with_epsilon ("min_value (1.0, 10°) == -400_m", field.min_value (1.0, 10_deg).value_or (-1_m), -400_m, 0.000001_m);
	test_asserts::verify_equal_with_epsilon ("min_value (2.0, 0°) == -100_m", field.min_value (2.0, 0_deg).value_or (-1_m), -100_m, 0.000001_m);
	test_asserts::verify_equal_with_epsilon ("min_value (2.0, 5°) == -70_m", field.min_value (2.0, 5_deg).value_or (-1_m), -70_m, 0.000001_m);
	test_asserts::verify_equal_with_epsilon ("min_value (2.0, 10°) == -100_m", field.min_value (2.0, 10_deg).value_or (-1_m), -100_m, 0.000001_m);
});


RuntimeTest t3prim ("Field<N arguments..., 1 value>", []{
	Field<double, si::Angle, si::Time, si::Length, si::Mass, si::Amount> field {
		{ 0.5, { { 1_rad, { { 1_s, { { 1_m, { { 1_kg, 1_mol } } } } } } } } },
	};

	test_asserts::verify_equal_with_epsilon ("min_value() is 1_mol", field.min_value(), 1_mol, 0.000001_mol);
	test_asserts::verify_equal_with_epsilon ("min_value (0.5) is 1_mol", field.min_value (0.5).value_or (-1_mol), 1_mol, 0.000001_mol);
	test_asserts::verify_equal_with_epsilon ("min_value (0.5, 1_rad) is 1_mol", field.min_value (0.5, 1_rad).value_or (-1_mol), 1_mol, 0.000001_mol);
	test_asserts::verify_equal_with_epsilon ("min_value (0.5, 1_rad, 1_s) is 1_mol", field.min_value (0.5, 1_rad, 1_s).value_or (-1_mol), 1_mol, 0.000001_mol);
	test_asserts::verify_equal_with_epsilon ("min_value (0.5, 1_rad, 1_s, 1_m) is 1_mol", field.min_value (0.5, 1_rad, 1_s, 1_m).value_or (-1_mol), 1_mol, 0.000001_mol);
});


RuntimeTest t4 ("Field<..., math::Vector<...>>", []{
	using Vector = math::Vector<double, 3>;

	Field<double, Vector> field {
		{
			0.0,
			Vector { 1.0, 0.0, 0.0 },
		},
		{
			1.0,
			Vector { 0.0, 1.0, 0.0 },
		},
	};

	auto const epsilon = 0.000001 * Vector { 1.0, 1.0, 1.0 };

	test_asserts::verify_equal_with_epsilon ("field.value (0.5) is vector { 0.5, 0.5, 0.0 }", abs (*field.value (0.5)), abs (Vector { 0.5, 0.5, 0.0 }), abs (epsilon));
});

} // namespace
} // namespace neutrino::test

