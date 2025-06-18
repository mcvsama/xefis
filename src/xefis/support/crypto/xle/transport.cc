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
#include "transport.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/crypto/aes.h>
#include <neutrino/crypto/hkdf.h>
#include <neutrino/crypto/utility.h>

// Standard:
#include <cstddef>


namespace xf::crypto::xle {

Transport::Transport (Params const& params):
	_hmac_size (params.hmac_size)
{
	_hmac_key = compute_hkdf<kSignatureHMACHashAlgorithm> ({
		.salt = params.authentication_secret,
		.key_material = params.ephemeral_session_key,
		.info = params.hkdf_user_info + to_blob ("hmac_key"),
		.result_length = 32,
	});

	_data_encryption_key = compute_hkdf<kDataEncryptionKeyHKDFHashAlgorithm> ({
		.salt = params.data_encryption_secret,
		.key_material = params.ephemeral_session_key,
		.info = params.hkdf_user_info + to_blob ("data_encryption_key"),
		.result_length = 32,
	});

	_seq_num_encryption_key = compute_hkdf<kSeqNumEncryptionKeyHKDFHashAlgorithm> ({
		.salt = params.seq_num_encryption_secret,
		.key_material = params.ephemeral_session_key,
		.info = params.hkdf_user_info + to_blob ("seq_num_encryption_key"),
		.result_length = 32,
	});
}


Transmitter::Transmitter (boost::random::random_device& random_device, Params const& params):
	Transport (params),
	_random_device (random_device)
{ }


/**
 * Encrypted packet structure:
 *
 * encrypted_packet
 * {
 *     encrypted_sequence_number (8 B);
 *     encrypted_data (variable length + kDataSaltSize + _hmac_size B)
 *     {
 *         data (variable length);
 *         random salt (kDataSaltSize B);
 *         hmac (_hmac_size B);
 *     };
 * };
 */
Blob
Transmitter::encrypt_packet (BlobView const data)
{
	++_sequence_number;

	// It's required that to_blob() gives little-endian encoding:
	auto const binary_sequence_number = to_blob (_sequence_number);
	auto const salt = random_blob (kDataSaltSize, _random_device);
	auto const full_hmac = compute_hmac<kSignatureHMACHashAlgorithm> ({
		.data = data + salt + binary_sequence_number,
		.key = *_hmac_key,
	});

	if (full_hmac.size() < _hmac_size)
		throw std::logic_error ("HMAC size doesn't fit requirements");

	auto const hmac = full_hmac.substr (0, _hmac_size);
	auto const encrypted_data = aes_ctr_xor ({
		.data = data + salt + hmac,
		.key = *_data_encryption_key,
		.nonce = compute_hash<kDataNonceHashAlgorithm> (binary_sequence_number).substr (0, 8),
	});
	auto const encrypted_sequence_number = aes_ctr_xor ({
		.data = binary_sequence_number,
		.key = *_seq_num_encryption_key,
		// Encrypted data must be at least 8 bytes, but longer is better for better entropy to avoid
		// repeating nonce ever. That's why data salt is added before encryption.
		.nonce = compute_hash<kSeqNumNonceHashAlgorithm> (encrypted_data).substr (0, 8),
	});
	auto const encrypted_packet = encrypted_sequence_number + encrypted_data;

	return encrypted_packet;
}


Blob
Receiver::decrypt_packet (BlobView const encrypted_packet, std::optional<SequenceNumber> const maximum_allowed_sequence_number)
{
	auto const encrypted_sequence_number = encrypted_packet.substr (0, sizeof (_sequence_number));
	auto const encrypted_data = encrypted_packet.substr (sizeof (_sequence_number));
	auto const binary_sequence_number = aes_ctr_xor ({
		.data = encrypted_sequence_number,
		.key = *_seq_num_encryption_key,
		.nonce = compute_hash<kSeqNumNonceHashAlgorithm> (encrypted_data).substr (0, 8),
	});
	auto const data_with_hmac = aes_ctr_xor ({
		.data = encrypted_data,
		.key = *_data_encryption_key,
		.nonce = compute_hash<kDataNonceHashAlgorithm> (binary_sequence_number).substr (0, 8),
	});

	if (data_with_hmac.size() >= kDataSaltSize + _hmac_size)
	{
		auto data = BlobView (data_with_hmac);
		auto salt = data;
		auto hmac = data;
		data.remove_suffix (kDataSaltSize + _hmac_size);
		salt.remove_prefix (data.size());
		salt.remove_suffix (_hmac_size);
		hmac.remove_prefix (data.size() + kDataSaltSize);

		auto const computed_full_hmac = compute_hmac<kSignatureHMACHashAlgorithm> ({
			.data = Blob (data) + salt + binary_sequence_number,
			.key = *_hmac_key,
		});
		auto computed_hmac = BlobView (computed_full_hmac).substr (0, _hmac_size);

		if (computed_hmac == hmac)
		{
			auto const sequence_number = parse<SequenceNumber> (binary_sequence_number);

			if (sequence_number <= _sequence_number)
				throw DecryptionFailure (ErrorCode::SeqNumFromPast, "sequence number from past is invalid");

			if (maximum_allowed_sequence_number)
				if (sequence_number > *maximum_allowed_sequence_number)
					throw DecryptionFailure (ErrorCode::SeqNumFromFarFuture, "sequence number from far future is invalid");

			_sequence_number = sequence_number;

			return Blob (data);
		}
		else
			throw DecryptionFailure (ErrorCode::InvalidAuthentication, "invalid authentication");
	}
	else
		throw DecryptionFailure (ErrorCode::HMACTooShort, "HMAC too short");
}

} // namespace xf::crypto::xle

