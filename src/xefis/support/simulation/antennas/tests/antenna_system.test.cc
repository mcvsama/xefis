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
#include <xefis/config/all.h>
#include <xefis/support/math/rotations.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/simulation/antennas/antenna_system.h>
#include <xefis/support/simulation/antennas/whip_antenna_model.h>
#include <xefis/support/simulation/devices/antenna.h>

// Neutrino:
#include <neutrino/test/auto_test.h>

// Standard:
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>


namespace xf::test {
namespace {

namespace test_asserts = nu::test_asserts;
using namespace nu::si::literals;

class TestAntenna: public xf::sim::Antenna
{
  public:
	explicit
	TestAntenna (AntennaModel const& antenna_model, AntennaSystem& antenna_system):
		Antenna (MassMoments<BodyCOM>::zero(), antenna_model, antenna_system, [this] (Antenna::ReceivedSignal const& signal) {
			_received_signals.push_back (signal);
		})
	{ }

	explicit
	TestAntenna (MassMomentsAtArm<BodyCOM> const& mass_moments, AntennaModel const& antenna_model, AntennaSystem& antenna_system):
		Antenna (mass_moments, antenna_model, antenna_system, [this] (Antenna::ReceivedSignal const& signal) {
			_received_signals.push_back (signal);
		})
	{ }

	[[nodiscard]]
	std::vector<Antenna::ReceivedSignal> const&
	received_signals() const noexcept
		{ return _received_signals; }

