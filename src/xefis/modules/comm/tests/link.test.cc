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
#include <xefis/modules/comm/link.h>

// Neutrino:
#include <neutrino/test/auto_test.h>

// Standard:
#include <cstddef>


namespace xf::test {
namespace {

xf::LoggerOutput g_logger_output (std::clog);
xf::Logger g_logger (g_logger_output);


template<template<class> class SocketType>
	class GCS2AircraftLink: public Module
	{
	  public:
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
		void
		fetch_all (Cycle const& cycle)
		{
			std::initializer_list<BasicSocket*> const sockets = {
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


using GCS_Tx_Link = GCS2AircraftLink<xf::ModuleIn>;
using Aircraft_Rx_Link = GCS2AircraftLink<xf::ModuleOut>;


constexpr auto kFallbackBool	= true;
constexpr auto kFallbackInt		= 12UL;


class GCS_Tx_LinkProtocol: public LinkProtocol
{
  public:
	// Ctor
	template<class IO>
		explicit
		GCS_Tx_LinkProtocol (IO* io):
			LinkProtocol ({
				envelope (Magic ({ 0xe4, 0x40 }), {
					signature (NonceBytes (8), SignatureBytes (12), Key ({ 0x88, 0x99, 0xaa, 0xbb }), {
						socket<8> (io->nil_si_prop,				Retained (false)),
						socket<8> (io->angle_prop,				Retained (false)),
						socket<8> (io->angle_prop_r,			Retained (true)),
						socket<2> (io->velocity_prop,			Retained (false)),
						socket<2> (io->velocity_prop_r,			Retained (true)),
						socket<2> (io->velocity_prop_offset,	Retained (false),	1000_kph),
						socket<2> (io->velocity_prop_offset_r,	Retained (true),	1000_kph),
						socket<2> (io->int_prop,				Retained (false),	0L),
						socket<2> (io->int_prop_r,				Retained (true),	0L),
					}),
				}),
				envelope (Magic ({ 0xa3, 0x80 }), {
					signature (NonceBytes (8), SignatureBytes (8), Key ({ 0x55, 0x37, 0x12, 0xf9 }), {
						bitfield ({
							bitfield_socket (io->bool_prop,					Retained (false),	kFallbackBool),
							bitfield_socket (io->bool_prop_r,				Retained (true),	kFallbackBool),
							bitfield_socket (io->uint_prop,		Bits (4),	Retained (true),	kFallbackInt),
							bitfield_socket (io->uint_prop_r,	Bits (4),	Retained (true),	kFallbackInt),
						}),
					}),
				}),
				envelope (Magic ({ 0x01, 0x02 }), SendEvery (10), SendOffset (8), {
					socket<4> (io->dummy, Retained (false), 0L),
				}),
			})
		{ }
};


class GCS_Rx_LinkProtocol: public LinkProtocol
{
  public:
	// Ctor
	template<class IO>
		explicit
		GCS_Rx_LinkProtocol (IO* io):
			LinkProtocol({
				envelope (Magic ({ 0xe4, 0x40 }), {
					signature (NonceBytes (8), SignatureBytes (12), Key ({ 0x87, 0x11, 0x65, 0xa4 }), {
					}),
				}),
			})
		{ }
};


void transmit (LinkProtocol& tx_protocol, LinkProtocol& rx_protocol)
{
	Blob blob;
	tx_protocol.produce (blob, g_logger);
	auto end = rx_protocol.eat (blob.begin(), blob.end(), nullptr, nullptr, nullptr, g_logger);

	test_asserts::verify ("rx_protocol ate all input bytes", end == blob.end());
}


AutoTest t1 ("modules/io/link: protocol: valid data transmission", []{
	GCS_Tx_Link tx;
	Aircraft_Rx_Link rx;
	GCS_Tx_LinkProtocol tx_protocol (&tx);
	GCS_Tx_LinkProtocol rx_protocol (&rx);
	TestCycle cycle;

	auto test = [&] {
		cycle += 1_s;
		tx.fetch_all (cycle);
		transmit (tx_protocol, rx_protocol);
		test_asserts::verify ("nil_si_prop transmitted properly", rx.nil_si_prop == tx.nil_si_prop);
		test_asserts::verify ("angle_prop transmitted properly (optional() version)", rx.angle_prop == tx.angle_prop);
		test_asserts::verify ("angle_prop transmitted properly", *rx.angle_prop == *tx.angle_prop);
		test_asserts::verify_equal_with_epsilon ("velocity transmitted properly", *rx.velocity_prop, *tx.velocity_prop, 0.1_kph);
		test_asserts::verify_equal_with_epsilon ("velocity prop with offset transmitted properly", *rx.velocity_prop_offset, *tx.velocity_prop_offset, 0.1_mps);
		test_asserts::verify ("bool_prop transmitted properly", *rx.bool_prop == *tx.bool_prop);
		test_asserts::verify ("int_prop transmitted properly", *rx.int_prop == *tx.int_prop);
		test_asserts::verify ("uint_prop transmitted properly", *rx.uint_prop == *tx.uint_prop);
	};

	tx.angle_prop << 1.99_rad;
	tx.velocity_prop << 101_kph;
	tx.velocity_prop_offset << 101_kph;
	tx.bool_prop << true;
	tx.int_prop << -2;
	tx.uint_prop << 3u;
	test();

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
	GCS_Tx_Link tx;
	Aircraft_Rx_Link rx;
	GCS_Tx_LinkProtocol tx_protocol (&tx);
	GCS_Tx_LinkProtocol rx_protocol (&rx);
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
	GCS_Tx_Link tx;
	Aircraft_Rx_Link rx;
	GCS_Tx_LinkProtocol tx_protocol (&tx);
	GCS_Tx_LinkProtocol rx_protocol (&rx);
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

	GCS_Tx_Link tx;
	Aircraft_Rx_Link rx;
	GCS_Tx_LinkProtocol tx_protocol (&tx);
	GCS_Tx_LinkProtocol rx_protocol (&rx);
	TestCycle cycle;

	// Transmit correctly first:
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
	rx_protocol.eat (blob.begin(), blob.end(), nullptr, nullptr, nullptr, g_logger);

	// Transmit invalid data:
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
	// Careful not to touch the last envelope, which shouldn't be sent in second:
	blob[blob.size() - 6] = 0xff;
	blob[blob.size() - 5] = 0x00;
	blob[blob.size() - 4] = 0xff;
	rx_protocol.eat (blob.begin(), blob.end(), nullptr, nullptr, nullptr, g_logger);

	// Test that values weren't changed during last invalid transmission:
	test_asserts::verify ("nil_si_prop didn't change", !rx.nil_si_prop);
	test_asserts::verify ("angle_prop is didn't change", *rx.angle_prop == 15_rad);
	test_asserts::verify_equal_with_epsilon ("velocity_prop didn't change", *rx.velocity_prop, 100_mps, 0.1_mps);
	test_asserts::verify_equal_with_epsilon ("velocity_prop_offset didn't change", *rx.velocity_prop_offset, 102_mps, 0.1_mps);
	test_asserts::verify ("int_prop didn't change", *rx.int_prop == -5);
	test_asserts::verify ("bool_prop didn't change", *rx.bool_prop == false);
	test_asserts::verify ("uint_prop didn't change", *rx.uint_prop == 15u);

	// Test fail-safe and retaining of original values:
	rx_protocol.failsafe();
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
	test_asserts::verify ("uint_prop is nil", !!rx.uint_prop);
	test_asserts::verify ("uint_prop_r is retained", *rx.uint_prop_r == 15u);
});


AutoTest t5 ("modules/io/link: protocol: send-every/send-offset", []{
	// The third envelope should be sent every two packets, starting from packet with index 1.

	GCS_Tx_Link tx;
	Aircraft_Rx_Link rx;
	GCS_Tx_LinkProtocol tx_protocol (&tx);
	GCS_Tx_LinkProtocol rx_protocol (&rx);
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

} // namespace
} // namespace xf::test

