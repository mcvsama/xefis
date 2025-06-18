/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
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
#include "link_protocol.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/modules/comm/link/input_link.h>
#include <xefis/utility/hextable.h>

// Neutrino:
#include <neutrino/blob.h>
#include <neutrino/crypto/hmac.h>
#include <neutrino/qt/qdom.h>
#include <neutrino/qt/qdom_iterator.h>
#include <neutrino/exception_support.h>
#include <neutrino/stdexcept.h>
#include <neutrino/string.h>

// Lib:
#include <boost/endian/conversion.hpp>

// Standard:
#include <cstddef>
#include <memory>
#include <random>


using namespace neutrino::si::literals;


LinkProtocol::Sequence::Sequence (PacketList packets):
	_packets (packets)
{ }


Blob::size_type
LinkProtocol::Sequence::size() const
{
	Blob::size_type s = 0;

	for (auto const& packet: _packets)
		s += packet->size();

	return s;
}


void
LinkProtocol::Sequence::produce (Blob& blob, xf::Logger const& logger)
{
	for (auto const& packet: _packets)
		packet->produce (blob, logger);
}


Blob::const_iterator
LinkProtocol::Sequence::consume (Blob::const_iterator begin, Blob::const_iterator end, xf::Logger const& logger)
{
	for (auto const& packet: _packets)
		begin = packet->consume (begin, end, logger);

	return begin;
}


void
LinkProtocol::Sequence::apply()
{
	for (auto const& packet: _packets)
		packet->apply();
}


void
LinkProtocol::Sequence::failsafe()
{
	for (auto const& packet: _packets)
		packet->failsafe();
}


LinkProtocol::Bitfield::Bitfield (std::initializer_list<SourceVariant> sources):
	_bit_sources (sources)
{
	Blob::size_type total_bits = 0;

	for (auto const& bsvariant: _bit_sources)
	{
		std::visit ([&total_bits] (auto&& bs) noexcept {
			total_bits += bs.bits;
		}, bsvariant);
	}

	_size = (total_bits + 7) / 8;
}


Blob::size_type
LinkProtocol::Bitfield::size() const
{
	return _size;
}


void
LinkProtocol::Bitfield::produce (Blob& blob, xf::Logger const&)
{
	std::vector<bool> bits;
	bits.reserve (8 * size());

	for (auto const& bsvariant: _bit_sources)
	{
		std::visit ([&bits] (auto&& bs) {
			uint_least64_t v = bs.value_if_nil;

			if (bs.socket && fits_in_bits (*bs.socket, bs.bits))
				v = *bs.socket;
			// TODO signal error if doesn't fits_in_bits()?

			for (uint8_t b = 0; b < bs.bits; ++b)
				bits.push_back ((v >> b) & 1);
		}, bsvariant);
	}

	bits.resize (8 * size(), 0);

	for (std::vector<bool>::size_type b = 0; b < bits.size(); b += 8)
	{
		uint8_t byte = 0;

		for (std::vector<bool>::size_type k = 0; k < 8; ++k)
			if (bits[k + b])
				byte |= 1u << k;

		blob.push_back (byte);
	}
}


Blob::const_iterator
LinkProtocol::Bitfield::consume (Blob::const_iterator begin, Blob::const_iterator end, xf::Logger const&)
{
	if (std::distance (begin, end) < static_cast<Blob::difference_type> (size()))
		throw InsufficientDataError();

	std::vector<bool> bits;
	bits.reserve (8 * size());

	for (Blob::const_iterator cur = begin; cur < begin + neutrino::to_signed (size()); ++cur)
		for (uint8_t b = 0; b < 8; ++b)
			bits.push_back ((*cur >> b) & 1);

	std::vector<bool>::iterator bit = bits.begin();

	for (auto& bsvariant: _bit_sources)
	{
		std::visit ([&bit, &bits] (auto&& bs) {
			uint_least64_t v = 0;

			for (uint8_t b = 0; b < bs.bits; ++b)
				if (*(bit + b))
					v |= 1u << b;

			bs.value = v;
			std::advance (bit, bs.bits);
		}, bsvariant);
	}

	return begin + neutrino::to_signed (size());
}


void
LinkProtocol::Bitfield::apply()
{
	for (auto& bsvariant: _bit_sources)
	{
		std::visit ([](auto&& bs) {
			if (bs.assignable_socket)
				*bs.assignable_socket = bs.value;
		}, bsvariant);
	}
}


