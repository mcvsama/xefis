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

// Xefis:
#include <xefis/test/test.h>
#include <xefis/modules/io/link.h>


namespace xf {
namespace test {

template<template<class> class PropertyType>
	class GCS2AircraftLinkIO: public LinkIO
	{
	  public:
		PropertyType<si::Angle>		nil_prop				{ this, "/nil" };
		PropertyType<si::Angle>		angle_prop				{ this, "/angle" };
		PropertyType<si::Angle>		angle_prop_r			{ this, "/angle_r" };
		PropertyType<si::Velocity>	velocity_prop			{ this, "/velocity" };
		PropertyType<si::Velocity>	velocity_prop_r			{ this, "/velocity_r" };
		PropertyType<si::Velocity>	velocity_prop_offset	{ this, "/velocity_prop_offset" };
		PropertyType<si::Velocity>	velocity_prop_offset_r	{ this, "/velocity_prop_offset_r" };
		PropertyType<bool>			bool_prop				{ this, "/bool" };
		PropertyType<bool>			bool_prop_r				{ this, "/bool_r" };
		PropertyType<int64_t>		int_prop				{ this, "/int" };
		PropertyType<int64_t>		int_prop_r				{ this, "/int_r" };
		PropertyType<uint64_t>		uint_prop				{ this, "/uint" };
		PropertyType<uint64_t>		uint_prop_r				{ this, "/uint_r" };
		PropertyType<int64_t>		dummy					{ this, "/dummy" };
	};


using GCS_Tx_LinkIO = GCS2AircraftLinkIO<v2::PropertyIn>;
using Aircraft_Rx_LinkIO = GCS2AircraftLinkIO<v2::PropertyOut>;


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
						property<8> (io->nil_prop,					Retained (false)),
						property<8> (io->angle_prop,				Retained (false)),
						property<8> (io->angle_prop_r,				Retained (true)),
						property<2> (io->velocity_prop,				Retained (false)),
						property<2> (io->velocity_prop_r,			Retained (true)),
						property<2> (io->velocity_prop_offset,		Retained (false),	1000_kph),
						property<2> (io->velocity_prop_offset_r,	Retained (true),	1000_kph),
						property<2> (io->int_prop,					Retained (false),	0L),
						property<2> (io->int_prop_r,				Retained (true),	0L),
					}),
				}),
				envelope (Magic ({ 0xa3, 0x80 }), {
					signature (NonceBytes (8), SignatureBytes (8), Key ({ 0x55, 0x37, 0x12, 0xf9 }), {
						bitfield ({
							bitfield_property (io->bool_prop,					Retained (false),	kFallbackBool),
							bitfield_property (io->bool_prop_r,					Retained (true),	kFallbackBool),
							bitfield_property (io->uint_prop,		Bits (4),	Retained (true),	kFallbackInt),
							bitfield_property (io->uint_prop_r,		Bits (4),	Retained (true),	kFallbackInt),
						}),
					}),
				}),
				envelope (Magic ({ 0x01, 0x02 }), SendEvery (10), SendOffset (8), {
					property<4> (io->dummy, Retained (false), 0L),
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
	tx_protocol.produce (blob);
	auto end = rx_protocol.eat (blob.begin(), blob.end(), nullptr, nullptr, nullptr);

	xf::test_asserts::verify ("rx_protocol ate all input bytes", end == blob.end());
}


static xf::RuntimeTest t1 ("modules/io/link: protocol: valid data transmission", []{
	GCS_Tx_LinkIO tx_io;
	Aircraft_Rx_LinkIO rx_io;
	GCS_Tx_LinkProtocol tx_protocol (&tx_io);
	GCS_Tx_LinkProtocol rx_protocol (&rx_io);

	auto test = [&] {
		transmit (tx_protocol, rx_protocol);
		xf::test_asserts::verify ("nil_prop transmitted properly", rx_io.angle_prop.get_optional() == tx_io.angle_prop.get_optional());
		xf::test_asserts::verify ("angle_prop transmitted properly", *rx_io.angle_prop == *tx_io.angle_prop);
		xf::test_asserts::verify_equal_with_epsilon ("velocity transmitted properly", *rx_io.velocity_prop, *tx_io.velocity_prop, 0.1_kph);
		xf::test_asserts::verify_equal_with_epsilon ("velocity prop with offset transmitted properly", *rx_io.velocity_prop_offset, *tx_io.velocity_prop_offset, 0.1_mps);
		xf::test_asserts::verify ("bool_prop transmitted properly", *rx_io.bool_prop == *tx_io.bool_prop);
		xf::test_asserts::verify ("int_prop transmitted properly", *rx_io.int_prop == *tx_io.int_prop);
		xf::test_asserts::verify ("uint_prop transmitted properly", *rx_io.uint_prop == *tx_io.uint_prop);
	};

	tx_io.angle_prop = 1.99_rad;
	tx_io.velocity_prop = 101_kph;
	tx_io.velocity_prop_offset = 101_kph;
	tx_io.bool_prop = true;
	tx_io.int_prop = -2;
	tx_io.uint_prop = 3;
	test();

	for (auto const& angle: { -12_rad, 0_rad, 0.99_rad, 1.59_rad, 300_rad })
	{
		tx_io.angle_prop = angle;
		test();
	}

	for (auto b: { false, true })
	{
		tx_io.bool_prop = b;
		test();
	}

	for (auto i: { -9, -7, -5, -3, -2, -1, 0, 1, 2, 3, 5, 7, 9 })
	{
		tx_io.int_prop = i;
		test();
	}

	for (auto i: { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 })
	{
		tx_io.uint_prop = i;
		test();
	}
});


static xf::RuntimeTest t2 ("modules/io/link: protocol: nils and out-of range values transmission", []{
	GCS_Tx_LinkIO tx_io;
	Aircraft_Rx_LinkIO rx_io;
	GCS_Tx_LinkProtocol tx_protocol (&tx_io);
	GCS_Tx_LinkProtocol rx_protocol (&rx_io);

	// Test bit-bool:

	tx_io.bool_prop = true;
	transmit (tx_protocol, rx_protocol);
	xf::test_asserts::verify ("bit-bool 1 is transmitted properly", *rx_io.bool_prop == *tx_io.bool_prop);

	tx_io.bool_prop = false;
	transmit (tx_protocol, rx_protocol);
	xf::test_asserts::verify ("bit-bool 0 is transmitted properly", *rx_io.bool_prop == *tx_io.bool_prop);

	tx_io.bool_prop.set_nil();
	transmit (tx_protocol, rx_protocol);
	xf::test_asserts::verify ("nil bit-bool set to fall-back value", *rx_io.bool_prop == kFallbackBool);

	// Test bit-int:

	tx_io.uint_prop = 11;
	transmit (tx_protocol, rx_protocol);
	xf::test_asserts::verify ("bit-int 11 transmitted properly", *rx_io.uint_prop == *tx_io.uint_prop);

	tx_io.uint_prop = 17;
	transmit (tx_protocol, rx_protocol);
	xf::test_asserts::verify ("out-of-range bit-int set to fall-back value", *rx_io.uint_prop == kFallbackInt);

	tx_io.uint_prop = 15;
	transmit (tx_protocol, rx_protocol);
	xf::test_asserts::verify ("bit-int 15 transmitted properly", *rx_io.uint_prop == *tx_io.uint_prop);

	tx_io.uint_prop.set_nil();
	transmit (tx_protocol, rx_protocol);
	xf::test_asserts::verify ("nil bit-int set to fall-back value", *rx_io.uint_prop == kFallbackInt);
});


static xf::RuntimeTest t3 ("modules/io/link: protocol: offsets increase precision", []{
	GCS_Tx_LinkIO tx_io;
	Aircraft_Rx_LinkIO rx_io;
	GCS_Tx_LinkProtocol tx_protocol (&tx_io);
	GCS_Tx_LinkProtocol rx_protocol (&rx_io);

	tx_io.velocity_prop = 1001_kph;
	tx_io.velocity_prop_offset = 1001_kph;
	transmit (tx_protocol, rx_protocol);
	auto delta = si::abs (*rx_io.velocity_prop - *tx_io.velocity_prop);
	auto delta_with_offset = si::abs (*rx_io.velocity_prop_offset - *tx_io.velocity_prop_offset);
	xf::test_asserts::verify ("offsets increase precision", delta_with_offset < delta);
});


static xf::RuntimeTest t4 ("modules/io/link: protocol: invalid data transmission (wrong signature)", []{
	// This tests signature verification and checks if values are retained or not, according to the protocol.

	GCS_Tx_LinkIO tx_io;
	Aircraft_Rx_LinkIO rx_io;
	GCS_Tx_LinkProtocol tx_protocol (&tx_io);
	GCS_Tx_LinkProtocol rx_protocol (&rx_io);

	// Transmit correctly first:
	tx_io.nil_prop.set_nil();
	tx_io.angle_prop = 15_rad;
	tx_io.angle_prop_r = 15_rad;
	tx_io.velocity_prop = 100_mps;
	tx_io.velocity_prop_r = 100_mps;
	tx_io.velocity_prop_offset = 102_mps;
	tx_io.velocity_prop_offset_r = 102_mps;
	tx_io.bool_prop = false;
	tx_io.bool_prop_r = false;
	tx_io.int_prop = -5;
	tx_io.int_prop_r = -5;
	tx_io.uint_prop = 15;
	tx_io.uint_prop_r = 15;

	Blob blob;
	tx_protocol.produce (blob);
	rx_protocol.eat (blob.begin(), blob.end(), nullptr, nullptr, nullptr);

	// Transmit invalid data:
	tx_io.nil_prop = 1_rad;
	tx_io.angle_prop = 16_rad;
	tx_io.angle_prop_r = 16_rad;
	tx_io.velocity_prop = 101_mps;
	tx_io.velocity_prop_r = 101_mps;
	tx_io.velocity_prop_offset = 103_mps;
	tx_io.velocity_prop_offset_r = 103_mps;
	tx_io.bool_prop = true;
	tx_io.bool_prop_r = true;
	tx_io.int_prop = -3;
	tx_io.int_prop_r = -3;
	tx_io.uint_prop = 12;
	tx_io.uint_prop_r = 12;

	blob.clear();
	tx_protocol.produce (blob);
	xf::test_asserts::verify ("blob is long enough", blob.size() >= 16);
	// Mess with both messages:
	blob[12] = 0x00;
	blob[13] = 0xff;
	blob[14] = 0x00;
	blob[15] = 0xff;
	// Careful not to touch the last envelope, which shouldn't be sent in second:
	blob[blob.size() - 6] = 0xff;
	blob[blob.size() - 5] = 0x00;
	blob[blob.size() - 4] = 0xff;
	rx_protocol.eat (blob.begin(), blob.end(), nullptr, nullptr, nullptr);

	// Test that values weren't changed during last invalid transmission:
	xf::test_asserts::verify ("nil_prop didn't change", !rx_io.nil_prop);
	xf::test_asserts::verify ("angle_prop is didn't change", *rx_io.angle_prop == 15_rad);
	xf::test_asserts::verify_equal_with_epsilon ("velocity_prop didn't change", *rx_io.velocity_prop, 100_mps, 0.1_mps);
	xf::test_asserts::verify_equal_with_epsilon ("velocity_prop_offset didn't change", *rx_io.velocity_prop_offset, 102_mps, 0.1_mps);
	xf::test_asserts::verify ("int_prop didn't change", *rx_io.int_prop == -5);
	xf::test_asserts::verify ("bool_prop didn't change", *rx_io.bool_prop == false);
	xf::test_asserts::verify ("uint_prop didn't change", *rx_io.uint_prop == 15);

	// Test fail-safe and retaining of original values:
	rx_protocol.failsafe();
	xf::test_asserts::verify ("nil_prop is nil", !rx_io.nil_prop);
	xf::test_asserts::verify ("angle_prop is nil", !rx_io.angle_prop);
	xf::test_asserts::verify ("angle_prop_r is retained", *rx_io.angle_prop_r == 15_rad);
	xf::test_asserts::verify ("velocity_prop is nil", !rx_io.velocity_prop);
	xf::test_asserts::verify_equal_with_epsilon ("velocity_prop_r is retained", *rx_io.velocity_prop_r, 100_mps, 0.1_mps);
	xf::test_asserts::verify ("velocity_prop_offset is nil", !rx_io.velocity_prop_offset);
	xf::test_asserts::verify_equal_with_epsilon ("velocity_prop_offset_r is retained", *rx_io.velocity_prop_offset_r, 102_mps, 0.1_mps);
	xf::test_asserts::verify ("bool_prop is nil", !rx_io.bool_prop);
	xf::test_asserts::verify ("bool_prop_r is retained", *rx_io.bool_prop_r == false);
	xf::test_asserts::verify ("int_prop is nil", !rx_io.int_prop);
	xf::test_asserts::verify ("int_prop_r is retained", *rx_io.int_prop_r == -5);
	xf::test_asserts::verify ("uint_prop is nil", !!rx_io.uint_prop);
	xf::test_asserts::verify ("uint_prop_r is retained", *rx_io.uint_prop_r == 15);
});


static xf::RuntimeTest t5 ("modules/io/link: protocol: send-every/send-offset works", []{
	// The third envelope should be sent every two packets, starting from packed with index 1.

	GCS_Tx_LinkIO tx_io;
	Aircraft_Rx_LinkIO rx_io;
	GCS_Tx_LinkProtocol tx_protocol (&tx_io);
	GCS_Tx_LinkProtocol rx_protocol (&rx_io);

	constexpr int64_t kFirstInt = 11223344;
	constexpr int64_t kSecondInt = 66775544;

	tx_io.dummy = kFirstInt;
	// According to envelope settings, one need to transmit 7 times before the envelope will be
	// actually sent:
	for (size_t i = 0; i < 8; ++i)
	{
		transmit (tx_protocol, rx_protocol);
		xf::test_asserts::verify ("last envelope not sent in " + std::to_string (i) + "-th transmission", !rx_io.dummy);
	}

	// 8th transmission (every 10th envelope is sent, but offset is 8 and this is the first time):
	transmit (tx_protocol, rx_protocol);
	xf::test_asserts::verify ("last envelope sent for the first time", *rx_io.dummy == *tx_io.dummy);

	tx_io.dummy = kSecondInt;

	// Next 9 transmissions should not send last envelope (it's sent every 10th):
	for (size_t i = 0; i < 9; ++i)
	{
		transmit (tx_protocol, rx_protocol);
		xf::test_asserts::verify ("last envelope not sent in subsequent transmissions", *rx_io.dummy == kFirstInt);
	}

	// 10th transmission updates the property:
	transmit (tx_protocol, rx_protocol);
	xf::test_asserts::verify ("last envelope sent for the second time", *rx_io.dummy == kSecondInt);
});

} // namespace test
} // namespace xf