  private:
	std::vector<Antenna::ReceivedSignal> _received_signals;
};


nu::AutoTest t_origin_placement ("Antenna body-origin placement stays exact for COM-offset bodies", []{
	auto antenna_model = WhipAntennaModel (1_m);
	auto system = AntennaSystem (1_s);
	auto antenna = TestAntenna (MassMomentsAtArm<BodyCOM> (1_kg, { 0_m, 0_m, 1_m }, math::identity), antenna_model, system);
	auto const origin_placement_in_com = Placement<BodyCOM, BodyOrigin> (
		{ 0_m, 0_m, -1_m },
		x_rotation<BodyCOM, BodyOrigin> (15_deg)
	);
	auto const expected_origin_placement = Placement<WorldSpace, BodyOrigin> (
		{ 10_m, 20_m, 30_m },
		z_rotation<WorldSpace, BodyOrigin> (30_deg)
	);

	antenna.set_origin_placement<BodyCOM> (origin_placement_in_com);
	antenna.set_origin_placement<WorldSpace> (expected_origin_placement);

	test_asserts::verify_equal_with_epsilon ("Antenna origin position is preserved",
											 antenna.origin_placement<WorldSpace>().position(),
											 expected_origin_placement.position(),
											 1e-12_m);
	test_asserts::verify_equal_with_epsilon ("Antenna origin rotation is preserved",
											 to_rotation_vector (antenna.origin_placement<WorldSpace>().body_rotation()),
											 to_rotation_vector (expected_origin_placement.body_rotation()),
											 1e-12_rad);
	test_asserts::verify_equal_with_epsilon ("Antenna COM reflects origin offset",
											 antenna.placement().position(),
											 expected_origin_placement.position() - antenna.placement().body_rotation() * origin_placement_in_com.position(),
											 1e-12_m);
});


nu::AutoTest t_1 ("Antenna signals get propagated", []{
	auto const matched_antenna_length = 1_m;
	auto antenna_model = WhipAntennaModel (matched_antenna_length);
	auto mismatched_antenna_model = WhipAntennaModel (1.2_m);

	auto system = AntennaSystem (1_s);
	auto tx_antenna = TestAntenna (antenna_model, system);
	auto rx_matched_antenna = TestAntenna (antenna_model, system);
	auto rx_mismatched_antenna = TestAntenna (mismatched_antenna_model, system);
	auto rx_misaligned_antenna = TestAntenna (antenna_model, system);
	auto const no_rotation = kNoRotation<WorldSpace, BodyOrigin>;
	auto const tx_placement = Placement<WorldSpace, BodyOrigin> ({ 0_m, 0_m, 0_m }, no_rotation);
	auto const rx_placement = Placement<WorldSpace, BodyOrigin> ({ 1_m, 0_m, 0_m }, no_rotation);
	auto const rx_misaligned_placement = Placement<WorldSpace, BodyOrigin> (
		{ 1_m, 0_m, 0_m },
		x_rotation<WorldSpace, BodyOrigin> (45_deg)
	);
	tx_antenna.set_origin_placement<WorldSpace> (tx_placement);
	rx_matched_antenna.set_origin_placement<WorldSpace> (rx_placement);
	rx_mismatched_antenna.set_origin_placement<WorldSpace> (rx_placement);
	rx_misaligned_antenna.set_origin_placement<WorldSpace> (rx_misaligned_placement);

	tx_antenna.emit_signal ({
		.time		= 0_s,
		.power		= 1_W,
		.frequency	= 100_MHz,
		.payload	= "test signal",
	});

	tx_antenna.emit_signal ({
		.time				= 0_s,
		.power				= 1_W,
		.frequency			= 100_MHz,
		.payload			= "test signal",
	});

	system.process (0_s);
	test_asserts::verify_equal ("Matched antenna receives nothing before propagation delay", rx_matched_antenna.received_signals().size(), 0uz);
	test_asserts::verify_equal ("Mismatched antenna receives nothing before propagation delay", rx_mismatched_antenna.received_signals().size(), 0uz);
	test_asserts::verify_equal ("Misaligned antenna receives nothing before propagation delay", rx_misaligned_antenna.received_signals().size(), 0uz);

	system.process (1_ms);
	test_asserts::verify_equal ("Matched antenna receives both signals", rx_matched_antenna.received_signals().size(), 2uz);
	test_asserts::verify_equal ("Mismatched antenna receives both signals", rx_mismatched_antenna.received_signals().size(), 2uz);
	test_asserts::verify_equal ("Misaligned antenna receives both signals", rx_misaligned_antenna.received_signals().size(), 2uz);

	auto const matched_signal_power = rx_matched_antenna.received_signals().front().signal_power;
	auto const mismatched_signal_power = rx_mismatched_antenna.received_signals().front().signal_power;
	auto const misaligned_signal_power = rx_misaligned_antenna.received_signals().front().signal_power;

	test_asserts::verify ("Mismatched antenna receives weaker signal", mismatched_signal_power < matched_signal_power);
	test_asserts::verify ("Misaligned antenna receives weaker signal", misaligned_signal_power < matched_signal_power);
	test_asserts::verify ("Misaligned antenna receives about half power",
						  misaligned_signal_power > 0.4999 * matched_signal_power &&
						  misaligned_signal_power < 0.5001 * matched_signal_power);

	auto const tuned_frequency = kSpeedOfLight / (4.0 * matched_antenna_length);
	tx_antenna.emit_signal ({
		.time		= 2_ms,
		.power		= 1_W,
		.frequency	= tuned_frequency,
		.payload	= "tuned signal",
	});
	system.process (3_ms);

	test_asserts::verify_equal ("Matched antenna receives tuned signal", rx_matched_antenna.received_signals().size(), 3uz);
	test_asserts::verify_equal ("Mismatched antenna receives tuned signal", rx_mismatched_antenna.received_signals().size(), 3uz);
	test_asserts::verify_equal ("Misaligned antenna receives tuned signal", rx_misaligned_antenna.received_signals().size(), 3uz);
	test_asserts::verify_equal ("Matched antenna receives tuned payload", rx_matched_antenna.received_signals().back().payload, "tuned signal");
	test_asserts::verify ("Tuned signal is stronger than before",
						  rx_matched_antenna.received_signals().back().signal_power > matched_signal_power);
});


nu::AutoTest t_2 ("Antenna signal arrives exactly at propagation-time boundary", []{
	auto antenna_model = WhipAntennaModel (1_m);
	auto system = AntennaSystem (10_s);
	auto tx_antenna = TestAntenna (antenna_model, system);
	auto rx_antenna = TestAntenna (antenna_model, system);
	auto const no_rotation = kNoRotation<WorldSpace, BodyOrigin>;
	auto const boundary_distance = kSpeedOfLight * 1_ms;

	tx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 0_m, 0_m, 0_m }, no_rotation));
	rx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ boundary_distance, 0_m, 0_m }, no_rotation));

	tx_antenna.emit_signal ({
		.time		= 0_s,
		.power		= 1_W,
		.frequency	= 100_MHz,
		.payload	= "boundary signal",
	});

	system.process (0_s);
	test_asserts::verify_equal ("No signal before propagation boundary", rx_antenna.received_signals().size(), 0uz);

	system.process (1_ms);
	test_asserts::verify_equal ("Signal received exactly at propagation boundary", rx_antenna.received_signals().size(), 1uz);
	test_asserts::verify_equal ("Boundary payload propagated", rx_antenna.received_signals().front().payload, "boundary signal");
});


