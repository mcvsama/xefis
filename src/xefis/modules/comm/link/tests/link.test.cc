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
#include <xefis/core/sockets/tests/test_cycle.h>
#include <xefis/core/sockets/module_in.h>
#include <xefis/core/sockets/module_out.h>
#include <xefis/core/module.h>
#include <xefis/modules/comm/link/input_link.h>
#include <xefis/modules/comm/link/link_protocol.h>
#include <xefis/modules/comm/link/output_link.h>
#include <xefis/modules/comm/xle_transceiver.h>
#include <xefis/test/test_processing_loop.h>

// Neutrino:
#include <neutrino/test/auto_test.h>
#include <neutrino/string.h>

// Standard:
#include <cstddef>


namespace xf::test {
namespace {

namespace xle = xf::crypto::xle;

auto g_logger_output	= xf::LoggerOutput (std::clog);
auto g_logger			= xf::Logger (g_logger_output);

auto constexpr kFallbackBool	= true;
auto constexpr kFallbackInt		= 12UL;


template<template<class> class SocketType>
	class GroundToAirData: public Module
	{
	  public:
		SocketType<std::string>		handshake_request		{ this, "handshake_request" };
		SocketType<std::string>		string_prop				{ this, "string_prop" };
		SocketType<std::string>		string_prop_r			{ this, "string_prop_r" };
		SocketType<std::string>		string_nil				{ this, "string_nil" };
		SocketType<std::string>		string_nil_trunc		{ this, "string_nil_trunc" };
		SocketType<std::string>		string_trunc			{ this, "string_trunc" };
		SocketType<std::string>		string_multiblock		{ this, "string_multiblock" };
		SocketType<std::string>		string_empty			{ this, "string_empty" };
		SocketType<si::Angle>		nil_si_prop				{ this, "nil" };
		SocketType<si::Angle>		angle_prop				{ this, "angle" };
		SocketType<si::Angle>		angle_prop_r			{ this, "angle_r" };
		SocketType<si::Velocity>	velocity_prop			{ this, "velocity" };
		SocketType<si::Velocity>	velocity_prop_r			{ this, "velocity_r" };
		SocketType<si::Velocity>	velocity_prop_offset	{ this, "velocity_prop_offset" };
		SocketType<si::Velocity>	velocity_prop_offset_r	{ this, "velocity_prop_offset_r" };
		SocketType<bool>			bool_prop				{ this, "bool" };
		SocketType<bool>			bool_prop_r				{ this, "bool_r" };
		SocketType<int64_t>			int_prop				{ this, "int" };
		SocketType<int64_t>			int_prop_r				{ this, "int_r" };
		SocketType<uint64_t>		uint_prop				{ this, "uint" };
		SocketType<uint64_t>		uint_prop_r				{ this, "uint_r" };
		SocketType<int64_t>			dummy					{ this, "dummy" };

	  public:
		// Ctor
		using Module::Module;

		void
		fetch_all (Cycle const& cycle)
		{
			std::initializer_list<BasicSocket*> const sockets = {
				&handshake_request,
				&string_prop,
				&string_prop_r,
				&string_nil,
				&string_nil_trunc,
				&string_trunc,
				&string_multiblock,
				&string_empty,
				&nil_si_prop,
				&angle_prop,
				&angle_prop_r,
				&velocity_prop,
				&velocity_prop_r,
				&velocity_prop_offset,
				&velocity_prop_offset_r,
				&bool_prop,
				&bool_prop_r,
				&int_prop,
				&int_prop_r,
				&uint_prop,
				&uint_prop_r,
				&dummy,
			};

			for (auto* socket: sockets)
				socket->fetch (cycle);
		}
	};


template<template<class> class SocketType>
	class AirToGroundData: public Module
	{
	  public:
		SocketType<std::string>		handshake_response		{ this, "handshake_response" };
		SocketType<int>				int_prop				{ this, "int_prop" };

	  public:
		// Ctor
		using Module::Module;

