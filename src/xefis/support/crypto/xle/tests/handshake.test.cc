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
#include <set>

// Boost:
#include <boost/random/random_device.hpp>

// Neutrino:
#include <neutrino/test/auto_test.h>

// Xefis:
#include <xefis/support/crypto/xle/handshake.h>


namespace xf::test {
namespace {

Blob const master_signature_key		{ 0x00, 0xfa, 0x55, 0xcd, 0x12, 0x7f, 0xdd, 0xee };
Blob const master_signature_key_1	{ 0x00, 0xfa, 0x55, 0xcd, 0x12, 0x7f, 0xdd, 0xee };
Blob const master_signature_key_2	{ 0xff, 0xfa, 0x55, 0xcd, 0x12, 0x7f, 0xdd, 0xef };
Blob const slave_signature_key		{ 0x00, 0x55, 0xda, 0xcc, 0x77, 0xff, 0x00, 0x07 };
Blob const slave_signature_key_1	{ 0xee, 0x55, 0xda, 0xcc, 0x77, 0xff, 0x00, 0x07 };
Blob const slave_signature_key_2	{ 0xcc, 0x55, 0xda, 0xcc, 0x77, 0xff, 0x00, 0x07 };


AutoTest t1 ("Xefis Lossy Encryption: correct handshake", []{
	boost::random::random_device rnd;
	xf::crypto::xle::HandshakeMaster master (rnd, master_signature_key, slave_signature_key);
	xf::crypto::xle::HandshakeSlave slave (rnd, master_signature_key, slave_signature_key);

	Blob const master_handshake = master.generate_handshake_blob();
	auto const slave_handshake_and_key = slave.generate_handshake_blob_and_key (master_handshake);
	Blob const master_key = master.calculate_key (slave_handshake_and_key.handshake_response);
	Blob const slave_key = slave_handshake_and_key.ephemeral_key;

	test_asserts::verify ("keys match", master_key == slave_key);
});


AutoTest t2 ("Xefis Lossy Encryption: handshake with wrong signature", []{
	boost::random::random_device rnd;

	{
		xf::crypto::xle::HandshakeMaster master (rnd, master_signature_key_1, slave_signature_key);
		xf::crypto::xle::HandshakeSlave slave (rnd, master_signature_key_2, slave_signature_key);

		Blob const master_handshake = master.generate_handshake_blob();
		bool thrown_wrong_signature = false;

		try {
			auto const slave_handshake_and_key = slave.generate_handshake_blob_and_key (master_handshake);
		}
		catch (xf::crypto::xle::Handshake::WrongSignature&)
		{
			thrown_wrong_signature = true;
		}

		test_asserts::verify ("wrong signature is signalled on Slave side", thrown_wrong_signature);
		// TODO test reply error code
	}

	{
		xf::crypto::xle::HandshakeMaster master_1 (rnd, master_signature_key, slave_signature_key_1);
		xf::crypto::xle::HandshakeMaster master_2 (rnd, master_signature_key, slave_signature_key_2);
		xf::crypto::xle::HandshakeSlave slave (rnd, master_signature_key, slave_signature_key_1);

		Blob const master_1_handshake = master_1.generate_handshake_blob();
		Blob const master_2_handshake = master_2.generate_handshake_blob();
		auto const slave_handshake_and_key = slave.generate_handshake_blob_and_key (master_1_handshake);
		bool thrown_wrong_signature = false;

		try {
			Blob const master_key = master_2.calculate_key (slave_handshake_and_key.handshake_response);
		}
		catch (xf::crypto::xle::Handshake::WrongSignature&)
		{
			thrown_wrong_signature = true;
		}

		test_asserts::verify ("wrong signature is signalled on Master side", thrown_wrong_signature);
	}
});


AutoTest t3 ("Xefis Lossy Encryption: reusing handshake ID", []{
	using xf::crypto::xle::HandshakeID;

	boost::random::random_device rnd;
	xf::crypto::xle::HandshakeMaster master (rnd, master_signature_key, slave_signature_key);
	xf::crypto::xle::HandshakeSlave slave (rnd, master_signature_key, slave_signature_key);

	Blob const master_handshake = master.generate_handshake_blob();
	auto const slave_handshake_and_key = slave.generate_handshake_blob_and_key (master_handshake);

	slave.set_callback_for_handshake_id_used_before ([used_handshake_ids = std::set<HandshakeID>()] (HandshakeID const handshake_id) mutable {
		return used_handshake_ids.insert (handshake_id).second;
	});

	bool thrown_reused_handshake_id = false;

	try {
		auto const slave_handshake_and_key = slave.generate_handshake_blob_and_key (master_handshake);
	}
	catch (xf::crypto::xle::Handshake::ReusedHandshakeID&)
	{
		thrown_reused_handshake_id = true;
		// TODO test reply error code
	}

	test_asserts::verify ("reused-handshake-id is signalled", thrown_reused_handshake_id);
});


AutoTest t4 ("Xefis Lossy Encryption: wrong timestamp", []{
	// TODO
});

} // namespace
} // namespace xf::test