nu::AutoTest t_3 ("Antenna signal is delivered only once", []{
	auto antenna_model = WhipAntennaModel (1_m);
	auto system = AntennaSystem (10_s);
	auto tx_antenna = TestAntenna (antenna_model, system);
	auto rx_antenna = TestAntenna (antenna_model, system);
	auto const no_rotation = kNoRotation<WorldSpace, BodyOrigin>;

	tx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 0_m, 0_m, 0_m }, no_rotation));
	rx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 1_m, 0_m, 0_m }, no_rotation));

	tx_antenna.emit_signal ({
		.time		= 0_s,
		.power		= 1_W,
		.frequency	= 100_MHz,
		.payload	= "single delivery",
	});

	system.process (1_ms);
	test_asserts::verify_equal ("Signal delivered first time", rx_antenna.received_signals().size(), 1uz);

	for (auto i = 0uz; i < 10; ++i)
	{
		system.process (2_ms);
		test_asserts::verify_equal ("Signal not delivered second time", rx_antenna.received_signals().size(), 1uz);
	}
});


nu::AutoTest t_4 ("Antenna signals reach nearer receivers first", []{
	auto antenna_model = WhipAntennaModel (1_m);
	auto system = AntennaSystem (10_s);
	auto tx_antenna = TestAntenna (antenna_model, system);
	auto rx_near_antenna = TestAntenna (antenna_model, system);
	auto rx_far_antenna = TestAntenna (antenna_model, system);
	auto const no_rotation = kNoRotation<WorldSpace, BodyOrigin>;
	auto const near_distance = kSpeedOfLight * 1_ms;
	auto const far_distance = kSpeedOfLight * 2_ms;

	tx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 0_m, 0_m, 0_m }, no_rotation));
	rx_near_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ near_distance, 0_m, 0_m }, no_rotation));
	rx_far_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ far_distance, 0_m, 0_m }, no_rotation));

	tx_antenna.emit_signal ({
		.time		= 0_s,
		.power		= 1_W,
		.frequency	= 100_MHz,
		.payload	= "distance ordering",
	});

	system.process (1_ms);
	test_asserts::verify_equal ("Near receiver gets signal first", rx_near_antenna.received_signals().size(), 1uz);
	test_asserts::verify_equal ("Far receiver still waiting", rx_far_antenna.received_signals().size(), 0uz);

	system.process (2_ms);
	test_asserts::verify_equal ("Far receiver gets signal later", rx_far_antenna.received_signals().size(), 1uz);
});


nu::AutoTest t_5 ("In-flight signal uses emitter placement at emission time", []{
	auto const run_scenario = [](bool const move_emitter_after_emit) {
		auto antenna_model = WhipAntennaModel (1_m);
		auto system = AntennaSystem (10_s);
		auto tx_antenna = TestAntenna (antenna_model, system);
		auto rx_antenna = TestAntenna (antenna_model, system);
		auto const no_rotation = kNoRotation<WorldSpace, BodyOrigin>;

		tx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 0_m, 0_m, 0_m }, no_rotation));
		rx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 1_m, 0_m, 0_m }, no_rotation));

		tx_antenna.emit_signal ({
			.time		= 0_s,
			.power		= 1_W,
			.frequency	= 100_MHz,
			.payload	= "snapshot",
		});

		if (move_emitter_after_emit)
		{
			tx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> (
				{ 0_m, 0_m, 0_m },
				y_rotation<WorldSpace, BodyOrigin> (90_deg)
			));
		}

		system.process (1_ms);
		test_asserts::verify_equal ("Scenario receiver gets exactly one signal", rx_antenna.received_signals().size(), 1uz);
		return rx_antenna.received_signals().front().signal_power;
	};

	auto const reference_power = run_scenario (false);
	auto const moved_power = run_scenario (true);

	test_asserts::verify_equal_with_epsilon (
		"Moving emitter after emission does not affect in-flight signal",
		moved_power,
		reference_power,
		1e-12_W
	);
});


nu::AutoTest t_6 ("Antenna deregistration before arrival does not break propagation", []{
	auto antenna_model = WhipAntennaModel (1_m);
	auto system = AntennaSystem (10_s);
	auto tx_antenna = TestAntenna (antenna_model, system);
	auto rx_survivor_antenna = TestAntenna (antenna_model, system);
	auto const no_rotation = kNoRotation<WorldSpace, BodyOrigin>;
	auto const boundary_distance = kSpeedOfLight * 1_ms;

	tx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 0_m, 0_m, 0_m }, no_rotation));
	rx_survivor_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ boundary_distance, 0_m, 0_m }, no_rotation));

	{
		auto rx_removed_antenna = TestAntenna (antenna_model, system);
		rx_removed_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ boundary_distance, 0_m, 0_m }, no_rotation));

		tx_antenna.emit_signal ({
			.time		= 0_s,
			.power		= 1_W,
			.frequency	= 100_MHz,
			.payload	= "deregistered receiver",
		});
	}

	system.process (1_ms);
	test_asserts::verify_equal ("Surviving receiver still gets the signal", rx_survivor_antenna.received_signals().size(), 1uz);
});