void
LinkProtocol::Bitfield::failsafe()
{
	for (auto& bsvariant: _bit_sources)
	{
		std::visit ([](auto&& bs) {
			if (bs.assignable_socket && !bs.retained)
				*bs.assignable_socket = xf::nil;
		}, bsvariant);
	}
}


LinkProtocol::Signature::Signature (Params&& params):
	Sequence (params.packets),
	_name (params.name),
	_nonce_bytes (params.nonce_bytes),
	_signature_bytes (params.signature_bytes),
	_key (params.key),
	_rng (std::random_device{}())
{
	_temp.reserve (size());
}


Blob::size_type
LinkProtocol::Signature::size() const
{
	return Sequence::size() + _nonce_bytes + _signature_bytes;
}


void
LinkProtocol::Signature::produce (Blob& blob, xf::Logger const& logger)
{
	_temp.clear();

	// Add data:
	Sequence::produce (_temp, logger);

	// Append nonce:
	std::uniform_int_distribution<uint8_t> distribution;

	for (unsigned int i = 0; i < _nonce_bytes; ++i)
		_temp.push_back (distribution (_rng));

	auto const hmac = xf::compute_hmac<xf::Hash::SHA3_256> ({ .data = _temp, .key = _key });
	// Add some of the bytes of HMAC signature:
	size_t hmac_bytes = std::min<size_t> (_signature_bytes, hmac.size());
	_temp += hmac.substr (0, hmac_bytes);

	// Output:
	blob += _temp;
}


Blob::const_iterator
LinkProtocol::Signature::consume (Blob::const_iterator begin, Blob::const_iterator end, xf::Logger const& logger)
{
	auto const data_size = Sequence::size();
	auto const whole_size = size();

	if (std::distance (begin, end) < static_cast<Blob::difference_type> (whole_size))
		throw InsufficientDataError();

	auto const sign_begin = begin + neutrino::to_signed (data_size + _nonce_bytes);
	auto const sign_end = begin + neutrino::to_signed (whole_size);

	// Make a temporary copy of the data:
	_temp.resize (data_size + _nonce_bytes);
	std::copy (begin, sign_begin, _temp.begin());

	auto const hmac = xf::compute_hmac<xf::Hash::SHA3_256> ({ .data = _temp, .key = _key });

	// If HMACs differ, it's a parsing error:
	if (!std::equal (sign_begin, sign_end, hmac.begin()))
		throw ParseError();

	auto const consuming_end = begin + neutrino::to_signed (data_size);

	if (Sequence::consume (begin, consuming_end, logger) != consuming_end)
		throw ParseError();

	return begin + neutrino::to_signed (whole_size);
}


LinkProtocol::Envelope::Envelope (Params&& params):
	Sequence (params.packets),
	_name (params.name),
	_unique_prefix (params.unique_prefix),
	_send_every (params.send_every),
	_send_offset (params.send_offset),
	_send_predicate (params.send_predicate),
	_transceiver (params.transceiver)
{ }


Blob const&
LinkProtocol::Envelope::unique_prefix() const
{
	return _unique_prefix;
}


Blob::size_type
LinkProtocol::Envelope::size() const
{
	if (_transceiver)
		return Sequence::size() + _transceiver->ciphertext_expansion();
	else
		return Sequence::size();
}


void
LinkProtocol::Envelope::produce (Blob& blob, xf::Logger const& logger)
{
	if (!_send_predicate || _send_predicate())
	{
		if (_send_pos % _send_every == _send_offset)
		{
			if (_transceiver)
			{
				try {
					if (_transceiver->ready())
					{
						Blob unencrypted_blob;
						Sequence::produce (unencrypted_blob, logger);
						blob += _unique_prefix + _transceiver->encrypt_packet (unencrypted_blob);
					}
				}
				catch (...)
				{
					logger << "Could not produce envelope: " << neutrino::describe_exception (std::current_exception()) << "\n";
					// Do not produce anything if encryption fails.
				}
			}
			else
			{
				blob += _unique_prefix;
				Sequence::produce (blob, logger);
			}
		}

		++_send_pos;
	}
}


