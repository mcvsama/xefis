/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
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
#include <xefis/support/stats/bandwidth_sampler.h>

// Neutrino:
#include <neutrino/test/auto_test.h>

// Standard:
#include <ranges>


namespace xf::test {
namespace {

namespace test_asserts = nu::test_asserts;
using namespace nu::si::literals;


nu::AutoTest t1 ("xf::BandwidthSampler anchors first flush at current time", []{
	auto sampler = xf::BandwidthSampler ({
		.bandwidth_measurement_interval = 1_s,
		.bandwidth_history_size = 8u,
	});

	sampler.flush (1_h);

	test_asserts::verify_equal ("first flush does not synthesize backlog",
								std::ranges::distance (sampler.samples()), 0l);

	sampler.record_bytes (256u, 1_h);
	sampler.flush (1_h + 1_s);

	auto samples = sampler.samples();
	test_asserts::verify_equal ("one elapsed interval yields one sample",
								std::ranges::distance (samples), 1l);
	test_asserts::verify_equal ("sample bandwidth matches recorded bytes over one second",
								*samples.begin(), xf::BandwidthSampler::Bandwidth (256.0));
});


nu::AutoTest t2 ("xf::BandwidthSampler distributes sparse timestamped events across elapsed buckets", []{
	auto sampler = xf::BandwidthSampler ({
		.bandwidth_measurement_interval = 1_s,
		.bandwidth_history_size = 8u,
	});

	sampler.record_bytes (100u, 1_h + 0.2_s);
	sampler.record_bytes (100u, 1_h + 1.2_s);
	sampler.record_bytes (100u, 1_h + 2.2_s);
	sampler.flush (1_h + 3.2_s);

	auto samples = sampler.samples();
	test_asserts::verify_equal ("three events one second apart yield three equal samples",
								std::ranges::distance (samples), 3l);

	auto it = samples.begin();
	test_asserts::verify_equal ("first sample matches first bucket", *it++, xf::BandwidthSampler::Bandwidth (100.0));
	test_asserts::verify_equal ("second sample matches second bucket", *it++, xf::BandwidthSampler::Bandwidth (100.0));
	test_asserts::verify_equal ("third sample matches third bucket", *it++, xf::BandwidthSampler::Bandwidth (100.0));
});

} // namespace
} // namespace xf::test