nu::AutoTest t_7 ("Receiver registered after emission receives in-flight signal", []{
	auto antenna_model = WhipAntennaModel (1_m);
	auto system = AntennaSystem (10_s);
	auto tx_antenna = TestAntenna (antenna_model, system);
	auto const no_rotation = kNoRotation<WorldSpace, BodyOrigin>;
	auto const boundary_distance = kSpeedOfLight * 1_ms;

	tx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 0_m, 0_m, 0_m }, no_rotation));
	tx_antenna.emit_signal ({
		.time		= 0_s,
		.power		= 1_W,
		.frequency	= 100_MHz,
		.payload	= "late receiver",
	});

	auto rx_late_antenna = TestAntenna (antenna_model, system);
	rx_late_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ boundary_distance, 0_m, 0_m }, no_rotation));

	system.process (0_s);
	test_asserts::verify_equal ("Late receiver gets nothing before arrival", rx_late_antenna.received_signals().size(), 0uz);

	system.process (1_ms);
	test_asserts::verify_equal ("Late receiver gets in-flight signal", rx_late_antenna.received_signals().size(), 1uz);
});


nu::AutoTest t_8 ("Zero-distance received signal power is finite", []{
	auto antenna_model = WhipAntennaModel (1_m);
	auto system = AntennaSystem (10_s);
	auto tx_antenna = TestAntenna (antenna_model, system);
	auto rx_antenna = TestAntenna (antenna_model, system);
	auto const no_rotation = kNoRotation<WorldSpace, BodyOrigin>;

	tx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 0_m, 0_m, 0_m }, no_rotation));
	rx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 0_m, 0_m, 0_m }, no_rotation));

	tx_antenna.emit_signal ({
		.time		= 0_s,
		.power		= 1_W,
		.frequency	= 100_MHz,
		.payload	= "zero distance",
	});

	system.process (0_s);
	test_asserts::verify_equal ("Zero-distance signal is delivered immediately", rx_antenna.received_signals().size(), 1uz);
	test_asserts::verify ("Zero-distance received signal power is finite", std::isfinite (rx_antenna.received_signals().front().signal_power.to_floating_point()));
	test_asserts::verify ("Zero-distance received signal power is non-negative", rx_antenna.received_signals().front().signal_power >= 0_W);
});


nu::AutoTest t_9 ("Orthogonal whip antenna polarization strongly suppresses reception", []{
	auto antenna_model = WhipAntennaModel (1_m);
	auto system = AntennaSystem (10_s);
	auto tx_antenna = TestAntenna (antenna_model, system);
	auto rx_aligned_antenna = TestAntenna (antenna_model, system);
	auto rx_orthogonal_antenna = TestAntenna (antenna_model, system);
	auto const no_rotation = kNoRotation<WorldSpace, BodyOrigin>;

	tx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 0_m, 0_m, 0_m }, no_rotation));
	rx_aligned_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 1_m, 0_m, 0_m }, no_rotation));
	rx_orthogonal_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> (
		{ 1_m, 0_m, 0_m },
		x_rotation<WorldSpace, BodyOrigin> (90_deg)
	));

	tx_antenna.emit_signal ({
		.time		= 0_s,
		.power		= 1_W,
		.frequency	= 100_MHz,
		.payload	= "orthogonal polarization",
	});

	system.process (1_ms);
	test_asserts::verify_equal ("Aligned receiver gets signal", rx_aligned_antenna.received_signals().size(), 1uz);
	test_asserts::verify_equal ("Orthogonal receiver gets signal object", rx_orthogonal_antenna.received_signals().size(), 1uz);

	auto const aligned_power = rx_aligned_antenna.received_signals().front().signal_power;
	auto const orthogonal_power = rx_orthogonal_antenna.received_signals().front().signal_power;
	test_asserts::verify ("Orthogonal polarization strongly suppresses power", orthogonal_power < 1e-6 * aligned_power);
});


