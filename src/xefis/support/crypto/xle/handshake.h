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

#ifndef XEFIS__SUPPORT__CRYPTO__XLE__HANDSHAKE_H__INCLUDED
#define XEFIS__SUPPORT__CRYPTO__XLE__HANDSHAKE_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>
#include <variant>

// Boost:
#include <boost/random/random_device.hpp>

// Neutrino:
#include <neutrino/crypto/diffie_hellman_exchange.h>
#include <neutrino/crypto/hash.h>

// Xefis:
#include <xefis/config/all.h>


namespace xf::crypto::xle {

using HandshakeID = uint64_t;


/**
 * Generates/parses handshake blobs and calculates final ephemeral key from
 * the handshake.
 */
class Handshake
{
  public:
	class WrongSignature: public std::runtime_error
	{
	  public:
		WrongSignature():
			std::runtime_error ("wrong signature")
		{ }
	};

	class ReusedHandshakeID: public std::runtime_error
	{
	  public:
		ReusedHandshakeID():
			std::runtime_error ("reusing handshake ID")
		{ }
	};

  protected:
	/**
	 * Serialized master handshake looks like this (all numbers are little-endian):
	 *
	 * {
	 *     salt (8 B)
	 *     handshake_id (8 B)
	 *     unix_timestamp_ms (8 B)
	 *     dhe_exchange (variable size)
	 *     signature (_hmac_size B)
	 * }
	 */
	class MasterHandshake
	{
	  public:
		HandshakeID	handshake_id;
		uint64_t	unix_timestamp_ms;
		Blob		dhe_exchange_blob;
	};

	/**
	 * Serialized slave handshake looks like this (all numbers are little-endian):
	 *
	 * {
	 *     salt (8 B)
	 *     handshake_id (8 B)
	 *     unix_timestamp_ms (8 B)
	 *     error_code (1 B)
	 *     dhe_exchange (variable size)
	 *     signature (_hmac_size B)
	 * }
	 */
	class SlaveHandshake
	{
	  public:
		HandshakeID	handshake_id;
		uint64_t	unix_timestamp_ms;
		uint8_t		error_code;
		Blob		dhe_exchange_blob;
	};

  public:
	// Ctor
	Handshake (boost::random::random_device&, BlobView master_signature_key, BlobView slave_signature_key, size_t hmac_size = 12);

  protected:
	static constexpr Hash::Algorithm const kHashAlgorithm = Hash::SHA3_256;

  protected:
	boost::random::random_device&	_random_device;
	Blob							_master_signature_key;
	Blob							_slave_signature_key;
	DiffieHellmanExchange			_dhe_exchange;
	size_t							_hmac_size;
};


class HandshakeMaster: public Handshake
{
  public:
	// Ctor
	using Handshake::Handshake;

	/**
	 * Generate new inital handshake blob to be sent to the other party.
	 */
	[[nodiscard]]
	Blob
	generate_handshake_blob();

	/**
	 * Return the ephemeral key to use for encryption.
	 * If key can't be calculated (eg. it was used before or the handshake is incorrect), return std::nullopt.
	 */
	[[nodiscard]]
	Blob
	calculate_key (BlobView slave_handshake_blob);

  private:
	[[nodiscard]]
	Blob
	make_master_handshake_blob (MasterHandshake const&);

	[[nodiscard]]
	SlaveHandshake
	parse_and_verify_slave_handshake_blob (BlobView);
};


class HandshakeSlave: public Handshake
{
  public:
	using IDUsedBeforeCallback = std::function<bool (HandshakeID)>;

	struct HandshakeAndKey
	{
		Blob	handshake_response;
		Blob	ephemeral_key;
	};

  public:
	// Ctor
	using Handshake::Handshake;

	/**
	 * Set function that tells if given handshake ID has been used before.
	 */
	void
	set_callback_for_handshake_id_used_before (IDUsedBeforeCallback const callback)
		{ _id_used_before = callback; }

	/**
	 * Generate handshake response blob to be sent to the initiator and the resulting
	 * ephemeral key to use for encryption.
	 */
	[[nodiscard]]
	HandshakeAndKey
	generate_handshake_blob_and_key (BlobView master_handshake_blob);

  private:
	[[nodiscard]]
	Blob
	make_slave_handshake_blob (SlaveHandshake const&);

	[[nodiscard]]
	MasterHandshake
	parse_and_verify_master_handshake_blob (BlobView);

  private:
	IDUsedBeforeCallback _id_used_before;
};

} // namespace xf::crypto::xle

#endif

