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

// Standard:
#include <cstddef>
#include <memory>
#include <random>

// Lib:
#include <boost/endian/conversion.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/qdom_iterator.h>
#include <xefis/utility/hextable.h>
#include <xefis/utility/hmac.h>
#include <xefis/utility/blob.h>

// Local:
#include "link.h"


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
LinkProtocol::Sequence::produce (Blob& blob)
{
	for (auto const& packet: _packets)
		packet->produce (blob);
}


Blob::const_iterator
LinkProtocol::Sequence::eat (Blob::const_iterator begin, Blob::const_iterator end)
{
	for (auto const& packet: _packets)
		begin = packet->eat (begin, end);

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
		std::visit ([&total_bits](auto&& bs) noexcept {
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
LinkProtocol::Bitfield::produce (Blob& blob)
{
	std::vector<bool> bits;
	bits.reserve (8 * size());

	for (auto const& bsvariant: _bit_sources)
	{
		std::visit ([&bits](auto&& bs) {
			uint_least64_t v = bs.fallback_value;

			if (bs.property && fits_in_bits (*bs.property, Bits (bs.bits)))
				v = *bs.property;

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
				byte |= 1 << k;

		blob.push_back (byte);
	}
}


Blob::const_iterator
LinkProtocol::Bitfield::eat (Blob::const_iterator begin, Blob::const_iterator end)
{
	if (std::distance (begin, end) < static_cast<Blob::difference_type> (size()))
		throw InsufficientDataError();

	std::vector<bool> bits;
	bits.reserve (8 * size());

	for (Blob::const_iterator cur = begin; cur < begin + size(); ++cur)
		for (uint8_t b = 0; b < 8; ++b)
			bits.push_back ((*cur >> b) & 1);

	std::vector<bool>::iterator bit = bits.begin();

	for (auto& bsvariant: _bit_sources)
	{
		std::visit ([&bit, &bits](auto&& bs) {
			uint_least64_t v = 0;

			for (uint8_t b = 0; b < bs.bits; ++b)
				if (*(bit + b))
					v |= 1 << b;

			bs.value = v;
			std::advance (bit, bs.bits);
		}, bsvariant);
	}

	return begin + size();
}


void
LinkProtocol::Bitfield::apply()
{
	for (auto& bsvariant: _bit_sources)
	{
		std::visit ([](auto&& bs) {
			bs.property = bs.value;
		}, bsvariant);
	}
}


void
LinkProtocol::Bitfield::failsafe()
{
	for (auto& bsvariant: _bit_sources)
	{
		std::visit ([](auto&& bs) {
			if (!bs.retained)
				bs.property.set_nil();
		}, bsvariant);
	}
}


LinkProtocol::Signature::Signature (NonceBytes nonce_bytes, SignatureBytes signature_bytes, Key key, PacketList packets):
	Sequence (packets),
	_nonce_bytes (*nonce_bytes),
	_signature_bytes (*signature_bytes),
	_key (*key),
	_random_device(),
	_rng (_random_device())
{
	_temp.reserve (size());
}


Blob::size_type
LinkProtocol::Signature::size() const
{
	return Sequence::size() + _nonce_bytes + _signature_bytes;
}


void
LinkProtocol::Signature::produce (Blob& blob)
{
	_temp.clear();

	// Add data:
	Sequence::produce (_temp);

	// Append nonce:
	std::uniform_int_distribution<uint8_t> dist;

	for (unsigned int i = 0; i < _nonce_bytes; ++i)
		_temp.push_back (dist (_rng));

	xf::HMAC hmac (xf::HMAC::Key (_key), _temp);

	// Add some of the bytes of HMAC signature:
	size_t hmac_bytes = std::min<size_t> (_signature_bytes, std::distance (hmac.begin(), hmac.end()));
	_temp.insert (_temp.end(), hmac.begin(), hmac.begin() + hmac_bytes);

	// Output:
	blob.insert (blob.end(), _temp.begin(), _temp.end());
}


Blob::const_iterator
LinkProtocol::Signature::eat (Blob::const_iterator begin, Blob::const_iterator end)
{
	auto const data_size = Sequence::size();
	auto const whole_size = size();

	if (std::distance (begin, end) < static_cast<Blob::difference_type> (whole_size))
		throw InsufficientDataError();

	auto const sign_begin = begin + data_size + _nonce_bytes;
	auto const sign_end = begin + whole_size;

	// Make a temporary copy of the data:
	_temp.resize (data_size + _nonce_bytes);
	std::copy (begin, sign_begin, _temp.begin());

	xf::HMAC hmac (xf::HMAC::Key (_key), _temp);

	// If HMACs differ, that's a parsing error:
	if (!std::equal (sign_begin, sign_end, hmac.begin()))
		throw ParseError();

	if (Sequence::eat (begin, begin + data_size) != begin + data_size)
		throw ParseError();

	return begin + whole_size;
}


LinkProtocol::Envelope::Envelope (Magic magic, PacketList packets):
	Sequence (packets),
	_magic (*magic)
{ }


LinkProtocol::Envelope::Envelope (Magic magic, SendEvery send_every, SendOffset send_offset, PacketList packets):
	Sequence (packets),
	_magic (*magic),
	_send_every (*send_every),
	_send_offset (*send_offset)
{ }


Blob const&
LinkProtocol::Envelope::magic() const
{
	return _magic;
}


Blob::size_type
LinkProtocol::Envelope::size() const
{
	return Sequence::size();
}


void
LinkProtocol::Envelope::produce (Blob& blob)
{
	if (_send_pos % _send_every == _send_offset)
	{
		blob.insert (blob.end(), _magic.begin(), _magic.end());
		Sequence::produce (blob);
	}

	++_send_pos;
}


LinkProtocol::LinkProtocol (EnvelopeList envelopes):
	_envelopes (envelopes)
{
	if (!_envelopes.empty())
	{
		_magic_size = _envelopes[0]->magic().size();

		for (auto const& e: _envelopes)
		{
			if (e->magic().size() != _magic_size)
				throw InvalidMagicSize();

			_envelope_magics[e->magic()] = e;
		}
	}
}


void
LinkProtocol::produce (Blob& blob)
{
	for (auto& e: _envelopes)
		e->produce (blob);

#if XEFIS_LINK_SEND_DEBUG
	log() << "Send: " << to_string (blob) << std::endl;
#endif
}


Blob::const_iterator
LinkProtocol::eat (Blob::const_iterator begin, Blob::const_iterator end, LinkIO* io, QTimer* reacquire_timer, QTimer* failsafe_timer)
{
#if XEFIS_LINK_RECV_DEBUG
	log() << "Recv: " << to_string (Blob (begin, end)) << std::endl;
#endif

	_aux_magic_buffer.resize (_magic_size);

	while (std::distance (begin, end) > static_cast<Blob::difference_type> (_magic_size + 1))
	{
		auto skip_byte_and_retry = [&]() {
			// Skip one byte and try again:
			if (std::distance (begin, end) >= 1)
				++begin;

			if (io)
				io->link_error_bytes = io->link_error_bytes.value_or (0) + 1;

			// Since there was an error, stop the reacquire timer:
			if (reacquire_timer)
				reacquire_timer->stop();
		};

		bool return_from_outer_function = false;
		Blob::const_iterator outer_result = begin;

		xf::Exception::guard ([&] {
			try {
				// Find the right magic and envelope:
				std::copy (begin, begin + _magic_size, _aux_magic_buffer.begin());
				auto envelope_and_magic = _envelope_magics.find (_aux_magic_buffer);

				// If not found, retry starting with next byte:
				if (envelope_and_magic == _envelope_magics.end())
					throw LinkProtocol::ParseError();

				auto envelope = envelope_and_magic->second;
				// Now see if we have enough data in input buffer for this envelope type.
				// If not, return and retry when enough data is read.
				if (static_cast<Blob::size_type> (std::distance (begin, end)) - _magic_size < envelope->size())
				{
					return_from_outer_function = true;
					outer_result = begin;
					return;
				}

				auto e = envelope->eat (begin + _magic_size, end);

				if (e != begin)
				{
					envelope->apply();
					begin = e;
				}

				if (io)
					io->link_valid_envelopes = io->link_valid_envelopes.value_or (0) + 1;

				// Restart failsafe timer:
				if (failsafe_timer)
					failsafe_timer->start();

				// If link is not valid, and we got valid envelope,
				// start reacquire timer:
				if (reacquire_timer && io)
					if (!io->link_valid.value_or (false) && !reacquire_timer->isActive())
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


std::string
LinkProtocol::to_string (Blob const& blob)
{
	if (blob.empty())
		return "";

	std::string s;

	for (auto v: blob)
		s += QString ("%1").arg (v, 2, 16, QChar ('0')).toStdString() + ":";

	s.pop_back();
	return s;
}


Link::Link (std::unique_ptr<LinkIO> module_io, std::unique_ptr<LinkProtocol> protocol, std::string const& instance):
	Module (std::move (module_io), instance),
	_protocol (std::move (protocol))
{
	_input_blob.reserve (2 * _protocol->size());
	_output_blob.reserve (2 * _protocol->size());

	_failsafe_timer = new QTimer (this);
	_failsafe_timer->setSingleShot (true);
	_failsafe_timer->setInterval (io.failsafe_after->quantity<Millisecond>());
	QObject::connect (_failsafe_timer, SIGNAL (timeout()), this, SLOT (failsafe()));

	_reacquire_timer = new QTimer (this);
	_reacquire_timer->setSingleShot (true);
	_reacquire_timer->setInterval (io.reacquire_after->quantity<Millisecond>());
	QObject::connect (_reacquire_timer, SIGNAL (timeout()), this, SLOT (reacquire()));

	_output_timer = new QTimer (this);
	_output_timer->setSingleShot (false);
	_output_timer->setInterval (1000_Hz / *io.send_frequency);
	QObject::connect (_output_timer, SIGNAL (timeout()), this, SLOT (send_output()));
	_output_timer->start();
}


void
Link::process (v2::Cycle const&)
{
	try {
		if (io.link_input && _input_changed())
		{
			_input_blob.insert (_input_blob.end(), io.link_input->begin(), io.link_input->end());
			auto e = _protocol->eat (_input_blob.begin(), _input_blob.end(), &io, _reacquire_timer, _failsafe_timer);
			auto valid_bytes = std::distance (_input_blob.cbegin(), e);
			io.link_valid_bytes = io.link_valid_bytes.value_or (0) + valid_bytes;
			_input_blob.erase (_input_blob.begin(), e);
		}
	}
	catch (LinkProtocol::ParseError const&)
	{
		log() << "Packet parse error. Couldn't synchronize." << std::endl;
	}
}


void
Link::send_output()
{
	_output_blob.clear();
	_protocol->produce (_output_blob);
	io.link_output = std::string (_output_blob.begin(), _output_blob.end());
}


void
Link::failsafe()
{
	io.link_valid = false;
	io.link_failsafes = io.link_failsafes.value_or (0) + 1;
	_protocol->failsafe();
}


void
Link::reacquire()
{
	io.link_valid = true;
	io.link_reacquires = io.link_reacquires.value_or (0) + 1;
}