nu::AutoTest t_10 ("Vertical offset reduces power for parallel whip antennas", []{
	auto antenna_model = WhipAntennaModel (1_m);
	auto system = AntennaSystem (10_s);
	auto tx_antenna = TestAntenna (antenna_model, system);
	auto rx_side_antenna = TestAntenna (antenna_model, system);
	auto rx_side_and_up_antenna = TestAntenna (antenna_model, system);
	auto const no_rotation = kNoRotation<WorldSpace, BodyOrigin>;

	tx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 0_m, 0_m, 0_m }, no_rotation));
	rx_side_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 1_m, 0.25_m, 0_m }, no_rotation));
	rx_side_and_up_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 1_m, 0.25_m, 0.1_m }, no_rotation));

	tx_antenna.emit_signal ({
		.time		= 0_s,
		.power		= 1_W,
		.frequency	= 100_MHz,
		.payload	= "vertical offset",
	});

	system.process (1_ms);
	test_asserts::verify_equal ("Side-offset receiver gets signal", rx_side_antenna.received_signals().size(), 1uz);
	test_asserts::verify_equal ("Side-and-up-offset receiver gets signal", rx_side_and_up_antenna.received_signals().size(), 1uz);

	auto const side_power = rx_side_antenna.received_signals().front().signal_power;
	auto const side_and_up_power = rx_side_and_up_antenna.received_signals().front().signal_power;
	test_asserts::verify ("Raising otherwise parallel receiver reduces received power", side_and_up_power < side_power);
});


nu::AutoTest t_11 ("Emission expires by ttl before reaching distant receiver", []{
	auto antenna_model = WhipAntennaModel (1_m);
	auto system = AntennaSystem (1_ms);
	auto tx_antenna = TestAntenna (antenna_model, system);
	auto rx_antenna = TestAntenna (antenna_model, system);
	auto const no_rotation = kNoRotation<WorldSpace, BodyOrigin>;
	auto const far_distance = kSpeedOfLight * 2_ms;

	tx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 0_m, 0_m, 0_m }, no_rotation));
	rx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ far_distance, 0_m, 0_m }, no_rotation));

	tx_antenna.emit_signal ({
		.time		= 0_s,
		.power		= 1_W,
		.frequency	= 100_MHz,
		.payload	= "ttl",
	});

	system.process (1.5_ms);
	test_asserts::verify_equal ("Signal not yet delivered before ttl cleanup", rx_antenna.received_signals().size(), 0uz);

	system.process (2_ms);
	test_asserts::verify_equal ("Expired signal never reaches receiver", rx_antenna.received_signals().size(), 0uz);
});


nu::AutoTest t_12 ("Signal payload integrity and ordering are preserved", []{
	auto antenna_model = WhipAntennaModel (1_m);
	auto system = AntennaSystem (10_s);
	auto tx_antenna = TestAntenna (antenna_model, system);
	auto rx_antenna = TestAntenna (antenna_model, system);
	auto const no_rotation = kNoRotation<WorldSpace, BodyOrigin>;

	tx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 0_m, 0_m, 0_m }, no_rotation));
	rx_antenna.set_origin_placement<WorldSpace> (Placement<WorldSpace, BodyOrigin> ({ 1_m, 0_m, 0_m }, no_rotation));

	tx_antenna.emit_signal ({
		.time		= 0_s,
		.power		= 1_W,
		.frequency	= 100_MHz,
		.payload	= "alpha",
	});
	tx_antenna.emit_signal ({
		.time		= 0_s,
		.power		= 1_W,
		.frequency	= 100_MHz,
		.payload	= "bravo",
	});
	tx_antenna.emit_signal ({
		.time		= 0_s,
		.power		= 1_W,
		.frequency	= 100_MHz,
		.payload	= "charlie",
	});

	system.process (1_ms);
	test_asserts::verify_equal ("Receiver got all payloads", rx_antenna.received_signals().size(), 3uz);
	test_asserts::verify_equal ("Payload #1 preserved", rx_antenna.received_signals()[0].payload, "alpha");
	test_asserts::verify_equal ("Payload #2 preserved", rx_antenna.received_signals()[1].payload, "bravo");
	test_asserts::verify_equal ("Payload #3 preserved", rx_antenna.received_signals()[2].payload, "charlie");
});


nu::AutoTest t_13 ("WhipAntennaModel rejects negative frequency-response sharpness", []{
	test_asserts::verify_throws<nu::InvalidArgument> ("Negative frequency-response sharpness should be rejected", []{
		[[maybe_unused]] auto const model = WhipAntennaModel (1_m, -1.0);
	});
});

} // namespace
} // namespace xf::test
