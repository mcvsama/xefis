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
#include <xefis/support/crypto/xle/handshake.h>

// Neutrino:
#include <neutrino/test/auto_test.h>
#include <neutrino/time_helper.h>

// Boost:
#include <boost/random/random_device.hpp>

// Standard:
#include <cstddef>
#include <set>


namespace xf::test {
namespace {

using xf::crypto::xle::HandshakeID;
using xf::crypto::xle::HandshakeMaster;
using xf::crypto::xle::HandshakeSlave;


Blob const master_signature_key		{ 0x00, 0xfa, 0x55, 0xcd, 0x12, 0x7f, 0xdd, 0xee };
Blob const master_signature_key_1	{ 0x00, 0xfa, 0x55, 0xcd, 0x12, 0x7f, 0xdd, 0xee };
Blob const master_signature_key_2	{ 0xff, 0xfa, 0x55, 0xcd, 0x12, 0x7f, 0xdd, 0xef };
Blob const slave_signature_key		{ 0x00, 0x55, 0xda, 0xcc, 0x77, 0xff, 0x00, 0x07 };
Blob const slave_signature_key_1	{ 0xee, 0x55, 0xda, 0xcc, 0x77, 0xff, 0x00, 0x07 };
Blob const slave_signature_key_2	{ 0xcc, 0x55, 0xda, 0xcc, 0x77, 0xff, 0x00, 0x07 };

HandshakeMaster::Params const params = {
	.master_signature_key = master_signature_key,
	.slave_signature_key = slave_signature_key,
	.hmac_size = 12,
	.max_time_difference = 10_s,
};


AutoTest t1 ("Xefis Lossy Encryption/Handshake: correct handshake", []{
	boost::random::random_device rnd;
	HandshakeMaster master (rnd, params);
	HandshakeSlave slave (rnd, params, {});

	Blob const master_handshake = master.generate_handshake_blob (neutrino::TimeHelper::utc_now());
	auto const slave_handshake_and_key = slave.generate_handshake_blob_and_key (master_handshake, neutrino::TimeHelper::utc_now());
	Blob const master_key = master.compute_key (*slave_handshake_and_key.handshake_response);
	Blob const slave_key = *slave_handshake_and_key.ephemeral_key;

	test_asserts::verify ("keys match", master_key == slave_key);
});


AutoTest t2 ("Xefis Lossy Encryption/Handshake: handshake with wrong signature", []{
	si::Time const now = neutrino::TimeHelper::utc_now();
	boost::random::random_device rnd;

	{
		HandshakeMaster master (rnd, {
			.master_signature_key = master_signature_key_1,
			.slave_signature_key = slave_signature_key,
		});
		HandshakeSlave slave (rnd, {
			.master_signature_key = master_signature_key_2,
			.slave_signature_key = slave_signature_key,
		}, {});

		Blob const master_handshake = master.generate_handshake_blob (now);
		bool thrown_wrong_signature = false;

		try {
			auto const slave_handshake_and_key [[maybe_unused]] = slave.generate_handshake_blob_and_key (master_handshake, now);
		}
		catch (xf::crypto::xle::HandshakeSlave::Exception const& exception)
		{
			test_asserts::verify ("correct error is signalled", exception.error_code() == xf::crypto::xle::HandshakeSlave::ErrorCode::WrongSignature);
			thrown_wrong_signature = true;
		}

		test_asserts::verify ("wrong signature is signalled on the Slave side", thrown_wrong_signature);
	}

	{
		HandshakeMaster master_1 (rnd, {
			.master_signature_key = master_signature_key,
			.slave_signature_key = slave_signature_key_1,
		});
		HandshakeMaster master_2 (rnd, {
			.master_signature_key = master_signature_key,
			.slave_signature_key = slave_signature_key_2,
		});
		HandshakeSlave slave (rnd, {
			.master_signature_key = master_signature_key,
			.slave_signature_key = slave_signature_key_1,
		}, {});

		Blob const master_1_handshake = master_1.generate_handshake_blob (now);
		Blob const master_2_handshake = master_2.generate_handshake_blob (now);
		auto const slave_handshake_and_key = slave.generate_handshake_blob_and_key (master_1_handshake, now);
		bool thrown_wrong_signature = false;

		try {
			Blob const master_key [[maybe_unused]] = master_2.compute_key (*slave_handshake_and_key.handshake_response);
		}
		catch (HandshakeMaster::Exception const& exception)
		{
			test_asserts::verify ("correct error is signalled", exception.error_code() == HandshakeMaster::ErrorCode::WrongSignature);
			thrown_wrong_signature = true;
		}

		test_asserts::verify ("wrong signature is signalled on Master side", thrown_wrong_signature);
	}
});


AutoTest t3 ("Xefis Lossy Encryption/Handshake: reusing handshake ID", []{
	auto const now = neutrino::TimeHelper::utc_now();
	auto rnd = boost::random::random_device();
	auto used_handshake_ids = std::set<HandshakeID>();
	HandshakeMaster master (rnd, params);
	HandshakeSlave slave (rnd, params, {
		.store_key_function = [&used_handshake_ids] (HandshakeID const handshake_id) {
			used_handshake_ids.insert (handshake_id);
		},
		.contains_key_function = [&used_handshake_ids] (HandshakeID const& handshake_id) {
			return used_handshake_ids.contains (handshake_id);
		},
	});

	Blob const master_handshake = master.generate_handshake_blob (now);
	auto const slave_handshake_and_key = slave.generate_handshake_blob_and_key (master_handshake, now);
	bool thrown_reused_handshake_id = false;

	try {
		auto const slave_handshake_and_key [[maybe_unused]] = slave.generate_handshake_blob_and_key (master_handshake, now);
	}
	catch (xf::crypto::xle::HandshakeSlave::Exception const& exception)
	{
		test_asserts::verify ("correct error is signalled", exception.error_code() == HandshakeSlave::ErrorCode::ReusedHandshakeID);
		thrown_reused_handshake_id = true;
	}

	test_asserts::verify ("reused-handshake-id is signalled", thrown_reused_handshake_id);
});


AutoTest t4 ("Xefis Lossy Encryption/Handshake: wrong timestamp on master side", []{
	boost::random::random_device rnd;
	HandshakeMaster master (rnd, params);
	HandshakeSlave slave (rnd, params, {});

	si::Time const now = neutrino::TimeHelper::utc_now();
	Blob const master_handshake = master.generate_handshake_blob (now + 20_s);

	bool thrown_delta_time_too_high = false;

	try {
		auto const slave_handshake_and_key [[maybe_unused]] = slave.generate_handshake_blob_and_key (master_handshake, now);
	}
	catch (xf::crypto::xle::HandshakeSlave::Exception const& exception)
	{
		test_asserts::verify ("correct error is signalled", exception.error_code() == HandshakeSlave::ErrorCode::DeltaTimeTooHigh);
		thrown_delta_time_too_high = true;
	}

	test_asserts::verify ("delta-time-too-high is signalled", thrown_delta_time_too_high);
});


AutoTest t5 ("Xefis Lossy Encryption/Handshake: wrong timestamp on slave side", []{
	boost::random::random_device rnd;
	HandshakeMaster master (rnd, params);
	HandshakeSlave slave (rnd, params, {});

	si::Time const now = neutrino::TimeHelper::utc_now();
	Blob const master_handshake = master.generate_handshake_blob (now);
	auto const slave_handshake_and_key = slave.generate_handshake_blob_and_key (master_handshake, now + 20_s);

	bool thrown_delta_time_too_high = false;

	try {
		auto const encryption_key [[maybe_unused]] = master.compute_key (*slave_handshake_and_key.handshake_response);
	}
	catch (xf::crypto::xle::HandshakeMaster::Exception const& exception)
	{
		test_asserts::verify ("correct error is signalled", exception.error_code() == HandshakeMaster::ErrorCode::DeltaTimeTooHigh);
		thrown_delta_time_too_high = true;
	}

	test_asserts::verify ("delta-time-too-high is signalled", thrown_delta_time_too_high);
});


AutoTest t6 ("Xefis Lossy Encryption/Handshake: mismatched handshake IDs", []{
	auto const now = neutrino::TimeHelper::utc_now();
	auto rnd = boost::random::random_device();
	auto used_handshake_ids = std::set<HandshakeID>();
	HandshakeMaster master_1 (rnd, params);
	HandshakeMaster master_2 (rnd, params);
	HandshakeSlave slave (rnd, params, {
		.store_key_function = [&used_handshake_ids] (HandshakeID const handshake_id) {
			used_handshake_ids.insert (handshake_id);
		},
		.contains_key_function = [&used_handshake_ids] (HandshakeID const& handshake_id) {
			return used_handshake_ids.contains (handshake_id);
		},
	});

	Blob const master_1_handshake = master_1.generate_handshake_blob (now);
	Blob const master_2_handshake = master_2.generate_handshake_blob (now);
	auto const slave_handshake_and_key = slave.generate_handshake_blob_and_key (master_1_handshake, now);
	bool thrown_invalid_handshake_id = false;

	try {
		Blob const key = master_2.compute_key (*slave_handshake_and_key.handshake_response);
	}
	catch (xf::crypto::xle::HandshakeMaster::Exception const& exception)
	{
		test_asserts::verify ("correct error is signalled", exception.error_code() == HandshakeMaster::ErrorCode::InvalidHandshakeID);
		thrown_invalid_handshake_id = true;
	}

	test_asserts::verify ("invalid-handshake-id is signalled", thrown_invalid_handshake_id);
});

} // namespace
} // namespace xf::test

