/* vim:ts=4
 *
 * Copyleft 2017  Michał Gawron
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
#include <xefis/modules/comm/link/link_decoder.h>
#include <xefis/modules/comm/link/link_encoder.h>
#include <xefis/modules/comm/link/helpers.h>
#include <xefis/modules/comm/link/link_protocol.h>
#include <xefis/modules/comm/xle_secure_channel.h>
#include <xefis/test/test_processing_loop.h>

// Neutrino:
#include <neutrino/test/auto_test.h>
#include <neutrino/string.h>

// Standard:
#include <cstddef>


namespace xf::test {
namespace {

namespace test_asserts = nu::test_asserts;
namespace xle = crypto::xle;

auto g_logger_output	= nu::LoggerOutput (std::clog);
auto g_logger			= nu::Logger (g_logger_output);

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
	template<std::derived_from<Module> IO>
		explicit
		GroundToAirLinkProtocol (IO& io, xle::SecureChannel* secure_channel = nullptr):
			LinkProtocol ({
				// Must be first envelope (its data will be messed with to test
				// if it's discarded in such case):
				envelope ({
					.name			= "data 1",
					.unique_prefix	= { 0x00, 0x01 },
					.secure_channel	= secure_channel,
					.packets		= {
						signature ({
							.name				= "data 1 signature",
							.nonce_bytes		= 9,
							.signature_bytes	= 12,
							.key				= { 0x88, 0x99, 0xaa, 0xbb },
							.packets			= {
								socket<30> (io.string_prop,				{ .retained = false,	.transfer = StringTransfer::Segmented }),
								socket<15> (io.string_prop_r,			{ .retained = true,		.transfer = StringTransfer::Segmented }),
								socket<10> (io.string_nil,				{ .retained = true,		.transfer = StringTransfer::Segmented }),
								socket<10> (io.string_nil_trunc,		{ .retained = true,		.transfer = StringTransfer::Truncated }),
								socket<4> (io.string_trunc,				{ .retained = true,		.transfer = StringTransfer::Truncated }),
								socket<5> (io.string_multiblock,		{ .retained = true,		.transfer = StringTransfer::Segmented }),
								socket<30> (io.string_empty,			{ .retained = true,		.transfer = StringTransfer::Segmented }),
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
					.name			= "XLE handshake",
					.unique_prefix	= { 0x00, 0x00 },
					.send_predicate	= [&io] { return io.handshake_request.valid(); },
					.packets		= {
						// Always good to have at least basic checksum:
						signature ({
							.name				= "XLE handshake signature",
							.nonce_bytes		= 0,
							.signature_bytes	= 4,
							.key				= { 0xaa, 0xaa },
							.packets			= {
								socket<256> (io.handshake_request, { .retained = false, .transfer = StringTransfer::Segmented }),
							},
						}),
					},
				}),
				// This must be last on the list of envelopes
				// that are transmitted immediately (not delayed):
				envelope ({
					.name			= "data 2",
					.unique_prefix	= { 0x00, 0x02 },
					.packets		= {
						signature ({
							.name				= "data 2 signature",
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
					.name			= "data 3",
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
	template<std::derived_from<Module> IO>
		explicit
		AirToGroundLinkProtocol (IO& io, xle::SecureChannel* secure_channel = nullptr):
			LinkProtocol ({
				// XLE handshake envelope:
				envelope ({
					.name			= "XLE handshake",
					.unique_prefix	= { 0xff, 0x00 },
					.send_predicate	= [&io] { return io.handshake_response.valid(); },
					.packets		= {
						// Always good to have at least basic checksum:
						signature ({
							.name				= "XLE handshake signature",
							.nonce_bytes		= 0,
							.signature_bytes	= 4,
							.key				= { 0xbb, 0xbb },
							.packets			= {
								socket<256> (io.handshake_response, { .retained = false, .transfer = StringTransfer::Segmented }),
							},
						}),
					},
				}),
				// Normal data envelope:
				envelope ({
					.name			= "data",
					.unique_prefix	= { 0xff, 0x01 },
					.secure_channel	= secure_channel,
					.packets		= {
						socket<4> (io.int_prop),
					},
				}),
			})
		{ }
};


using Ground_Tx_Data = GroundToAirData<ModuleIn>;
using Ground_Rx_Data = AirToGroundData<ModuleOut>;
using Air_Tx_Data = AirToGroundData<ModuleIn>;
using Air_Rx_Data = GroundToAirData<ModuleOut>;


template<template<class> class SocketType>
	class HelperTestIO: public Module
	{
	  public:
		SocketType<bool>		bool_prop	{ this, "bool_prop" };
		SocketType<int64_t>		int_prop	{ this, "int_prop" };
		SocketType<float>		float_prop	{ this, "float_prop" };

	  public:
		// Ctor
		using Module::Module;

		void
		fetch_all (Cycle const& cycle)
		{
			std::initializer_list<BasicSocket*> const sockets = {
				&bool_prop,
				&int_prop,
				&float_prop,
			};

			for (auto* socket: sockets)
				socket->fetch (cycle);
		}
	};


auto constinit crypto_params = xle::SecureChannel::CryptoParams {
	.master_signature_key		= { 0x00, 0x01, 0x02, 0x03 },
	.slave_signature_key		= { 0x0c, 0x0d, 0x0e, 0x0f },
	.authentication_secret		= { 0x01 },
	.data_encryption_secret		= { 0x02 },
	.seq_num_encryption_secret	= { 0x03 },
	.hmac_size					= 16,
	.max_time_difference		= 60_s,
};


xle::MasterSecureChannel
get_ground_secure_channel (ProcessingLoop& loop)
{
	return xle::MasterSecureChannel (loop, crypto_params, g_logger.with_context ("ground-secure-channel"), "ground/secure-channel");
}


xle::SlaveSecureChannel
get_air_secure_channel (ProcessingLoop& loop)
{
	return xle::SlaveSecureChannel (loop, crypto_params, {}, g_logger.with_context ("air-secure-channel"), "air/secure-channel");
}


void transmit (LinkProtocol& tx_protocol, LinkProtocol& rx_protocol)
{
	Blob blob;
	tx_protocol.produce_append (blob, g_logger);
	auto const consume_result = rx_protocol.consume (blob.begin(), blob.end(), g_logger);

	test_asserts::verify ("rx_protocol ate all input bytes", consume_result.parsing_end == blob.end());
	test_asserts::verify_equal ("valid_bytes == blob.size()", consume_result.valid_bytes, blob.size());
	test_asserts::verify ("valid_envelopes >= 1", consume_result.valid_envelopes >= 1u);
	test_asserts::verify_equal ("error_bytes == 0", consume_result.error_bytes, 0u);
}


nu::AutoTest t1 ("modules/io/link: protocol: valid data transmission", []{
	TestProcessingLoop loop (0.1_s);
	Ground_Tx_Data tx (loop);
	Air_Rx_Data rx (loop);
	GroundToAirLinkProtocol tx_protocol (tx);
	GroundToAirLinkProtocol rx_protocol (rx);
	TestCycle cycle;

	auto test = [&]{
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

	auto multiblock_test = [&]{
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


nu::AutoTest t2 ("modules/io/link: protocol: nils and out-of range values transmission", []{
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


nu::AutoTest t3 ("modules/io/link: protocol: offsets increase precision", []{
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


nu::AutoTest t4 ("modules/io/link: protocol: invalid data transmission (wrong signature)", []{
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

	{
		Blob blob;
		tx_protocol.produce_append (blob, g_logger);
		auto const consume_result = rx_protocol.consume (blob.begin(), blob.end(), g_logger);

		test_asserts::verify_equal ("all input is consumed", consume_result.parsing_end, blob.end());
		test_asserts::verify_equal ("valid_bytes == sent bytes", consume_result.valid_bytes, blob.size());

		// Expect 2 envelopes: "data 1" and "data 2".
		// The handshake envelope is not sent without io.handshake_request.valid() and "data 3" is sent
		// later (.send_every + .send_offset).
		test_asserts::verify_equal ("valid_envelopes == 2", consume_result.valid_envelopes, 2u);

		test_asserts::verify_equal ("error_bytes == 0", consume_result.error_bytes, 0u);
	}

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

	{
		Blob blob;
		tx_protocol.produce_append (blob, g_logger);
		test_asserts::verify ("blob is long enough", blob.size() >= 16);
		// Mess with both messages.
		// Careful not to touch the last envelope "data 3", which shouldn't be sent in the first second,
		// so we only have "data 1" and "data 2" here.
		blob[12] = 0x00;
		blob[13] = 0xff;
		blob[14] = 0x00;
		blob[15] = 0xff;
		blob[blob.size() - 6] = 0xff;
		blob[blob.size() - 5] = 0x00;
		blob[blob.size() - 4] = 0xff;

		auto const consume_result = rx_protocol.consume (blob.begin(), blob.end(), g_logger);
		auto const consumed_bytes = consume_result.parsing_end - blob.begin();

		test_asserts::verify ("not all input is consumed", consume_result.parsing_end != blob.end());
		test_asserts::verify_equal ("valid_bytes == 0", consume_result.valid_bytes, 0u);

		// Expect 0 envelopes consumed as they're both interferred with:
		test_asserts::verify_equal ("valid_envelopes == 0", consume_result.valid_envelopes, 0u);

		test_asserts::verify_equal ("error_bytes == consumed_bytes", consume_result.error_bytes, consumed_bytes);
	}

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


nu::AutoTest t5 ("modules/io/link: protocol: send-every/send-offset", []{
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


nu::AutoTest t6 ("modules/io/link: protocol: encrypted channel works", []{
	TestProcessingLoop loop (0.1_s);
	Ground_Tx_Data ground_tx_data (loop);
	Ground_Rx_Data ground_rx_data (loop);
	Air_Tx_Data air_tx_data (loop);
	Air_Rx_Data air_rx_data (loop);

	auto ground_transceiver = get_ground_secure_channel (loop);
	auto air_transceiver = get_air_secure_channel (loop);

	auto ground_tx_protocol = std::make_unique<GroundToAirLinkProtocol> (ground_tx_data, &ground_transceiver);
	auto ground_rx_protocol = std::make_unique<AirToGroundLinkProtocol> (ground_rx_data, &ground_transceiver);
	auto air_tx_protocol = std::make_unique<AirToGroundLinkProtocol> (air_tx_data, &air_transceiver);
	auto air_rx_protocol = std::make_unique<GroundToAirLinkProtocol> (air_rx_data, &air_transceiver);

	auto ground_tx_link = LinkEncoder (loop, std::move (ground_tx_protocol), { .send_frequency = 30_Hz }, g_logger.with_context ("ground-tx-link"), "ground/tx-link");
	auto ground_rx_link = LinkDecoder (loop, std::move (ground_rx_protocol), {}, g_logger.with_context ("ground-rx-link"), "ground/rx-link");
	auto air_tx_link = LinkEncoder (loop, std::move (air_tx_protocol), { .send_frequency = 30_Hz }, g_logger.with_context ("air-tx-link"), "air/tx-link");
	auto air_rx_link = LinkDecoder (loop, std::move (air_rx_protocol), {}, g_logger.with_context ("air-rx-link"), "air/rx-link");

	ground_tx_data.handshake_request << ground_transceiver.handshake_request;
	ground_transceiver.handshake_response << ground_rx_data.handshake_response;

	air_transceiver.handshake_request << air_rx_data.handshake_request;
	air_tx_data.handshake_response << air_transceiver.handshake_response;

	air_rx_link.encoded_input << ground_tx_link.encoded_output;
	ground_rx_link.encoded_input << air_tx_link.encoded_output;

	auto [session_prepared, session_activated] = ground_transceiver.start_handshake();
	auto constexpr kMaxCycles = 6u;

	for (size_t cycles = 0; !nu::ready (session_prepared) && !nu::ready (session_activated); ++cycles)
	{
		test_asserts::verify (std::format ("handshake completes in {} cycles", cycles), cycles < kMaxCycles);
		loop.next_cycle();
	}

	// Test data transmission in both directions:
	loop.next_cycles (5);

	// Ground to air encryption:
	{
		auto const u = nu::to_blob ("hello!");
		auto const e = ground_transceiver.encrypt_packet (u);
		auto const d = air_transceiver.decrypt_packet (e);

		test_asserts::verify ("encryption works from ground to air using transceivers directly", d == u);
	}

	// Air to ground encryption:
	{
		auto const u = nu::to_blob ("hello back!");
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


nu::AutoTest t7 ("modules/io/link/helpers: inputs->outputs, single module", []{
	TestProcessingLoop loop (0.1_s);
	HelperTestIO<ModuleIn> tx (loop);
	HelperTestIO<ModuleOut> rx (loop);
	auto tx_protocol = make_link_protocol_from_inputs (tx, "helper single", { 0x71, 0x01 });
	auto rx_protocol = make_link_protocol_from_outputs (rx, "helper single", { 0x71, 0x01 });
	TestCycle cycle;

	tx.bool_prop << true;
	tx.int_prop << -123456789LL;
	tx.float_prop << 1234.5f;
	tx.fetch_all (cycle += 1_s);
	transmit (*tx_protocol, *rx_protocol);

	test_asserts::verify ("sender input bool transmitted to receiver output", tx.bool_prop == rx.bool_prop);
	test_asserts::verify ("sender input int transmitted to receiver output", tx.int_prop == rx.int_prop);
	test_asserts::verify ("sender input float transmitted to receiver output", tx.float_prop == rx.float_prop);

	rx.bool_prop = false;
	rx.int_prop = 7LL;
	rx.float_prop = -9.75f;

	tx.bool_prop << xf::no_data_source;
	tx.int_prop << xf::no_data_source;
	tx.float_prop << xf::no_data_source;
	tx.fetch_all (cycle += 1_s);
	transmit (*tx_protocol, *rx_protocol);

	test_asserts::verify ("bool nil transmitted to output nil", !rx.bool_prop);
	test_asserts::verify ("int nil transmitted to output nil", !rx.int_prop);
	test_asserts::verify ("float nil transmitted to output nil", !rx.float_prop);
});


nu::AutoTest t8 ("modules/io/link/helpers: inputs->outputs, vector modules", []{
	TestProcessingLoop loop (0.1_s);
	HelperTestIO<ModuleIn> tx1 (loop, "tx1");
	HelperTestIO<ModuleIn> tx2 (loop, "tx2");
	HelperTestIO<ModuleOut> rx1 (loop, "rx1");
	HelperTestIO<ModuleOut> rx2 (loop, "rx2");
	auto tx_protocol = make_link_protocol_from_inputs ({ &tx1, &tx2 }, "helper vector", { 0x72, 0x01 });
	auto rx_protocol = make_link_protocol_from_outputs ({ &rx1, &rx2 }, "helper vector", { 0x72, 0x01 });
	TestCycle cycle;

	tx1.bool_prop << true;
	tx1.int_prop << 111LL;
	tx1.float_prop << 11.25f;
	tx2.bool_prop << false;
	tx2.int_prop << -222LL;
	tx2.float_prop << -22.5f;
	tx1.fetch_all (cycle += 1_s);
	tx2.fetch_all (cycle);
	transmit (*tx_protocol, *rx_protocol);

	test_asserts::verify ("vector module 1 bool transmitted", tx1.bool_prop == rx1.bool_prop);
	test_asserts::verify ("vector module 1 int transmitted", tx1.int_prop == rx1.int_prop);
	test_asserts::verify ("vector module 1 float transmitted", tx1.float_prop == rx1.float_prop);
	test_asserts::verify ("vector module 2 bool transmitted", tx2.bool_prop == rx2.bool_prop);
	test_asserts::verify ("vector module 2 int transmitted", tx2.int_prop == rx2.int_prop);
	test_asserts::verify ("vector module 2 float transmitted", tx2.float_prop == rx2.float_prop);

	rx1.bool_prop = true;
	rx1.int_prop = 1LL;
	rx1.float_prop = 1.0f;
	rx2.bool_prop = true;
	rx2.int_prop = 2LL;
	rx2.float_prop = 2.0f;

	tx1.bool_prop << xf::no_data_source;
	tx1.int_prop << xf::no_data_source;
	tx1.float_prop << xf::no_data_source;
	tx2.bool_prop << xf::no_data_source;
	tx2.int_prop << xf::no_data_source;
	tx2.float_prop << xf::no_data_source;
	tx1.fetch_all (cycle += 1_s);
	tx2.fetch_all (cycle);
	transmit (*tx_protocol, *rx_protocol);

	test_asserts::verify ("vector module 1 bool nil", !rx1.bool_prop);
	test_asserts::verify ("vector module 1 int nil", !rx1.int_prop);
	test_asserts::verify ("vector module 1 float nil", !rx1.float_prop);
	test_asserts::verify ("vector module 2 bool nil", !rx2.bool_prop);
	test_asserts::verify ("vector module 2 int nil", !rx2.int_prop);
	test_asserts::verify ("vector module 2 float nil", !rx2.float_prop);
});

} // namespace
} // namespace xf::test
