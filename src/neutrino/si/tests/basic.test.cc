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
#include <iostream>
#include <iomanip>
#include <limits>

// Neutrino:
#include <neutrino/test/test.h>

// Local:
#include "../si.h"


namespace neutrino::test {
namespace {

using namespace test_asserts;
using namespace si::units;
using namespace si::literals;

// Validity asserts:
static_assert (std::is_literal_type<Dimensionless>(), "Unit must be a literal type");
static_assert (std::is_literal_type<si::Quantity<Dimensionless>>(), "Quantity must be a literal type");


static RuntimeTest t_expression ("SI expression validity assertions", []{
	si::Quantity<Meter> one_meter;

	verify_compilation (5.0 * Meter());
	verify_compilation (Meter() * 5.0);

	verify_compilation (5.0 * one_meter);
	verify_compilation (one_meter * 5.0);

	si::Quantity<Meter> length;

	verify_compilation (length += one_meter);

	verify_compilation (std::numeric_limits<si::quantities::Current>::max());
	verify_compilation (si::quantities::Mass (1.0) * si::quantities::Mass (2.0));

	verify_compilation (one_meter.in<Foot>());
});


static RuntimeTest t_comparison ("SI comparison operators", []{
	si::Quantity<Meter> m1 { 10.0 };
	si::Quantity<Meter> m2 { 10.1 };

	verify ("10.0 m < 10.1 m", m1 < m2);
	verify ("10.0 m <= 10.1 m", m1 <= m2);
	verify ("10.0 m <= 10.0 m", m1 <= m1);

	verify ("10.1 m > 10.0 m", m2 > m1);
	verify ("10.1 m >= 10.0 m", m2 >= m1);
	verify ("10.0 m >= 10.0 m", m1 >= m1);

	m1 += 1.0 * Meter();
	m2 -= 0.1 * Meter();

	verify_equal_with_epsilon ("10.0 m + 1 m = 11 m", m1, 11.0 * Meter(), 1e-8 * Meter());
	verify_equal_with_epsilon ("10.1 m - 0.1 m = 10 m", m2, 10.0 * Meter(), 1e-8 * Meter());
	verify ("10.0 m == 10.0 m", m1 == m1);
	verify ("10.0 m != 10.1 m", m1 != m2);
});


static RuntimeTest t_basic_arithmetic_operators ("SI basic arithmetic operations", []{
	// Adding quantities for differently scaled units:
	auto len1 = 10.0 * Meter() + 1.0 * Foot();
	auto len2 = 10.0 * Meter() - 1.0 * Foot();
	auto area1 = 10.0 * Meter() * (1.0 * Foot());
	auto area2 = 10.0 * Meter() / (1.0 * Foot());
	auto volume = 1.0 * Kilometer() * (1.0 * Kilometer()) * (1.0 * Meter());

	verify_equal_with_epsilon ("10 m + 1 ft", len1, 10.304800609 * Meter(), 1e-9 * Meter());
	verify_equal_with_epsilon ("10 m - 1 ft", len2, 9.695199391 * Meter(), 1e-9 * Meter());
	verify_equal_with_epsilon ("10 m * 1 ft", area1, 3.048006096012 * SquareMeter(), 1e-9 * SquareMeter());
	verify_equal_with_epsilon ("10 m / 1 ft", area2, 32.808333333333, 1e-9);
	verify_equal_with_epsilon ("1 km * 1 km * 1 m", volume, 1'000'000.0 * CubicMeter(), 1e-9 * CubicMeter());
});


static RuntimeTest t_multiplication_division ("SI multiplication, division and counting quantities", []{
	using SquareMeter = decltype (1_m * 1_m)::Unit;
	using SquareKilometer = decltype (1_km * 1_km)::Unit;
	using InvMeter = decltype (1 / 1_m)::Unit;
	using InvKilometer = decltype (1 / 1_km)::Unit;

	verify_equal_with_epsilon ("1 m * 1 km = 1000.0 [in<SquareMeter>()]", (1_m * 1_km).in<SquareMeter>(), 1000.0, 1e-9);
	verify_equal_with_epsilon ("1 m * 1 km = 1000 m² [static_cast<> to SquareMeter]", static_cast<si::Quantity<SquareMeter>> (1_m * 1_km), 1000_m2, 1e-9_m2);
	verify_equal_with_epsilon ("1 m * 1 km = 1.0 [value()]", (1_m * 1_km).value(), 1.0, 1e-9);
	verify_equal_with_epsilon ("1 m * 1 km = 1000.0 [base_value()]", (1_m * 1_km).base_value(), 1000.0, 1e-9);
	verify_equal_with_epsilon ("1 m * 1 km = 0.001 [in<SquareKilometer>()]", (1_m * 1_km).in<SquareKilometer>(), 0.001, 1e-9);
	verify_equal_with_epsilon ("1 m * 1 km = 0.001 [static_cast<> to SquareKilometer]", static_cast<si::Quantity<SquareKilometer>> (1_m * 1_km).value(), 0.001, 1e-9);
	verify_equal_with_epsilon ("2 km * 1 km / 5 = 400'000 [value()]", (2_km * 1_km / 5_m).in<si::Meter>(), 400'000.0, 1e-9);
	verify_equal_with_epsilon ("2 km * 1 km / 5 m = 400'000 m", 2_km * 1_km / 5_m, 400'000_m, 1e-9_m);
	verify_equal_with_epsilon ("5 / 1_m == 5000 / km [in<InvKilometer>()]", (5 / 1_m).in<InvKilometer>(), 5000.0, 1e-9);
	verify_equal_with_epsilon ("5 / 1_m == 5000 / km [static_cast<> to InvKilometer]", static_cast<si::Quantity<InvKilometer>> (5 / 1_m), 5000.0 * InvKilometer(), 1e-9 * InvMeter());
	verify_equal_with_epsilon ("1_m / 5_m == 0.2", 1_m / 5_m, 0.2, 1e-9);
	verify_equal_with_epsilon ("1_km / 5_m == 200.0", 1_km / 5_m, 200.0, 1e-9);
	verify_equal_with_epsilon ("1 / 1_in * 5_in == 5.0", 1 / 1_in * 5_in, 5.0, 1e-9);
});


static RuntimeTest t_temperature ("SI temperature tests", []{
	si::Quantity<Kelvin> t1 { 273.15 - 40.0 };
	si::Quantity<Celsius> t2 { -40.0 };
	si::Quantity<Fahrenheit> t3 { -40.0 };

	using si::quantities::Temperature;

	verify_equal_with_epsilon ("temperature quantities are equal (t1, t2)", Temperature (t1), Temperature (t2), 1e-9 * Kelvin());
	verify_equal_with_epsilon ("temperature quantities are equal (t2, t3)", Temperature (t2), Temperature (t3), 1e-9 * Kelvin());
	verify_equal_with_epsilon ("temperature quantities are equal (t3, t1)", Temperature (t3), Temperature (t1), 1e-9 * Kelvin());
});


static RuntimeTest t_angle ("SI angle tests", []{
	si::Quantity<Radian> a1 { 1.0 };
	si::Quantity<Degree> a2 { 57.295'779'513 };
	si::Quantity<RadianPerSecond> s1 { 1.0 };
	si::Quantity<Hertz> s2 { 1.0 / (2.0 * M_PI) };
	si::Quantity<RadianPerSecond> s3 { s2.in<Hertz>() * (2.0 * M_PI) };
	si::Quantity<RadianPerSecond> s4 = si::convert (1.0_Hz / (2.0 * M_PI));

	verify_equal_with_epsilon ("radians/degrees equality test", a1, a2, 1e-9 * Radian());
	verify_equal_with_epsilon ("radians per second/hertz conversion test 1", s1, s3, 1e-9 * RadianPerSecond());
	verify_equal_with_epsilon ("radians per second/hertz conversion test 2", s3, s4, 1e-9 * RadianPerSecond());
});


static RuntimeTest t_velocity ("SI velocity tests", []{
	si::Quantity<MeterPerSecond> s1 { 1.0 };
	si::Quantity<FootPerMinute> s2 { 196.85 };
	si::Quantity<FootPerSecond> s3 { 3.280833333333333 };
	si::Quantity<Knot> s4 { 1.9438444924406046432 };
	si::Quantity<KilometerPerHour> s5 { 3.6 };

	verify_equal_with_epsilon ("velocity quantities are equal (s1, s2)", s1, s2, 1e-9 * MeterPerSecond());
	verify_equal_with_epsilon ("velocity quantities are equal (s2, s3)", s2, s3, 1e-9 * MeterPerSecond());
	verify_equal_with_epsilon ("velocity quantities are equal (s3, s4)", s3, s4, 1e-9 * MeterPerSecond());
	verify_equal_with_epsilon ("velocity quantities are equal (s4, s5)", s4, s5, 1e-9 * MeterPerSecond());
	verify_equal_with_epsilon ("velocity quantities are equal (s5, s1)", s5, s1, 1e-9 * MeterPerSecond());
});


static RuntimeTest t_parsing ("SI parsing tests", []{
	si::DynamicUnit unit;

	unit = si::parse_unit (" m^2 ");
	verify ("parsed unit is SquareMeter", unit == SquareMeter::dynamic_unit());

	unit = si::parse_unit ("m^2 kg s^-2 A^0 K^-1 mol^0 cd^0 rad^0");
	verify ("parsed unit is JoulePerKelvin", unit == JoulePerKelvin::dynamic_unit());

	verify ("m^2 km is parsed correctly", si::parse_unit ("m^2   km") == si::DynamicUnit (3, 0, 0, 0, 0, 0, 0, 0, si::DynamicRatio (1000, 1)));
	verify ("m km^2 is parsed correctly", si::parse_unit ("m  km^2  ") == si::DynamicUnit (3, 0, 0, 0, 0, 0, 0, 0, si::DynamicRatio (1000000, 1)));
	verify ("m^-1 km^2 is parsed correctly", si::parse_unit (" m^-1 km^2  ") == si::DynamicUnit (1, 0, 0, 0, 0, 0, 0, 0, si::DynamicRatio (1000000, 1)));

	using si::quantities::Velocity;

	// No exceptions expected:
	{
		si::Quantity<FootPerMinute> v1;
		parse ("1 fpm", v1);
	}

	// No exceptions expected - quantity is convertible and should be converted:
	{
		si::Quantity<MeterPerSecond> v2;
		parse ("1 fpm", v2);
	}

	// Test type incompatibility:
	try {
		Velocity v;
		parse ("1 kg", v);
		verify ("exception IncompatibleTypes is thrown on incompatible types", false);
	}
	catch (si::IncompatibleTypes&)
	{ }

	// Make sure that the parse() function can convert values and units,
	// if they're convertible (same SI exponents vector):
	si::Quantity<MeterPerSecond> v1;
	si::Quantity<FootPerSecond> v2;
	parse ("15 m s^-1", v1);
	parse ("15 m s^-1", v2);
	verify_equal_with_epsilon ("v1 is 15 m/s", v1, 15.0 * MeterPerSecond(), 1e-9 * MeterPerSecond());
	verify_equal_with_epsilon ("v1 == v2", v1, v2, 1e-9 * MeterPerSecond());
	parse ("49.2125 ft s^-1", v1);
	parse ("49.2125 ft s^-1", v2);
	verify_equal_with_epsilon ("v1 is 15 m/s", v1, 15.0 * MeterPerSecond(), 1e-9 * MeterPerSecond());
	verify_equal_with_epsilon ("v1 == v2", v1, v2, 1e-9 * MeterPerSecond());

	// Test non-base symbols:
	parse ("100 fpm", v1);
	verify_equal_with_epsilon ("v1 is 100 fpm", v1, 100.0 * FootPerMinute(), 1e-9 * MeterPerSecond());

	// Test generic conversion:
	verify_equal_with_epsilon ("15 m/s converts correctly to 'fps'", quantity (15.0 * MeterPerSecond(), "fps"), 49.2125, 1e-9);

	// Test division character:
	verify ("m/s == m / s", si::parse_unit ("m/s") == si::parse_unit ("m / s"));
	verify ("m s^-1 == m/s", si::parse_unit ("m s^-1") == si::parse_unit ("m/s"));
	verify ("m s^-2 kg^-3 == m / s^2 / kg^3", si::parse_unit ("m s^-2 kg^-3") == si::parse_unit ("m / s^2 / kg^3"));
	verify ("/s == s^-1", si::parse_unit (" / s") == si::parse_unit ("s^-1"));
	verify ("m / s kg == m s^-1 kg", si::parse_unit ("m / s kg") == si::parse_unit ("m s^-1 kg"));
});

} // namespace
} // namespace neutrino::test


