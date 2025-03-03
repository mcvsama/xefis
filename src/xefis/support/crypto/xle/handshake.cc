/* vim:ts=4
 *
 * Copyleft 2021  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Local:
#include "handshake.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/crypto/hmac.h>
#include <neutrino/crypto/modp.h>
#include <neutrino/crypto/utility.h>
#include <neutrino/numeric.h>

// Boost:
#include <boost/random.hpp>

// Standard:
#include <cstddef>
#include <limits>


namespace xf::crypto::xle {

Handshake::Handshake (boost::random::random_device& random_device, Params const& params):
	_random_device (random_device),
	_master_signature_key (params.master_signature_key),
	_slave_signature_key (params.slave_signature_key),
	_dhe_exchange (random_device, RFC3526_Group14<DiffieHellmanExchange::Integer>),
	_hmac_size (params.hmac_size),
	_max_time_difference (params.max_time_difference)
{ }


Blob
HandshakeMaster::generate_handshake_blob (si::Time const unix_timestamp)
{
	using HandshakeIDLimits = std::numeric_limits<HandshakeID>;

	auto handshake_id_distribution = boost::random::uniform_int_distribution<HandshakeID> (HandshakeIDLimits::min(), HandshakeIDLimits::max());
	_handshake_id = handshake_id_distribution (_random_device);

	return make_master_handshake_blob ({
		.handshake_id = _handshake_id,
		.unix_timestamp_ms = round_to<uint64_t> (unix_timestamp.in<si::Millisecond>()),
		.dhe_exchange_blob = _dhe_exchange.generate_exchange_blob(),
	});
}


Blob
HandshakeMaster::calculate_key (BlobView const slave_handshake_blob)
{
	auto const slave_handshake = parse_and_verify_slave_handshake_blob (slave_handshake_blob);
	Blob const ephemeral_key_with_weak_bits = _dhe_exchange.calculate_key_with_weak_bits (slave_handshake.dhe_exchange_blob);
	Blob const ephemeral_key = calculate_hash<kHashAlgorithm> (ephemeral_key_with_weak_bits);

	return ephemeral_key;
}


Blob
HandshakeMaster::make_master_handshake_blob (MasterHandshake const& master_handshake)
{
	Blob const salt = random_blob (8, _random_device);
	Blob const handshake_id_blob = to_blob (master_handshake.handshake_id);
	Blob const handshake_data = salt + handshake_id_blob + to_blob (master_handshake.unix_timestamp_ms) + master_handshake.dhe_exchange_blob;
	Blob const signature = calculate_hmac<kHashAlgorithm> ({ .data = handshake_data, .key = *_master_signature_key }).substr (0, _hmac_size);

	return handshake_data + signature;
}


Handshake::SlaveHandshake
HandshakeMaster::parse_and_verify_slave_handshake_blob (BlobView const slave_handshake_blob)
{
	BlobView const extracted_dhe_exchange_blob_and_signature = slave_handshake_blob.substr (24);
	BlobView extracted_dhe_exchange_blob = extracted_dhe_exchange_blob_and_signature;
	BlobView extracted_signature = extracted_dhe_exchange_blob_and_signature;
	extracted_dhe_exchange_blob.remove_suffix (_hmac_size);
	extracted_signature.remove_prefix (extracted_dhe_exchange_blob.size());

	BlobView const extracted_handshake_data = slave_handshake_blob.substr (0, slave_handshake_blob.size() - _hmac_size);
	Blob const calculated_signature = calculate_hmac<kHashAlgorithm> ({ .data = extracted_handshake_data, .key = *_slave_signature_key }).substr (0, _hmac_size);

	SlaveHandshake extracted;
	extracted.handshake_id = neutrino::parse<decltype (extracted.handshake_id)> (slave_handshake_blob.substr (8, 8));
	extracted.unix_timestamp_ms = neutrino::parse<decltype (extracted.unix_timestamp_ms)> (slave_handshake_blob.substr (16, 8));
	extracted.dhe_exchange_blob = extracted_dhe_exchange_blob;

	if (extracted_signature != calculated_signature)
		throw Exception (ErrorCode::WrongSignature, "wrong signature");

	if (abs (1_ms * extracted.unix_timestamp_ms - neutrino::TimeHelper::utc_now()) > _max_time_difference)
		throw Exception (ErrorCode::DeltaTimeTooHigh, "delta time too high");

	if (extracted.handshake_id != _handshake_id)
		throw Exception (ErrorCode::InvalidHandshakeID, "invalid handshake ID");

	return extracted;
}


HandshakeSlave::HandshakeSlave (boost::random::random_device& rnd,
                                Params const& params,
                                IDReuseCheckCallback const id_reuse_check_callback):
    Handshake (rnd, params),
    _id_used_before (id_reuse_check_callback)
{ }


HandshakeSlave::HandshakeAndKey
HandshakeSlave::generate_handshake_blob_and_key (BlobView const master_handshake_blob, si::Time const unix_timestamp)
{
	auto const master_handshake = parse_and_verify_master_handshake_blob (master_handshake_blob);

	// Handshake receiver should verify that new handshake has never been used before,
	// to prevent replay attacks. Only correct handshakes should be checked,
	// otherwise we'd be vulnerable to DoS attacks.
	if (_id_used_before && _id_used_before (master_handshake.handshake_id))
		throw Exception (ErrorCode::ReusedHandshakeID, "reusing handshake ID");

	if (abs (1_ms * master_handshake.unix_timestamp_ms - neutrino::TimeHelper::utc_now()) > _max_time_difference)
		throw Exception (ErrorCode::DeltaTimeTooHigh, "delta time too high");

	Blob const dhe_exchange_blob = _dhe_exchange.generate_exchange_blob();
	Blob const ephemeral_key_with_weak_bits = _dhe_exchange.calculate_key_with_weak_bits (master_handshake.dhe_exchange_blob);
	Blob const ephemeral_key = calculate_hash<kHashAlgorithm> (ephemeral_key_with_weak_bits);

	return {
		.handshake_response = make_slave_handshake_blob ({
			.handshake_id = master_handshake.handshake_id,
			.unix_timestamp_ms = round_to<uint64_t> (unix_timestamp.in<si::Millisecond>()),
			.dhe_exchange_blob = dhe_exchange_blob,
		}),
		.ephemeral_key = ephemeral_key,
	};
}


Blob
HandshakeSlave::make_slave_handshake_blob (SlaveHandshake const& slave_handshake)
{
	Blob const salt = random_blob (8, _random_device);
	Blob const handshake_id_blob = to_blob (slave_handshake.handshake_id);
	Blob const handshake_data = salt + handshake_id_blob + to_blob (slave_handshake.unix_timestamp_ms) + slave_handshake.dhe_exchange_blob;
	Blob const signature = calculate_hmac<kHashAlgorithm> ({ .data = handshake_data, .key = *_slave_signature_key }).substr (0, _hmac_size);

	return handshake_data + signature;
}


Handshake::MasterHandshake
HandshakeSlave::parse_and_verify_master_handshake_blob (BlobView const master_handshake)
{
	// Skip salt, handshake_id and timestamp:
	BlobView const extracted_dhe_exchange_blob_and_signature = master_handshake.substr (24);
	BlobView extracted_dhe_exchange_blob = extracted_dhe_exchange_blob_and_signature;
	BlobView extracted_signature = extracted_dhe_exchange_blob_and_signature;
	extracted_dhe_exchange_blob.remove_suffix (_hmac_size);
	extracted_signature.remove_prefix (extracted_dhe_exchange_blob.size());

	BlobView const extracted_handshake_data = master_handshake.substr (0, master_handshake.size() - _hmac_size);
	Blob const calculated_signature = calculate_hmac<kHashAlgorithm> ({ .data = extracted_handshake_data, .key = *_master_signature_key }).substr (0, _hmac_size);

	MasterHandshake extracted;
	extracted.handshake_id = neutrino::parse<decltype (extracted.handshake_id)> (master_handshake.substr (8, 8));
	extracted.unix_timestamp_ms = neutrino::parse<decltype (extracted.unix_timestamp_ms)> (master_handshake.substr (16, 8));
	extracted.dhe_exchange_blob = extracted_dhe_exchange_blob;

	if (extracted_signature != calculated_signature)
		throw Exception (ErrorCode::WrongSignature, "wrong signature");

	return extracted;
}

} // namespace xf::crypto::xle

