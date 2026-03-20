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
#include <xefis/core/sockets/tests/test_cycle.h>
#include <xefis/modules/simulation/virtual_modem.h>
#include <xefis/support/simulation/antennas/antenna_system.h>
#include <xefis/support/simulation/antennas/whip_antenna_model.h>
#include <xefis/support/simulation/devices/antenna.h>
#include <xefis/test/test_processing_loop.h>

// Neutrino:
#include <neutrino/test/auto_test.h>


namespace xf::test {
namespace {

namespace test_asserts = nu::test_asserts;
using namespace nu::si::literals;


nu::AutoTest t_1 ("VirtualModem attenuates off-frequency signals before sensitivity check", []{
	auto system = AntennaSystem (1_s);
	auto antenna_model = WhipAntennaModel (1_m);
	auto antenna = sim::Antenna (MassMoments<BodyCOM>::zero(), antenna_model, system);
	auto loop = TestProcessingLoop (1_ms);
	auto modem = VirtualModem ({
		.module_parameters = {
			.loop = loop,
			.instance = "modem",
		},
		.antenna = antenna,
		.frequency = 100_MHz,
		.bandwidth = 20_kHz,
		.bitrate = 9'600_Hz,
		.tx_buffer_size = 16'384,
		.tx_power = 1_W,
		.rx_sensitivity = 0.6_W,
	});
	auto cycle = TestCycle();

	antenna.receive_signal ({
		.power = 1_W,
		.frequency = 100_MHz,
		.bandwidth = 25_kHz,
		.payload = "on-frequency",
	});
	antenna.receive_signal ({
		.power = 1_W,
		.frequency = 100_MHz + 10_kHz,
		.bandwidth = 25_kHz,
		.payload = "off-frequency",
	});

	modem.process (cycle);

	test_asserts::verify_equal ("Only on-frequency payload passes modem filter", modem.receive.value_or (""), "on-frequency");
	test_asserts::verify_equal ("RSSI reports strongest filtered signal", modem.rssi.value_or (0_W), 1_W);
});


nu::AutoTest t_2 ("VirtualModem intentionally ignores transmit bandwidth in the simplified filter model", []{
	auto system = AntennaSystem (1_s);
	auto antenna_model = WhipAntennaModel (1_m);
	auto antenna = sim::Antenna (MassMoments<BodyCOM>::zero(), antenna_model, system);
	auto loop = TestProcessingLoop (1_ms);
	auto modem = VirtualModem ({
		.module_parameters = {
			.loop = loop,
			.instance = "modem",
		},
		.antenna = antenna,
		.frequency = 100_MHz,
		.bandwidth = 20_kHz,
		.bitrate = 9'600_Hz,
		.tx_buffer_size = 16'384,
		.tx_power = 1_W,
		.rx_sensitivity = 0.4_W,
	});
	auto cycle = TestCycle();

	antenna.receive_signal ({
		.power = 1_W,
		.frequency = 100_MHz + 10_kHz,
		.bandwidth = 5_kHz,
		.payload = "narrow",
	});
	antenna.receive_signal ({
		.power = 1_W,
		.frequency = 100_MHz + 10_kHz,
		.bandwidth = 200_kHz,
		.payload = "wide",
	});

	modem.process (cycle);

	test_asserts::verify_equal ("Signal bandwidth does not change simplified filter decision", modem.receive.value_or (""), "narrowwide");
	test_asserts::verify_equal_with_epsilon ("RSSI uses Gaussian attenuation from frequency offset", modem.rssi.value_or (0_W), 0.5_W, 1e-12_W);
});


nu::AutoTest t_2b ("VirtualModem receive output is per-cycle and does not retain stale payloads", []{
	auto system = AntennaSystem (1_s);
	auto antenna_model = WhipAntennaModel (1_m);
	auto antenna = sim::Antenna (MassMoments<BodyCOM>::zero(), antenna_model, system);
	auto loop = TestProcessingLoop (1_ms);
	auto modem = VirtualModem ({
		.module_parameters = {
			.loop = loop,
			.instance = "modem",
		},
		.antenna = antenna,
		.frequency = 100_MHz,
		.bandwidth = 20_kHz,
		.bitrate = 9'600_Hz,
		.tx_buffer_size = 16'384,
		.tx_power = 1_W,
		.rx_sensitivity = 0.4_W,
	});
	auto cycle = TestCycle();

	antenna.receive_signal ({
		.power = 1_W,
		.frequency = 100_MHz,
		.bandwidth = 25_kHz,
		.payload = "first",
	});

	modem.process (cycle);
	test_asserts::verify_equal ("First receive cycle forwards the current payload", modem.receive.value_or (""), "first");

	cycle += 1_ms;
	modem.process (cycle);
	test_asserts::verify ("Idle receive cycle clears the output socket", modem.receive.is_nil());

	antenna.receive_signal ({
		.power = 1_W,
		.frequency = 100_MHz,
		.bandwidth = 25_kHz,
		.payload = "second",
	});

	cycle += 1_ms;
	modem.process (cycle);
	test_asserts::verify_equal ("Later receive cycle does not concatenate stale payloads", modem.receive.value_or (""), "second");
});


nu::AutoTest t_3 ("VirtualModem rejects non-positive tuning parameters", []{
	auto system = AntennaSystem (1_s);
	auto antenna_model = WhipAntennaModel (1_m);
	auto antenna = sim::Antenna (MassMoments<BodyCOM>::zero(), antenna_model, system);
	auto loop = TestProcessingLoop (1_ms);

	test_asserts::verify_throws<nu::InvalidArgument> ("Zero modem frequency should be rejected", [&]{
		[[maybe_unused]] auto const modem = VirtualModem ({
			.module_parameters = {
				.loop = loop,
				.instance = "modem-zero-frequency",
			},
			.antenna = antenna,
			.frequency = 0_Hz,
			.bandwidth = 20_kHz,
			.bitrate = 9'600_Hz,
			.tx_buffer_size = 16'384,
			.tx_power = 1_W,
			.rx_sensitivity = 1_W,
		});
	});

	test_asserts::verify_throws<nu::InvalidArgument> ("Zero modem bandwidth should be rejected", [&]{
		[[maybe_unused]] auto const modem = VirtualModem ({
			.module_parameters = {
				.loop = loop,
				.instance = "modem-zero-bandwidth",
			},
			.antenna = antenna,
			.frequency = 100_MHz,
			.bandwidth = 0_Hz,
			.bitrate = 9'600_Hz,
			.tx_buffer_size = 16'384,
			.tx_power = 1_W,
			.rx_sensitivity = 1_W,
		});
	});

	test_asserts::verify_throws<nu::InvalidArgument> ("Zero modem bitrate should be rejected", [&]{
		[[maybe_unused]] auto const modem = VirtualModem ({
			.module_parameters = {
				.loop = loop,
				.instance = "modem-zero-bitrate",
			},
			.antenna = antenna,
			.frequency = 100_MHz,
			.bandwidth = 20_kHz,
			.bitrate = 0_Hz,
			.tx_buffer_size = 16'384,
			.tx_power = 1_W,
			.rx_sensitivity = 1_W,
		});
	});
});


nu::AutoTest t_4 ("VirtualModem transmits queued payload in bitrate-limited chunks", []{
	auto system = AntennaSystem (1_s);
	auto antenna_model = WhipAntennaModel (1_m);
	auto tx_antenna = sim::Antenna (MassMoments<BodyCOM>::zero(), antenna_model, system);
	auto rx_antenna = sim::Antenna (MassMoments<BodyCOM>::zero(), antenna_model, system);
	rx_antenna.set_recording_enabled (true);
	auto loop = TestProcessingLoop (1_ms);
	auto modem = VirtualModem ({
		.module_parameters = {
			.loop = loop,
			.instance = "modem",
		},
		.antenna = tx_antenna,
		.frequency = 100_MHz,
		.bandwidth = 20_kHz,
		.bitrate = 8_kHz,
		.tx_buffer_size = 16'384,
		.tx_power = 1_W,
		.rx_sensitivity = 0.4_W,
	});
	auto cycle = TestCycle();

	modem.send << "hello";

	for (auto const expected_chunk: { "h", "e", "l", "l", "o" })
	{
		cycle += 1_ms;
		modem.send.fetch (cycle);
		modem.process (cycle);
		system.process (cycle.update_time());

		auto const received = rx_antenna.take_recorded_signals();
		test_asserts::verify_equal ("Exactly one chunk should be emitted per cycle", received.size(), 1uz);
		test_asserts::verify_equal ("Chunk payload should follow bitrate-limited segmentation", received.front().payload, expected_chunk);
	}

	test_asserts::verify_equal ("No bytes should be dropped when buffer has space", modem.tx_bytes_dropped.value_or (0), 0u);
});


nu::AutoTest t_5 ("VirtualModem drops bytes that do not fit in the TX buffer", []{
	auto system = AntennaSystem (1_s);
	auto antenna_model = WhipAntennaModel (1_m);
	auto tx_antenna = sim::Antenna (MassMoments<BodyCOM>::zero(), antenna_model, system);
	auto rx_antenna = sim::Antenna (MassMoments<BodyCOM>::zero(), antenna_model, system);
	rx_antenna.set_recording_enabled (true);
	auto loop = TestProcessingLoop (1_ms);
	auto modem = VirtualModem ({
		.module_parameters = {
			.loop = loop,
			.instance = "modem",
		},
		.antenna = tx_antenna,
		.frequency = 100_MHz,
		.bandwidth = 20_kHz,
		.bitrate = 1_Hz,
		.tx_buffer_size = 4,
		.tx_power = 1_W,
		.rx_sensitivity = 0.4_W,
	});
	auto cycle = TestCycle();

	modem.send << "abcdef";

	cycle += 1_ms;
	modem.send.fetch (cycle);
	modem.process (cycle);
	system.process (cycle.update_time());

	test_asserts::verify_equal ("Bytes beyond TX buffer capacity should be counted as dropped", modem.tx_bytes_dropped.value_or (0), 2u);
	test_asserts::verify_equal ("No chunk should be emitted before enough bitrate budget is available", rx_antenna.take_recorded_signals().size(), 0uz);

	cycle += 32_s;
	modem.send.fetch (cycle);
	modem.process (cycle);
	system.process (cycle.update_time());

	auto const received = rx_antenna.take_recorded_signals();
	test_asserts::verify_equal ("Buffered bytes should still transmit later", received.size(), 1uz);
	test_asserts::verify_equal ("Only bytes that fit in the TX buffer should be sent", received.front().payload, "abcd");
});

} // namespace
} // namespace xf::test
