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

// Standard:
#include <cstddef>
#include <limits>

// Boost:
#include <boost/random.hpp>

// Neutrino:
#include <neutrino/crypto/hmac.h>
#include <neutrino/crypto/modp.h>
#include <neutrino/crypto/utility.h>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "handshake.h"


namespace xf::crypto::xle {

Handshake::Handshake (boost::random::random_device& random_device, BlobView const master_signature_key, BlobView const slave_signature_key, size_t hmac_size):
	_random_device (random_device),
	_master_signature_key (master_signature_key),
	_slave_signature_key (slave_signature_key),
	_dhe_exchange (random_device, RFC3526_Group14<DiffieHellmanExchange::Integer>),
	_hmac_size (hmac_size)
{ }


Blob
HandshakeMaster::generate_handshake_blob()
{
	using HandshakeIDLimits = std::numeric_limits<HandshakeID>;

	auto handshake_id_distribution = boost::random::uniform_int_distribution<HandshakeID> (HandshakeIDLimits::min(), HandshakeIDLimits::max());

	return make_master_handshake_blob ({
		.handshake_id = handshake_id_distribution (_random_device),
		.unix_timestamp_ms = 0, // TODO
		.dhe_exchange_blob = _dhe_exchange.generate_exchange_blob(),
	});
}


Blob
HandshakeMaster::calculate_key (BlobView const slave_handshake_blob)
{
	auto const slave_handshake = parse_and_verify_slave_handshake_blob (slave_handshake_blob);
	// TODO handle error_code
	Blob const ephemeral_key_with_weak_bits = _dhe_exchange.calculate_key_with_weak_bits (slave_handshake.dhe_exchange_blob);
	Blob const ephemeral_key = calculate_hash<kHashAlgorithm> (ephemeral_key_with_weak_bits);

	return ephemeral_key;
}


Blob
HandshakeMaster::make_master_handshake_blob (MasterHandshake const& master_handshake)
{
	Blob const salt = random_blob (8, _random_device);
	Blob const handshake_id_blob = value_to_blob (master_handshake.handshake_id);
	Blob const handshake_data = salt + handshake_id_blob + value_to_blob (master_handshake.unix_timestamp_ms) + master_handshake.dhe_exchange_blob;
	Blob const signature = calculate_hmac<kHashAlgorithm> ({ .data = handshake_data, .key = _master_signature_key }).substr (0, _hmac_size);

	return handshake_data + signature;
}


Handshake::SlaveHandshake
HandshakeMaster::parse_and_verify_slave_handshake_blob (BlobView const slave_handshake_blob)
{
	BlobView const extracted_dhe_exchange_blob_and_signature = slave_handshake_blob.substr (25);
	BlobView extracted_dhe_exchange_blob = extracted_dhe_exchange_blob_and_signature;
	BlobView extracted_signature = extracted_dhe_exchange_blob_and_signature;
	extracted_dhe_exchange_blob.remove_suffix (_hmac_size);
	extracted_signature.remove_prefix (extracted_dhe_exchange_blob.size());

	BlobView const extracted_handshake_data = slave_handshake_blob.substr (0, slave_handshake_blob.size() - _hmac_size);
	Blob const calculated_signature = calculate_hmac<kHashAlgorithm> ({ .data = extracted_handshake_data, .key = _slave_signature_key }).substr (0, _hmac_size);

	SlaveHandshake extracted;
	blob_to_value (slave_handshake_blob.substr (8, 8), extracted.handshake_id);
	blob_to_value (slave_handshake_blob.substr (16, 8), extracted.unix_timestamp_ms);
	extracted.error_code = slave_handshake_blob[24];
	extracted.dhe_exchange_blob = extracted_dhe_exchange_blob;

	if (extracted_signature != calculated_signature)
		throw WrongSignature();

	return extracted;
}


HandshakeSlave::HandshakeAndKey
HandshakeSlave::generate_handshake_blob_and_key (BlobView const initial_handshake_blob)
{
	auto const initial_handshake = parse_and_verify_master_handshake_blob (initial_handshake_blob);

	// Handshake receiver should verify that new handshake has never been used before,
	// to prevent replay attacks.
	if (_id_used_before && _id_used_before (initial_handshake.handshake_id))
		throw ReusedHandshakeID();

	Blob const dhe_exchange_blob = _dhe_exchange.generate_exchange_blob();
	// TODO check initial_handshake.unix_timestamp_ms
	Blob const ephemeral_key_with_weak_bits = _dhe_exchange.calculate_key_with_weak_bits (initial_handshake.dhe_exchange_blob);
	Blob const ephemeral_key = calculate_hash<kHashAlgorithm> (ephemeral_key_with_weak_bits);

	return {
		.handshake_response = make_slave_handshake_blob ({
			.handshake_id = initial_handshake.handshake_id,
			.unix_timestamp_ms = 0, // TODO
			.error_code = 0, // TODO
			.dhe_exchange_blob = dhe_exchange_blob,
		}),
		.ephemeral_key = ephemeral_key,
	};
}


Blob
HandshakeSlave::make_slave_handshake_blob (SlaveHandshake const& slave_handshake)
{
	Blob const salt = random_blob (8, _random_device);
	Blob const handshake_id_blob = value_to_blob (slave_handshake.handshake_id);
	Blob const handshake_data = salt + handshake_id_blob + value_to_blob (slave_handshake.unix_timestamp_ms) + slave_handshake.error_code + slave_handshake.dhe_exchange_blob;
	Blob const signature = calculate_hmac<kHashAlgorithm> ({ .data = handshake_data, .key = _slave_signature_key }).substr (0, _hmac_size);

	return handshake_data + signature;
}


Handshake::MasterHandshake
HandshakeSlave::parse_and_verify_master_handshake_blob (BlobView const master_handshake)
{
	BlobView const extracted_dhe_exchange_blob_and_signature = master_handshake.substr (24);
	BlobView extracted_dhe_exchange_blob = extracted_dhe_exchange_blob_and_signature;
	BlobView extracted_signature = extracted_dhe_exchange_blob_and_signature;
	extracted_dhe_exchange_blob.remove_suffix (_hmac_size);
	extracted_signature.remove_prefix (extracted_dhe_exchange_blob.size());

	BlobView const extracted_handshake_data = master_handshake.substr (0, master_handshake.size() - _hmac_size);
	Blob const calculated_signature = calculate_hmac<kHashAlgorithm> ({ .data = extracted_handshake_data, .key = _master_signature_key }).substr (0, _hmac_size);

	MasterHandshake extracted;
	blob_to_value (master_handshake.substr (8, 8), extracted.handshake_id);
	blob_to_value (master_handshake.substr (16, 8), extracted.unix_timestamp_ms);
	extracted.dhe_exchange_blob = extracted_dhe_exchange_blob;

	if (extracted_signature != calculated_signature)
		throw WrongSignature();

	return extracted;
}

} // namespace xf::crypto::xle