		void
		fetch_all (Cycle const& cycle)
		{
			std::initializer_list<BasicSocket*> const sockets = {
				&handshake_response,
				&int_prop,
			};

			for (auto* socket: sockets)
				socket->fetch (cycle);
		}
	};


class GroundToAirLinkProtocol: public LinkProtocol
{
  public:
	// Ctor
	template<class IO>
		explicit
		GroundToAirLinkProtocol (IO& io, xle::Transceiver* transceiver = nullptr):
			LinkProtocol ({
				// Must be first envelope (its data will be messed with to test
				// if it's discarded in such case):
				envelope ({
					.unique_prefix	= { 0x00, 0x01 },
					.transceiver	= transceiver,
					.packets		= {
						signature ({
							.nonce_bytes		= 9,
							.signature_bytes	= 12,
							.key				= { 0x88, 0x99, 0xaa, 0xbb },
							.packets			= {
								socket<30> (io.string_prop,				{ .retained = false }),
								socket<15> (io.string_prop_r,			{ .retained = true }),
								socket<10> (io.string_nil,				{ .retained = true }),
								socket<10> (io.string_nil_trunc,		{ .retained = true,		.truncate = true }),
								socket<4> (io.string_trunc,				{ .retained = true,		.truncate = true }),
								socket<5> (io.string_multiblock,		{ .retained = true }),
								socket<30> (io.string_empty,			{ .retained = true }),
								socket<8> (io.nil_si_prop),
								socket<8> (io.angle_prop),
								socket<8> (io.angle_prop_r,				{ .retained = true }),
								socket<2> (io.velocity_prop,			{ .retained = false }),
								socket<2> (io.velocity_prop_r,			{ .retained = true }),
								socket<2> (io.velocity_prop_offset,		{ .retained = false,	.offset = 1000_kph }),
								socket<2> (io.velocity_prop_offset_r,	{ .retained = true,		.offset = 1000_kph }),
								socket<2> (io.int_prop,					{ .retained = false,	.value_if_nil = 0L }),
								socket<2> (io.int_prop_r,				{ .retained = true,		.value_if_nil = 0L }),
							},
						}),
					},
				}),
				// XLE handshake envelope:
				envelope ({
					.unique_prefix	= { 0x00, 0x00 },
					.send_predicate	= [&io] { return io.handshake_request.valid(); },
					.packets		= {
						// Always good to have at least basic checksum:
						signature ({
							.nonce_bytes		= 0,
							.signature_bytes	= 4,
							.key				= { 0xaa, 0xaa },
							.packets			= {
								socket<256> (io.handshake_request, { .retained = false }),
							},
						}),
					},
				}),
				// This must be last on the list of envelopes
				// that are transmitted immediately (not delayed):
				envelope ({
					.unique_prefix	= { 0x00, 0x02 },
					.packets		= {
						signature ({
							.nonce_bytes		= 8,
							.signature_bytes	= 8,
							.key				= { 0x55, 0x37, 0x12, 0xf9 },
							.packets			= {
								bitfield ({
									bitfield_socket (io.bool_prop,		{ .retained = false,	.value_if_nil = kFallbackBool }),
									bitfield_socket (io.bool_prop_r,	{ .retained = true,		.value_if_nil = kFallbackBool }),
									bitfield_socket (io.uint_prop,		{ .bits = 4,			.retained = false,	.value_if_nil = kFallbackInt }),
									bitfield_socket (io.uint_prop_r,	{ .bits = 4,			.retained = true,	.value_if_nil = kFallbackInt }),
								}),
							},
						}),
					},
				}),
				// Envelope that will not be transmitted until some time
				// passes:
				envelope ({
					.unique_prefix	= { 0x00, 0x03 },
					.send_every		= 10,
					.send_offset	= 8,
					.packets		= {
						socket<4> (io.dummy, { .retained = false, .value_if_nil = 0L }),
					},
				}),
			})
		{ }
};


class AirToGroundLinkProtocol: public LinkProtocol
{
  public:
	// Ctor
	template<class IO>
		explicit
		AirToGroundLinkProtocol (IO& io, xle::Transceiver* transceiver = nullptr):
			LinkProtocol ({
				// XLE handshake envelope:
				envelope ({
					.unique_prefix	= { 0xff, 0x00 },
					.send_predicate	= [&io] { return io.handshake_response.valid(); },
					.packets		= {
						// Always good to have at least basic checksum:
						signature ({
							.nonce_bytes		= 0,
							.signature_bytes	= 4,
							.key				= { 0xbb, 0xbb },
							.packets			= {
								socket<256> (io.handshake_response, { .retained = false }),
							},
						}),
					},
				}),
				// Normal data envelope:
				envelope ({
					.unique_prefix	= { 0xff, 0x01 },
					.transceiver	= transceiver,
					.packets		= {
						socket<4> (io.int_prop),
					},
				}),
			})
		{ }
};


using Ground_Tx_Data = GroundToAirData<xf::ModuleIn>;
using Ground_Rx_Data = AirToGroundData<xf::ModuleOut>;
using Air_Tx_Data = AirToGroundData<xf::ModuleIn>;
using Air_Rx_Data = GroundToAirData<xf::ModuleOut>;

auto constinit crypto_params = xle::Transceiver::CryptoParams {
	.master_signature_key		= { 0x00, 0x01, 0x02, 0x03 },
	.slave_signature_key		= { 0x0c, 0x0d, 0x0e, 0x0f },
	.authentication_secret		= { 0x01 },
	.data_encryption_secret		= { 0x02 },
	.seq_num_encryption_secret	= { 0x03 },
	.hmac_size					= 16,
	.max_time_difference		= 60_s,
};


xle::MasterTransceiver
get_ground_transceiver (xf::ProcessingLoop& loop)
{
	return xle::MasterTransceiver (loop, crypto_params, g_logger.with_context ("ground-transceiver"), "ground/transceiver");
}


xle::SlaveTransceiver
get_air_transceiver (xf::ProcessingLoop& loop)
{
	return xle::SlaveTransceiver (loop, crypto_params, {}, g_logger.with_context ("air-transceiver"), "air/transceiver");
}


void transmit (LinkProtocol& tx_protocol, LinkProtocol& rx_protocol)
{
	Blob blob;
	tx_protocol.produce (blob, g_logger);
	auto end = rx_protocol.consume (blob.begin(), blob.end(), nullptr, nullptr, nullptr, g_logger);

	test_asserts::verify ("rx_protocol ate all input bytes", end == blob.end());
}


AutoTest t1 ("modules/io/link: protocol: valid data transmission", []{
	TestProcessingLoop loop (0.1_s);
	Ground_Tx_Data tx (loop);
	Air_Rx_Data rx (loop);
	GroundToAirLinkProtocol tx_protocol (tx);
	GroundToAirLinkProtocol rx_protocol (rx);
	TestCycle cycle;

	auto test = [&] {
		cycle += 1_s;
		tx.fetch_all (cycle);
		transmit (tx_protocol, rx_protocol);
		test_asserts::verify ("string_prop transmitted properly", rx.string_prop == tx.string_prop);
		test_asserts::verify ("string_prop_r transmitted properly", rx.string_prop_r == tx.string_prop_r);
		test_asserts::verify ("string_nil transmitted properly", !rx.string_nil);
		test_asserts::verify ("string_nil_trunc transmitted properly", !rx.string_nil_trunc);
		test_asserts::verify ("string_trunc transmitted (and truncated) as expected", rx.string_trunc.value_or ("nil!") == "1234");
		test_asserts::verify ("string_empty transmitted properly", rx.string_empty == tx.string_empty);
		test_asserts::verify ("nil_si_prop transmitted properly", rx.nil_si_prop == tx.nil_si_prop);
		test_asserts::verify ("angle_prop transmitted properly (optional() version)", rx.angle_prop == tx.angle_prop);
		test_asserts::verify ("angle_prop transmitted properly", *rx.angle_prop == *tx.angle_prop);
		test_asserts::verify_equal_with_epsilon ("velocity transmitted properly", *rx.velocity_prop, *tx.velocity_prop, 0.1_kph);
		test_asserts::verify_equal_with_epsilon ("velocity prop with offset transmitted properly", *rx.velocity_prop_offset, *tx.velocity_prop_offset, 0.1_mps);
		test_asserts::verify ("bool_prop transmitted properly", *rx.bool_prop == *tx.bool_prop);
		test_asserts::verify ("int_prop transmitted properly", *rx.int_prop == *tx.int_prop);
		test_asserts::verify ("uint_prop transmitted properly", *rx.uint_prop == *tx.uint_prop);
	};

	auto multiblock_test = [&] {
		for (auto i = 0u; i < 4u; ++i)
		{
			cycle += 1_s;
			tx.fetch_all (cycle);
			transmit (tx_protocol, rx_protocol);
		}

		test_asserts::verify ("string_multiblock transmitted properly", rx.string_multiblock == tx.string_multiblock);
	};

	tx.string_prop << "123456789012345678901234567890";
	tx.string_prop_r << "retained string";
	tx.string_nil << xf::no_data_source;
	tx.string_nil_trunc << xf::no_data_source;
	tx.string_trunc << "1234567890";
	tx.string_multiblock << "12345678901234567"; // socket<5> uses 5-byte buffer, so this should be transmitted within 4 cycles.
	tx.string_empty << "";
	tx.angle_prop << 1.99_rad;
	tx.velocity_prop << 101_kph;
	tx.velocity_prop_offset << 101_kph;
	tx.bool_prop << true;
	tx.int_prop << -2;
	tx.uint_prop << 3u;
	test();
	multiblock_test();

	for (auto const& angle: { -12_rad, 0_rad, 0.99_rad, 1.59_rad, 300_rad })
	{
		tx.angle_prop << angle;
		test();
	}

	for (auto b: { false, true })
	{
		tx.bool_prop << b;
		test();
	}

	for (auto i: { -9, -7, -5, -3, -2, -1, 0, 1, 2, 3, 5, 7, 9 })
	{
		tx.int_prop << i;
		test();
	}

	for (unsigned int i: { 0u, 1u, 2u, 3u, 4u, 5u, 6u, 7u, 8u, 9u, 10u, 11u, 12u, 13u, 14u, 15u })
	{
		tx.uint_prop << i;
		test();
	}
});


AutoTest t2 ("modules/io/link: protocol: nils and out-of range values transmission", []{
	TestProcessingLoop loop (0.1_s);
	Ground_Tx_Data tx (loop);
	Air_Rx_Data rx (loop);
	GroundToAirLinkProtocol tx_protocol (tx);
	GroundToAirLinkProtocol rx_protocol (rx);
	TestCycle cycle;

	// Test bit-bool:

	tx.bool_prop << true;
	tx.fetch_all (cycle += 1_s);
	transmit (tx_protocol, rx_protocol);
	test_asserts::verify ("bit-bool 1 is transmitted properly", *rx.bool_prop == *tx.bool_prop);

	tx.bool_prop << false;
	tx.fetch_all (cycle += 1_s);
	transmit (tx_protocol, rx_protocol);
	test_asserts::verify ("bit-bool 0 is transmitted properly", *rx.bool_prop == *tx.bool_prop);

	tx.bool_prop << xf::no_data_source;
	tx.fetch_all (cycle += 1_s);
	transmit (tx_protocol, rx_protocol);
	test_asserts::verify ("nil bit-bool set to fall-back value", *rx.bool_prop == kFallbackBool);

	// Test bit-int:

	tx.uint_prop << 11u;
	tx.fetch_all (cycle += 1_s);
	transmit (tx_protocol, rx_protocol);
	test_asserts::verify ("bit-int 11 transmitted properly", *rx.uint_prop == *tx.uint_prop);

	tx.uint_prop << 17u;
	tx.fetch_all (cycle += 1_s);
	transmit (tx_protocol, rx_protocol);
	test_asserts::verify ("out-of-range bit-int set to fall-back value", *rx.uint_prop == kFallbackInt);

	tx.uint_prop << 15u;
	tx.fetch_all (cycle += 1_s);
	transmit (tx_protocol, rx_protocol);
	test_asserts::verify ("bit-int 15 transmitted properly", *rx.uint_prop == *tx.uint_prop);

	tx.uint_prop << xf::no_data_source;
	tx.fetch_all (cycle += 1_s);
	transmit (tx_protocol, rx_protocol);
	test_asserts::verify ("nil bit-int set to fall-back value", *rx.uint_prop == kFallbackInt);
});


AutoTest t3 ("modules/io/link: protocol: offsets increase precision", []{
	TestProcessingLoop loop (0.1_s);
	Ground_Tx_Data tx (loop);
	Air_Rx_Data rx (loop);
	GroundToAirLinkProtocol tx_protocol (tx);
	GroundToAirLinkProtocol rx_protocol (rx);
	TestCycle cycle;

	tx.velocity_prop << 1001_kph;
	tx.velocity_prop_offset << 1001_kph;
	tx.fetch_all (cycle += 1_s);
	transmit (tx_protocol, rx_protocol);
	auto delta = si::abs (*rx.velocity_prop - *tx.velocity_prop);
	auto delta_with_offset = si::abs (*rx.velocity_prop_offset - *tx.velocity_prop_offset);
	test_asserts::verify ("offsets increase precision", delta_with_offset < delta);
});


AutoTest t4 ("modules/io/link: protocol: invalid data transmission (wrong signature)", []{
	// This tests signature verification and checks if values are retained or not, according to the protocol.

	TestProcessingLoop loop (0.1_s);
	Ground_Tx_Data tx (loop);
	Air_Rx_Data rx (loop);
	GroundToAirLinkProtocol tx_protocol (tx);
	GroundToAirLinkProtocol rx_protocol (rx);
	TestCycle cycle;

	// Transmit correctly first:
	tx.string_prop << "non-retained string";
	tx.string_prop_r << "retained string";
	tx.nil_si_prop << xf::no_data_source;
	tx.angle_prop << 15_rad;
	tx.angle_prop_r << 15_rad;
	tx.velocity_prop << 100_mps;
	tx.velocity_prop_r << 100_mps;
	tx.velocity_prop_offset << 102_mps;
	tx.velocity_prop_offset_r << 102_mps;
	tx.bool_prop << false;
	tx.bool_prop_r << false;
	tx.int_prop << -5;
	tx.int_prop_r << -5;
	tx.uint_prop << 15u;
	tx.uint_prop_r << 15u;
	tx.fetch_all (cycle += 1_s);

	Blob blob;
	tx_protocol.produce (blob, g_logger);
	rx_protocol.consume (blob.begin(), blob.end(), nullptr, nullptr, nullptr, g_logger);

	// Transmit invalid data:
	tx.string_prop << "invalid string";
	tx.string_prop_r << "invalid retained string";
	tx.nil_si_prop << 1_rad;
	tx.angle_prop << 16_rad;
	tx.angle_prop_r << 16_rad;
	tx.velocity_prop << 101_mps;
	tx.velocity_prop_r << 101_mps;
	tx.velocity_prop_offset << 103_mps;
	tx.velocity_prop_offset_r << 103_mps;
	tx.bool_prop << true;
	tx.bool_prop_r << true;
	tx.int_prop << -3;
	tx.int_prop_r << -3;
	tx.uint_prop << 12u;
	tx.uint_prop_r << 12u;
	tx.fetch_all (cycle += 1_s);

	blob.clear();
	tx_protocol.produce (blob, g_logger);
	test_asserts::verify ("blob is long enough", blob.size() >= 16);
	// Mess with both messages:
	blob[12] = 0x00;
	blob[13] = 0xff;
	blob[14] = 0x00;
	blob[15] = 0xff;
	// Careful not to touch the last envelope, which shouldn't be sent in a first second:
	blob[blob.size() - 6] = 0xff;
	blob[blob.size() - 5] = 0x00;
	blob[blob.size() - 4] = 0xff;
	rx_protocol.consume (blob.begin(), blob.end(), nullptr, nullptr, nullptr, g_logger);

	// Test that values weren't changed during last invalid transmission:
	test_asserts::verify ("string_prop didn't change", rx.string_prop.value_or ("nil!") == "non-retained string");
	test_asserts::verify ("nil_si_prop didn't change", !rx.nil_si_prop);
	test_asserts::verify ("angle_prop is didn't change", *rx.angle_prop == 15_rad);
	test_asserts::verify_equal_with_epsilon ("velocity_prop didn't change", *rx.velocity_prop, 100_mps, 0.1_mps);
	test_asserts::verify_equal_with_epsilon ("velocity_prop_offset didn't change", *rx.velocity_prop_offset, 102_mps, 0.1_mps);
	test_asserts::verify ("int_prop didn't change", *rx.int_prop == -5);
	test_asserts::verify ("bool_prop didn't change", *rx.bool_prop == false);
	test_asserts::verify ("uint_prop didn't change", *rx.uint_prop == 15u);

	// Test fail-safe and retaining of original values:
	rx_protocol.failsafe();
	test_asserts::verify ("string_prop is nil", !rx.string_prop);
	test_asserts::verify ("string_prop_r is retained", rx.string_prop_r.value_or ("nil!") == "retained string");
	test_asserts::verify ("nil_si_prop is nil", !rx.nil_si_prop);
	test_asserts::verify ("angle_prop is nil", !rx.angle_prop);
	test_asserts::verify ("angle_prop_r is retained", *rx.angle_prop_r == 15_rad);
	test_asserts::verify ("velocity_prop is nil", !rx.velocity_prop);
	test_asserts::verify_equal_with_epsilon ("velocity_prop_r is retained", *rx.velocity_prop_r, 100_mps, 0.1_mps);
	test_asserts::verify ("velocity_prop_offset is nil", !rx.velocity_prop_offset);
	test_asserts::verify_equal_with_epsilon ("velocity_prop_offset_r is retained", *rx.velocity_prop_offset_r, 102_mps, 0.1_mps);
	test_asserts::verify ("bool_prop is nil", !rx.bool_prop);
	test_asserts::verify ("bool_prop_r is retained", *rx.bool_prop_r == false);
	test_asserts::verify ("int_prop is nil", !rx.int_prop);
	test_asserts::verify ("int_prop_r is retained", *rx.int_prop_r == -5);
	test_asserts::verify ("uint_prop is nil", !rx.uint_prop);
	test_asserts::verify ("uint_prop_r is retained", *rx.uint_prop_r == 15u);
});


AutoTest t5 ("modules/io/link: protocol: send-every/send-offset", []{
	// The third envelope should be sent every two packets, starting from packet with index 1.

	TestProcessingLoop loop (0.1_s);
	Ground_Tx_Data tx (loop);
	Air_Rx_Data rx (loop);
	GroundToAirLinkProtocol tx_protocol (tx);
	GroundToAirLinkProtocol rx_protocol (rx);
	TestCycle cycle;

	constexpr int64_t kFirstInt = 11223344;
	constexpr int64_t kSecondInt = 66775544;

	tx.dummy << kFirstInt;
	tx.fetch_all (cycle += 1_s);
	// According to envelope settings, one need to transmit 7 times before the envelope will be
	// actually sent:
	for (size_t i = 0; i < 8; ++i)
	{
		transmit (tx_protocol, rx_protocol);
		test_asserts::verify ("last envelope not sent in " + std::to_string (i) + "-th transmission", !rx.dummy);
	}

	// 8th transmission (every 10th envelope is sent, but offset is 8 and this is the first time):
	transmit (tx_protocol, rx_protocol);
	test_asserts::verify ("last envelope sent for the first time", *rx.dummy == *tx.dummy);

	tx.dummy << kSecondInt;
	tx.fetch_all (cycle += 1_s);

	// Next 9 transmissions should not send last envelope (it's sent every 10th):
	for (size_t i = 0; i < 9; ++i)
	{
		transmit (tx_protocol, rx_protocol);
		test_asserts::verify ("last envelope not sent in subsequent transmissions", *rx.dummy == kFirstInt);
	}

	// 10th transmission updates the socket:
	transmit (tx_protocol, rx_protocol);
	test_asserts::verify ("last envelope sent for the second time", *rx.dummy == kSecondInt);
});


AutoTest t6 ("modules/io/link: protocol: encrypted channel works", []{
	TestProcessingLoop loop (0.1_s);
	Ground_Tx_Data ground_tx_data (loop);
	Ground_Rx_Data ground_rx_data (loop);
	Air_Tx_Data air_tx_data (loop);
	Air_Rx_Data air_rx_data (loop);

	auto ground_transceiver = get_ground_transceiver (loop);
	auto air_transceiver = get_air_transceiver (loop);

	auto ground_tx_protocol = std::make_unique<GroundToAirLinkProtocol> (ground_tx_data, &ground_transceiver);
	auto ground_rx_protocol = std::make_unique<AirToGroundLinkProtocol> (ground_rx_data, &ground_transceiver);
	auto air_tx_protocol = std::make_unique<AirToGroundLinkProtocol> (air_tx_data, &air_transceiver);
	auto air_rx_protocol = std::make_unique<GroundToAirLinkProtocol> (air_rx_data, &air_transceiver);

	auto ground_tx_link = OutputLink (loop, std::move (ground_tx_protocol), 30_Hz, g_logger.with_context ("ground-tx-link"), "ground/tx-link");
	auto ground_rx_link = InputLink (loop, std::move (ground_rx_protocol), {}, g_logger.with_context ("ground-rx-link"), "ground/rx-link");
	auto air_tx_link = OutputLink (loop, std::move (air_tx_protocol), 30_Hz, g_logger.with_context ("air-tx-link"), "air/tx-link");
	auto air_rx_link = InputLink (loop, std::move (air_rx_protocol), {}, g_logger.with_context ("air-rx-link"), "air/rx-link");

	ground_tx_data.handshake_request << ground_transceiver.handshake_request;
	ground_transceiver.handshake_response << ground_rx_data.handshake_response;

	air_transceiver.handshake_request << air_rx_data.handshake_request;
	air_tx_data.handshake_response << air_transceiver.handshake_response;

	air_rx_link.link_input << ground_tx_link.link_output;
	ground_rx_link.link_input << air_tx_link.link_output;

	auto [session_prepared, session_activated] = ground_transceiver.start_handshake();
	auto constexpr kMaxCycles = 6u;

	for (size_t cycles = 0; !ready (session_prepared) && !ready (session_activated); ++cycles)
	{
		test_asserts::verify (std::format ("handshake completes in {} cycles", cycles), cycles < kMaxCycles);
		loop.next_cycle();
	}

	// Test data transmission in both directions:
	loop.next_cycles (5);

	// Ground to air encryption:
	{
		auto const u = to_blob ("hello!");
		auto const e = ground_transceiver.encrypt_packet (u);
		auto const d = air_transceiver.decrypt_packet (e);

		test_asserts::verify ("encryption works from ground to air using transceivers directly", d == u);
	}

	// Air to ground encryption:
	{
		auto const u = to_blob ("hello back!");
		auto const e = air_transceiver.encrypt_packet (u);
		auto const d = ground_transceiver.decrypt_packet (e);

		test_asserts::verify ("encryption works from air to ground using transceivers directly", d == u);
	}

	// Test data transmission in both directions, after directly using
	// transceivers:
	loop.next_cycles (5);

	ground_tx_data.string_prop << "abc123";
	loop.next_cycles (1);
	test_asserts::verify ("data transmitted properly", ground_tx_data.string_prop == air_rx_data.string_prop);
});

} // namespace
} // namespace xf::test

