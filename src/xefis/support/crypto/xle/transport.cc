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

// Neutrino:
#include <neutrino/crypto/aes.h>
#include <neutrino/crypto/hkdf.h>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "transport.h"


namespace xf::crypto::xle {

Transport::Transport (BlobView const ephemeral_session_key, size_t const hmac_size, BlobView const salt, BlobView const hkdf_user_info):
	_hmac_size (hmac_size)
{
	_hmac_key = calculate_hkdf<kHashAlgorithm> ({
		.salt = salt,
		.key_material = ephemeral_session_key,
		.info = Blob (hkdf_user_info) + value_to_blob ("hmac_key"),
		.result_length = 16,
	});

	_data_encryption_key = calculate_hkdf<kHashAlgorithm> ({
		.salt = salt,
		.key_material = ephemeral_session_key,
		.info = hkdf_user_info + value_to_blob ("data_encryption_key"),
		.result_length = 16,
	});

	_seq_num_encryption_key = calculate_hkdf<kHashAlgorithm> ({
		.salt = salt,
		.key_material = ephemeral_session_key,
		.info = hkdf_user_info + value_to_blob ("seq_num_encryption_key"),
		.result_length = 16,
	});
}


Blob
Transmitter::encrypt_packet (BlobView const data)
{
	++_sequence_number;

	auto const binary_sequence_number = value_to_blob (_sequence_number);
	auto const full_hmac = calculate_hmac<kHashAlgorithm> ({
		.data = data + binary_sequence_number,
		.key = _hmac_key,
	});

	if (full_hmac.size() < _hmac_size)
		throw std::logic_error ("HMAC size doesn't fit requirements");

	auto const hmac = full_hmac.substr (0, _hmac_size);
	auto const encrypted_data = aes_ctr_mode_xor ({
		.data = data + hmac,
		.key = _data_encryption_key,
		.nonce = binary_sequence_number.substr (0, 8),
	});
	auto const encrypted_sequence_number = aes_ctr_mode_xor ({
		.data = binary_sequence_number,
		.key = _seq_num_encryption_key,
		.nonce = encrypted_data.substr (0, 8),
	});
	auto const encrypted_packet = encrypted_sequence_number + encrypted_data;

	++_sequence_number;

	return encrypted_packet;
}


Blob
Receiver::decrypt_packet (BlobView const encrypted_packet, std::optional<SequenceNumber> const maximum_allowed_sequence_number)
{
	auto const encrypted_sequence_number = encrypted_packet.substr (0, sizeof (_sequence_number));
	auto const encrypted_data = encrypted_packet.substr (sizeof (_sequence_number));
	auto const binary_sequence_number = aes_ctr_mode_xor ({
		.data = encrypted_sequence_number,
		.key = _seq_num_encryption_key,
		.nonce = encrypted_data.substr (0, 8),
	});
	auto const data_with_hmac = aes_ctr_mode_xor ({
		.data = encrypted_data,
		.key = _data_encryption_key,
		.nonce = binary_sequence_number.substr (0, 8),
	});

	if (data_with_hmac.size() >= _hmac_size)
	{
		auto data = BlobView (data_with_hmac);
		auto hmac = BlobView (data_with_hmac);
		data.remove_suffix (_hmac_size);
		hmac.remove_prefix (data.size());

		auto const calculated_full_hmac = calculate_hmac<kHashAlgorithm> ({
			.data = data + binary_sequence_number,
			.key = _hmac_key,
		});
		auto calculated_hmac = BlobView (calculated_full_hmac).substr (0, _hmac_size);

		if (calculated_hmac == hmac)
		{
			SequenceNumber sequence_number;
			blob_to_value (binary_sequence_number, sequence_number);

			if (sequence_number <= _sequence_number)
				throw DecryptionFailure ("sequence number from past is invalid");

			if (maximum_allowed_sequence_number)
				if (sequence_number > *maximum_allowed_sequence_number)
					throw DecryptionFailure ("sequence number from far future is invalid");

			_sequence_number = sequence_number;

			return Blob (data);
		}
		else
			throw DecryptionFailure ("invalid authentication");
	}
	else
		throw DecryptionFailure ("HMAC too short");
}

} // namespace xf::crypto::xle