Blob::const_iterator
LinkProtocol::Envelope::consume (Blob::const_iterator begin, Blob::const_iterator end, xf::Logger const& logger)
{
	if (_transceiver)
	{
		auto const envelope_end = std::next (begin, neutrino::to_signed (size()));

		try {
			if (_transceiver->ready())
			{
				auto const decrypted = _transceiver->decrypt_packet (BlobView (begin, envelope_end));
				auto read_iterator = Sequence::consume (decrypted.begin(), decrypted.end(), logger);

				if (read_iterator != decrypted.end())
					throw xf::Exception ("Envelope::consume(): not all data consumed by the envelope after decryption");
			}
		}
		catch (...)
		{
			logger << "Could not consume envelope: " << neutrino::describe_exception (std::current_exception()) << "\n";
			// TODO Maybe each envelope should have its own failsafe timer?
			failsafe();
		}

		return envelope_end;
	}
	else
		return Sequence::consume (begin, end, logger);
}


LinkProtocol::LinkProtocol (EnvelopeList envelopes):
	_envelopes (envelopes)
{
	if (!_envelopes.empty())
	{
		_unique_prefix_size = _envelopes[0]->unique_prefix().size();

		for (auto const& e: _envelopes)
		{
			if (e->unique_prefix().size() != _unique_prefix_size)
				throw InvalidMagicSize();

			_envelope_unique_prefixes[e->unique_prefix()] = e;
		}
	}
}


void
LinkProtocol::produce (Blob& blob, [[maybe_unused]] xf::Logger const& logger)
{
	for (auto& e: _envelopes)
		e->produce (blob, logger);

#if XEFIS_LINK_SEND_DEBUG
	logger << "Send: " << neutrino::to_hex_string (blob, ":") << std::endl;
#endif
}


Blob::const_iterator
LinkProtocol::consume (Blob::const_iterator begin,
					   Blob::const_iterator end,
					   InputLink* input_link,
					   QTimer* reacquire_timer,
					   QTimer* failsafe_timer,
					   xf::Logger const& logger)
{
#if XEFIS_LINK_RECV_DEBUG
	logger << "Recv: " << neutrino::to_hex_string (BlobView (begin, end), ":") << std::endl;
#endif

	_aux_unique_prefix_buffer.resize (_unique_prefix_size);

	while (std::distance (begin, end) > static_cast<Blob::difference_type> (_unique_prefix_size + 1))
	{
		auto skip_byte_and_retry = [&]() {
			// Skip one byte and try again:
			if (std::distance (begin, end) >= 1)
				++begin;

			if (input_link)
				input_link->link_error_bytes = input_link->link_error_bytes.value_or (0) + 1;

			// Since there was an error, stop the reacquire timer:
			if (reacquire_timer)
				reacquire_timer->stop();
		};

		bool return_from_outer_function = false;
		Blob::const_iterator outer_result = begin;

		xf::Exception::catch_and_log (logger, [&] {
			try {
				// Find the right unique_prefix and envelope:
				std::copy (begin, begin + neutrino::to_signed (_unique_prefix_size), _aux_unique_prefix_buffer.begin());
				auto envelope_and_unique_prefix = _envelope_unique_prefixes.find (_aux_unique_prefix_buffer);

				// If not found, retry starting with next byte:
				if (envelope_and_unique_prefix == _envelope_unique_prefixes.end())
					throw LinkProtocol::ParseError();

				auto envelope = envelope_and_unique_prefix->second;
				// Now see if we have enough data in input buffer for this envelope type.
				// If not, return and retry when enough data is read.
				if (neutrino::to_unsigned (std::distance (begin, end)) - _unique_prefix_size < envelope->size())
				{
					return_from_outer_function = true;
					outer_result = begin;
					return;
				}

				auto e = envelope->consume (begin + neutrino::to_signed (_unique_prefix_size), end, logger);

				if (e != begin)
				{
					envelope->apply();
					begin = e;
				}

				if (input_link)
					input_link->link_valid_envelopes = input_link->link_valid_envelopes.value_or (0) + 1;

				// Restart failsafe timer:
				if (failsafe_timer)
					failsafe_timer->start();

				// If input_link is not valid, and we got valid envelope,
				// start reacquire timer:
				if (reacquire_timer && input_link)
					if (!input_link->link_valid.value_or (false) && !reacquire_timer->isActive())
						reacquire_timer->start();
			}
			catch (LinkProtocol::ParseError&)
			{
				skip_byte_and_retry();
			}
			catch (...)
			{
				skip_byte_and_retry();
				throw;
			}
		});

		if (return_from_outer_function)
			return outer_result;
	}

	return begin;
}


Blob::size_type
LinkProtocol::size() const
{
	Blob::size_type s = 0;

	for (auto p: _envelopes)
		s += p->size();

	return s;
}


void
LinkProtocol::failsafe()
{
	for (auto& e: _envelopes)
		e->failsafe();
}

