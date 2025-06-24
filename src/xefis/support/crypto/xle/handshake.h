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

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/crypto/diffie_hellman_exchange.h>
#include <neutrino/crypto/hash.h>
#include <neutrino/crypto/secure.h>
#include <neutrino/exception.h>

// Boost:
#include <boost/random/random_device.hpp>

// Standard:
#include <cstddef>
#include <functional>
#include <variant>


namespace xf::crypto::xle {

using HandshakeID = uint64_t;

/**
 * Generates/parses handshake blobs and computes final ephemeral key from
 * the handshake for master and slave ends. The resulting ephemeral key from
 * the DHE is hashed to get uniform pseudorandom distribution.
 */
class Handshake
{
  public:
	struct Params
	{
		BlobView	master_signature_key;
		BlobView	slave_signature_key;
		size_t		hmac_size				{ 12 };
		si::Time	max_time_difference		{ 60_s };
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
	 *     dhe_exchange (variable size)
	 *     signature (_hmac_size B)
	 * }
	 */
	class SlaveHandshake
	{
	  public:
		HandshakeID	handshake_id;
		uint64_t	unix_timestamp_ms;
		Blob		dhe_exchange_blob;
	};

  public:
	// Ctor
	Handshake (boost::random::random_device&, Params const&);

  protected:
	static constexpr nu::Hash::Algorithm const kHashAlgorithm = nu::Hash::SHA3_256;

  protected:
	boost::random::random_device&	_random_device;
	nu::Secure<Blob> const			_master_signature_key;
	nu::Secure<Blob> const			_slave_signature_key;
	nu::DiffieHellmanExchange		_dhe_exchange;
	size_t const					_hmac_size;
	si::Time const					_max_time_difference;
};


class HandshakeMaster: public Handshake
{
  public:
	enum class ErrorCode: uint8_t
	{
		WrongSignature,
		DeltaTimeTooHigh,
		InvalidHandshakeID,
	};

	class Exception: public nu::Exception
	{
	  public:
		Exception (ErrorCode, std::string_view message);

		ErrorCode
		error_code() const
			{ return _error_code; }

	  private:
		ErrorCode _error_code;
	};

  public:
	// Ctor
	using Handshake::Handshake;

	/**
	 * Generate new inital handshake blob to be sent to the other party.
	 */
	[[nodiscard]]
	Blob
	generate_handshake_blob (si::Time unix_timestamp);

	/**
	 * Return the ephemeral key to use for encryption.
	 * If key can't be computed (eg. it was used before or the handshake is incorrect),
	 * throws Exception with appropriate error code.
	 */
	[[nodiscard]]
	Blob
	compute_key (BlobView slave_handshake_blob);

  private:
	[[nodiscard]]
	Blob
	make_master_handshake_blob (MasterHandshake const&);

	[[nodiscard]]
	SlaveHandshake
	parse_and_verify_slave_handshake_blob (BlobView);

  private:
	HandshakeID _handshake_id;
};


class HandshakeSlave: public Handshake
{
  public:
	/**
     * This function should return true if and only if given handshake ID
     * has been used before. It should also store given key as 'used before'.
     */
	using IDReuseCheckCallback = std::function<bool (HandshakeID)>;//XXX

	// This function should store provided key in a storage (mark is at used).
	using StoreKeyFunction = std::function<void (HandshakeID)>;

	// This function should return true if provided key is present in the key storage.
	using ContainsKeyFunction = std::function<bool (HandshakeID)>;

	enum class ErrorCode: uint8_t
	{
		WrongSignature,
		ReusedHandshakeID,
		DeltaTimeTooHigh,
	};

	class Exception: public nu::Exception
	{
	  public:
		Exception (ErrorCode, std::string_view message);

		ErrorCode
		error_code() const
			{ return _error_code; }

	  private:
		ErrorCode _error_code;
	};

	struct HandshakeAndKey
	{
		nu::Secure<Blob>	handshake_response;
		nu::Secure<Blob>	ephemeral_key;
	};

	struct KeyCheckFunctions
	{
		StoreKeyFunction	store_key_function		{ nullptr };
		ContainsKeyFunction	contains_key_function	{ nullptr };
	};

  public:
	// Ctor
	HandshakeSlave (boost::random::random_device&, Params const&, KeyCheckFunctions);

	/**
	 * Generate handshake response blob to be sent to the initiator and the resulting
	 * ephemeral key to use for encryption.
	 */
	[[nodiscard]]
	HandshakeAndKey
	generate_handshake_blob_and_key (BlobView master_handshake_blob, si::Time unix_timestamp);

  private:
	[[nodiscard]]
	Blob
	make_slave_handshake_blob (SlaveHandshake const&);

	[[nodiscard]]
	MasterHandshake
	parse_and_verify_master_handshake_blob (BlobView);

  private:
	KeyCheckFunctions _key_check_callbacks;
};


inline
HandshakeMaster::Exception::Exception (ErrorCode const error_code, std::string_view const message):
	nu::Exception (message, false),
	_error_code (error_code)
{ }


inline
HandshakeSlave::Exception::Exception (ErrorCode const error_code, std::string_view const message):
	nu::Exception (message, false),
	_error_code (error_code)
{ }

} // namespace xf::crypto::xle

#endif

