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
#include <xefis/config/all.h>
#include <xefis/support/simulation/components/capacitor.h>
#include <xefis/support/simulation/components/resistor.h>
#include <xefis/support/simulation/components/voltage_source.h>
#include <xefis/support/simulation/electrical/network.h>
#include <xefis/support/simulation/electrical/node_voltage_solver.h>

// Neutrino:
#include <neutrino/test/auto_test.h>
#include <neutrino/test/test_values.h>
#include <neutrino/time_helper.h>

// Standard:
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <vector>


namespace xf::test {
namespace {

std::filesystem::path const	kTestDataDir	= "share/tests/xefis/support/simulation/electrical/tests/network.test/";


AutoTest t_r_1 ("Electrical: network R.1 single R", []{
	electrical::Network network;
	auto& gnd = network.make_node ("GND");
	auto& vcc = network.make_node ("VCC");

	auto& v1 = network.add<electrical::VoltageSource> ("V1", 5_V, 10_Ohm);
	vcc << v1 << gnd;

	auto& r1 = network.add<electrical::Resistor> ("R1", 10_Ohm);
	vcc >> r1 >> gnd;

	auto const precision = 1e-6;
	electrical::NodeVoltageSolver solver (network, precision);

	test_asserts::verify_equal_with_epsilon ("R1 voltage is correct", r1.voltage(), +2.5_V, precision * 1_V);
	test_asserts::verify_equal_with_epsilon ("R1 current is correct", r1.current(), +0.25_A, precision * 1_A);
});


AutoTest t_r_2 ("Electrical: network R.2 parallel R", []{
	electrical::Network network;
	auto& gnd = network.make_node ("GND");
	auto& vcc = network.make_node ("VCC");

	auto& v1 = network.add<electrical::VoltageSource> ("V1", 5_V, 5_Ohm);
	vcc << v1 << gnd;

	auto& r1 = network.add<electrical::Resistor> ("R1", 10_Ohm);
	vcc >> r1 >> gnd;

	auto& r2 = network.add<electrical::Resistor> ("R2", 10_Ohm);
	vcc >> r2 >> gnd;

	auto const precision = 1e-6;
	electrical::NodeVoltageSolver solver (network, precision);

	test_asserts::verify_equal_with_epsilon ("R1 voltage is correct", r1.voltage(), +2.5_V, precision * 1_V);
	test_asserts::verify_equal_with_epsilon ("R1 current is correct", r1.current(), +0.25_A, precision * 1_A);
	test_asserts::verify_equal_with_epsilon ("R2 voltage is correct", r2.voltage(), +2.5_V, precision * 1_V);
	test_asserts::verify_equal_with_epsilon ("R2 current is correct", r2.current(), +0.25_A, precision * 1_A);
});


AutoTest t_r_3 ("Electrical: network R.3 serial R", []{
	electrical::Network network;
	auto& gnd = network.make_node ("GND");
	auto& vcc = network.make_node ("VCC");
	auto& n1 = network.make_node ("N1");

	auto& v1 = network.add<electrical::VoltageSource> ("V1", 5_V, 5_Ohm);
	vcc << v1 << gnd;

	auto& r1 = network.add<electrical::Resistor> ("R1", 2.5_Ohm);
	vcc >> r1 >> n1;

	auto& r2 = network.add<electrical::Resistor> ("R2", 2.5_Ohm);
	n1 >> r2 >> gnd;

	auto const precision = 1e-6;

	{
		electrical::NodeVoltageSolver solver (network, precision);
		test_asserts::verify_equal_with_epsilon ("R1 (2.5 Ω) voltage is correct", r1.voltage(), +1.25_V, precision * 1_V);
		test_asserts::verify_equal_with_epsilon ("R1 (2.5 Ω) current is correct", r1.current(), +0.5_A, precision * 1_A);
		test_asserts::verify_equal_with_epsilon ("R2 (2.5 Ω) voltage is correct", r2.voltage(), +1.25_V, precision * 1_V);
		test_asserts::verify_equal_with_epsilon ("R2 (2.5 Ω) current is correct", r2.current(), +0.5_A, precision * 1_A);
	}

	r1.set_resistance (2_Ohm);
	r2.set_resistance (3_Ohm);

	{
		electrical::NodeVoltageSolver solver (network, precision);
		test_asserts::verify_equal_with_epsilon ("R1 (2 Ω) voltage is correct", r1.voltage(), +1_V, precision * 1_V);
		test_asserts::verify_equal_with_epsilon ("R1 (2 Ω) current is correct", r1.current(), +0.5_A, precision * 1_A);
		test_asserts::verify_equal_with_epsilon ("R2 (3 Ω) voltage is correct", r2.voltage(), +1.5_V, precision * 1_V);
		test_asserts::verify_equal_with_epsilon ("R2 (3 Ω) current is correct", r2.current(), +0.5_A, precision * 1_A);
	}
});


AutoTest t_r_4 ("Electrical: network R.4", []{
	electrical::Network network;
	auto& gnd = network.make_node ("GND");
	auto& vcc = network.make_node ("VCC");
	auto& n1 = network.make_node ("N1");
	auto& n2 = network.make_node ("N2");

	auto& v1 = network.add<electrical::VoltageSource> ("V1", 5_V, 1e-9_Ohm);
	vcc << v1 << gnd;

	auto& r1 = network.add<electrical::Resistor> ("R1", 10_Ohm);
	vcc >> r1 >> n1;

	auto& r2 = network.add<electrical::Resistor> ("R2", 5_Ohm);
	vcc >> r2 >> n2;

	auto& r3 = network.add<electrical::Resistor> ("R3", 5_Ohm);
	n1 << r3 << n2;

	auto& r4 = network.add<electrical::Resistor> ("R4", 5_Ohm);
	n1 >> r4 >> gnd;

	auto& r5 = network.add<electrical::Resistor> ("R5", 5_Ohm);
	n2 >> r5 >> gnd;

	auto const precision = 1e-5;
	electrical::NodeVoltageSolver solver (network, precision);

	test_asserts::verify_equal_with_epsilon ("R1 voltage is correct", r1.voltage(), +3.07692_V, precision * 1_V);
	test_asserts::verify_equal_with_epsilon ("R2 voltage is correct", r2.voltage(), +2.69231_V, precision * 1_V);
	test_asserts::verify_equal_with_epsilon ("R3 voltage is correct", r3.voltage(), +0.384615_V, precision * 1_V);
	test_asserts::verify_equal_with_epsilon ("R4 voltage is correct", r4.voltage(), +1.92308_V, precision * 1_V);
	test_asserts::verify_equal_with_epsilon ("R5 voltage is correct", r5.voltage(), +2.30769_V, precision * 1_V);
});


AutoTest t_c_1 ("Electrical: network C.1", []{
	constexpr bool write_file = false;

	electrical::Network network;
	auto& gnd = network.make_node ("GND");
	auto& vcc = network.make_node ("VCC");
	auto& n1 = network.make_node ("N1");

	auto& v1 = network.add<electrical::VoltageSource> ("V1", 5_V, 1_mOhm);
	vcc << v1 << gnd;

	auto& r1 = network.add<electrical::Resistor> ("R1", 100_Ohm);
	vcc >> r1 >> n1;

	auto& c1 = network.add<electrical::Capacitor> ("C1", 1_uF, 10_Ohm);
	n1 >> c1 >> gnd;

	auto t = 0_s;
	auto const dt = 500_ns;
	auto const precision = write_file ? 1e-9 : 1e-6;
	// Errors accumulate during integration (evolution), so required precision when comparing must be far less than simulation precision:
	auto const required_precision = 1000 * precision;
	electrical::NodeVoltageSolver solver (network, precision, 1000);
	TestValues test_values;

	for (int j = 0; j < 6; ++j)
	{
		for (; t < j * 1_ms; t += dt)
		{
			solver.evolve (dt);
			test_values.add_line (t, v1.voltage(), r1.voltage(), c1.voltage());
		}

		v1.set_source_voltage (-v1.source_voltage());
	}

	write_or_compare (test_values, kTestDataDir / "t_c_1.dat", required_precision, write_file);
});


AutoTest t_c_2 ("Electrical: network C.2", []{
	constexpr bool write_file = false;
	constexpr auto high_z = 1_GOhm;

	electrical::Network network;
	auto& gnd = network.make_node ("GND");
	auto& vcc = network.make_node ("VCC");
	auto& n1 = network.make_node ("N1");
	auto& n2 = network.make_node ("N2");
	auto& n3 = network.make_node ("N3");

	auto& v1 = network.add<electrical::VoltageSource> ("V1", 5_V, 1_mOhm);
	vcc << v1 << gnd;

	auto& s1 = network.add<electrical::Resistor> ("RS1", 1_mOhm);
	vcc >> s1 >> n2;

	auto& s2 = network.add<electrical::Resistor> ("RS2", high_z);
	n2 >> s2 >> n3;

	auto& r1 = network.add<electrical::Resistor> ("R1", 100_Ohm);
	n2 >> r1 >> n1;

	auto& c1 = network.add<electrical::Capacitor> ("C1", 100_nF, 1_Ohm);
	n1 >> c1 >> gnd;

	auto& c2 = network.add<electrical::Capacitor> ("C2", 1_uF, 1_Ohm);
	n3 >> c2 >> gnd;

	auto t = 0_s;
	auto const dt = 100_ns;
	auto const precision = write_file ? 1e-9 : 1e-6;
	auto const required_precision = 1000 * precision;
	electrical::NodeVoltageSolver solver (network, precision, 20000);
	TestValues test_values;

	for (; t < 0.1_ms; t += dt)
	{
		solver.evolve (dt);
		test_values.add_line (t, r1.voltage(), c1.voltage(), c2.voltage(), s1.voltage(), s2.voltage());
	}

	v1.set_source_voltage (5_V);
	s1.set_resistance (high_z);
	s2.set_resistance (1_mOhm);
	solver.solve_throwing();

	for (; t < 0.25_ms; t += dt)
	{
		solver.evolve (dt);
		test_values.add_line (t, r1.voltage(), c1.voltage(), c2.voltage(), s1.voltage(), s2.voltage());
	}

	write_or_compare (test_values, kTestDataDir / "t_c_2.dat", required_precision, write_file);
});


AutoTest t_v_1 ("Electrical: network V.1 (two voltage sources)", []{
	electrical::Network network;
	auto& gnd = network.make_node ("GND");
	auto& vcc1 = network.make_node ("VCC1");
	auto& vcc2 = network.make_node ("VCC2");
	auto& n1 = network.make_node ("N1");

	auto& v1 = network.add<electrical::VoltageSource> ("V1", 5_V, 1_mOhm);
	vcc1 << v1 << gnd;

	auto& v2 = network.add<electrical::VoltageSource> ("V2", 3_V, 1_mOhm);
	vcc2 << v2 << gnd;

	auto& r1 = network.add<electrical::Resistor> ("R1", 100_Ohm);
	vcc1 >> r1 >> n1;

	auto& r2 = network.add<electrical::Resistor> ("R2", 500_Ohm);
	vcc2 >> r2 >> n1;

	auto& r3 = network.add<electrical::Resistor> ("R3", 1_kOhm);
	n1 >> r3 >> gnd;

	auto const precision = 1e-6;
	electrical::NodeVoltageSolver solver (network, precision);

	test_asserts::verify_equal_with_epsilon ("R1 voltage is correct", r1.voltage(), +0.692306_V, precision * 1_V);
	test_asserts::verify_equal_with_epsilon ("R2 voltage is correct", r2.voltage(), -1.307685_V, precision * 1_V);
	test_asserts::verify_equal_with_epsilon ("R3 voltage is correct", r3.voltage(), +4.307687_V, precision * 1_V);
});

} // namespace
} // namespace xf::test

