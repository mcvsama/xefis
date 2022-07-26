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
#include <xefis/support/simulation/failure/sigmoidal_temperature_failure.h>

// Neutrino:
#include <neutrino/test/auto_test.h>

// Standard:
#include <cmath>


namespace xf::test {
namespace {

si::Time
test_lifetime (SigmoidalTemperatureFailure& failure_model, si::Temperature const test_temperature)
{
	auto failures = 0u;
	auto const tests = 1000000u;

	for (auto i = 0u; i < tests; ++i)
		if (failure_model.should_fail (test_temperature, 1_s))
			++failures;

	auto const actual_lifetime = 1_s * tests / static_cast<double> (failures);

	return actual_lifetime;
}


AutoTest t_1 ("SigmoidalTemperatureFailure", []{
	auto const expected_lifetime = 1000_s;
	SigmoidalTemperatureFailure failure_model (expected_lifetime, 300_K, 400_K);

	test_asserts::verify_equal_with_epsilon ("actual lifetime is more or less the expected lifetime at normal temperature",
											 test_lifetime (failure_model, 300_K), expected_lifetime, 0.1 * expected_lifetime);
	test_asserts::verify_equal_with_epsilon ("actual lifetime is more or less 0.5_s at the stress temperature",
											 test_lifetime (failure_model, 400_K), 2_s, 0.1 * 2_s);
});

} // namespace
} // namespace xf::test

