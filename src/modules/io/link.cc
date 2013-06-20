/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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

// Qt:
#include <QtXml/QDomElement>

// Lib:
#include <half/half.hpp>
#include <boost/endian/conversion.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/hash.h>

// Local:
#include "link.h"


XEFIS_REGISTER_MODULE_CLASS ("io/link", Link);


Link::ItemStream::ItemStream (Link* link, QDomElement& element)
{
	for (QDomElement& e: element)
	{
		if (e == "property")
			_items.push_back (new PropertyItem (link, e));
		else if (e == "bitfield")
			_items.push_back (new BitfieldItem (link, e));
		else if (e == "signature")
			_items.push_back (new SignatureItem (link, e));
	}
}


inline Link::Blob::size_type
Link::ItemStream::size() const
{
	Blob::size_type s = 0;
	for (Item* item: _items)
		s += item->size();
	return s;
}


inline void
Link::ItemStream::produce (Blob& blob)
{
	for (Item* item: _items)
		item->produce (blob);
}


inline Link::Blob::iterator
Link::ItemStream::eat (Blob::iterator begin, Blob::iterator end)
{
	for (Item* item: _items)
		begin = item->eat (begin, end);
	return begin;
}


void
Link::ItemStream::apply()
{
	for (Item* item: _items)
		item->apply();
}


Link::PropertyItem::PropertyItem (Link*, QDomElement& element)
{
	if (!element.hasAttribute ("type"))
		throw Xefis::Exception ("<property> needs attribute 'type'");

	QString type_attr = element.attribute ("type");
	if (type_attr == "float16")
		_type = Type::Float16;
	else if (type_attr == "float32")
		_type = Type::Float32;
	else if (type_attr == "float64")
		_type = Type::Float64;
	else if (type_attr == "int8")
		_type = Type::Int8;
	else if (type_attr == "int16")
		_type = Type::Int16;
	else if (type_attr == "int32")
		_type = Type::Int32;
	else if (type_attr == "int64")
		_type = Type::Int64;

	if (_type == Type::Unknown)
		throw Xefis::Exception ("unknown type: " + type_attr.toStdString());

	if (!element.hasAttribute ("path"))
		throw Xefis::Exception ("<property> needs attribute 'path'");

	_property.set_path (element.attribute ("path").toStdString());
}


inline Link::Blob::size_type
Link::PropertyItem::size() const
{
	switch (_type)
	{
		case Type::Int64:
		case Type::Float64:
			return 8;

		case Type::Int32:
		case Type::Float32:
			return 4;

		case Type::Int16:
		case Type::Float16:
			return 2;

		case Type::Int8:
			return 1;

		case Type::Unknown:
			// Impossible.
			break;
	}

	return 0;
}


void
Link::PropertyItem::produce (Blob& blob)
{
	using namespace boost::endian;

	switch (_type)
	{
		case Type::Int64:
			break;

		case Type::Int32:
			break;

		case Type::Int16:
			break;

		case Type::Int8:
			break;

		case Type::Float64:
		{
			double f64 = *_property;
			uint64_t* i64 = reinterpret_cast<uint64_t*> (&f64);
			native_to_little (*i64);
			for (int shift = 0; shift < 64; shift += 8)
				blob.push_back ((*i64 >> shift) & 0xff);
			break;
		}

		case Type::Float32:
		{
			float f32 = *_property;
			uint32_t* i32 = reinterpret_cast<uint32_t*> (&f32);
			native_to_little (*i32);
			for (int shift = 0; shift < 32; shift += 8)
				blob.push_back ((*i32 >> shift) & 0xff);
			break;
		}

		case Type::Float16:
		{
			half_float::half f16 (*_property);
			uint16_t* i16 = reinterpret_cast<uint16_t*> (&f16);
			native_to_little (*i16);
			for (int shift = 0; shift < 16; shift += 8)
				blob.push_back ((*i16 >> shift) & 0xff);
			break;
		}

		case Type::Unknown:
			// Impossible.
			break;
	}
}


Link::Blob::iterator
Link::PropertyItem::eat (Blob::iterator begin, Blob::iterator end)
{
	using namespace boost::endian;

	Blob::iterator result = begin;

	switch (_type)
	{
		case Type::Int64:
			break;

		case Type::Int32:
			break;

		case Type::Int16:
			break;

		case Type::Int8:
			break;

		case Type::Float64:
		{
			if (std::distance (begin, end) < 8)
				throw ParseError();

			double f64 = 0;
			uint64_t* i64 = reinterpret_cast<uint64_t*> (&f64);
			std::copy (begin, begin + 8, reinterpret_cast<uint8_t*> (i64));
			little_to_native (*i64);
			_value = f64;
			result = begin + 8;
			break;
		}

		case Type::Float32:
		{
			if (std::distance (begin, end) < 4)
				throw ParseError();

			float f32 = 0;
			uint32_t* i32 = reinterpret_cast<uint32_t*> (&f32);
			std::copy (begin, begin + 4, reinterpret_cast<uint8_t*> (i32));
			little_to_native (*i32);
			_value = f32;
			result = begin + 4;
			break;
		}

		case Type::Float16:
		{
			if (std::distance (begin, end) < 2)
				throw ParseError();

			half_float::half f16;
			uint16_t* i16 = reinterpret_cast<uint16_t*> (&f16);
			std::copy (begin, begin + 2, reinterpret_cast<uint8_t*> (i16));
			little_to_native (*i16);
			_value = f16;
			result = begin + 2;
			break;
		}

		case Type::Unknown:
			// Impossible.
			break;
	}

	return result;
}


void
Link::PropertyItem::apply()
{
	_property.write (_value);
}


Link::BitfieldItem::BitfieldItem (Link*, QDomElement&)
{
	// TODO
}


Link::Blob::size_type
Link::BitfieldItem::size() const
{
	// TODO
	return 0;
}


void
Link::BitfieldItem::produce (Blob&)
{
	// TODO
}


Link::Blob::iterator
Link::BitfieldItem::eat (Blob::iterator begin, Blob::iterator)
{
	return begin;
}


void
Link::BitfieldItem::apply()
{
	// TODO
}


Link::SignatureItem::SignatureItem (Link* link, QDomElement& element):
	ItemStream (link, element),
	_rd(),
	_rng (_rd())
{
	if (element.hasAttribute ("random-bytes"))
	{
		int i = element.attribute ("random-bytes").toInt();
		if (i >= 0)
			_random_bytes = i;
		else
			_random_bytes = 0;
	}

	if (element.hasAttribute ("signature-bytes"))
	{
		int i = element.attribute ("signature-bytes").toInt();
		if (i >= 0)
			_signature_bytes = i;
		else
			_signature_bytes = 0;
	}

	if (element.hasAttribute ("key"))
		_key = parse_binary_string (element.attribute ("key"));
	else
		_key = { 0 };

	_temp.reserve (size());
}


inline Link::Blob::size_type
Link::SignatureItem::size() const
{
	return ItemStream::size() + _random_bytes + _signature_bytes;
}


void
Link::SignatureItem::produce (Blob& blob)
{
	_temp.clear();
	ItemStream::produce (_temp);

	// Append random bytes:
	_temp.reserve (_temp.size() + _random_bytes);
	std::uniform_int_distribution<uint8_t> dist;
	for (unsigned int i = 0; i < _random_bytes; ++i)
		_temp.push_back (dist (_rng));
	// Append the key, compute signature:
	_temp.insert (_temp.end(), _key.begin(), _key.end());
	Hash hash (_temp);
	// Erase appended key:
	_temp.erase (_temp.end() - std::distance (_key.begin(), _key.end()), _temp.end());
	// Add signature:
	auto hash_end = hash.end();
	if (_signature_bytes > 0)
		hash_end = hash.begin() + _signature_bytes;
	if (hash_end > hash.end())
		hash_end = hash.end();
	_temp.insert (_temp.end(), hash.begin(), hash_end);

	// Output:
	blob.insert (blob.end(), _temp.begin(), _temp.end());
}


Link::Blob::iterator
Link::SignatureItem::eat (Blob::iterator begin, Blob::iterator end)
{
	Blob::size_type data_size = ItemStream::size();
	Blob::size_type whole_size = size();

	if (std::distance (begin, end) < static_cast<Blob::difference_type> (whole_size))
		throw ParseError();

	// First check the signature.
	Blob::iterator sign_begin = begin + data_size + _random_bytes;
	Blob::iterator sign_end = begin + whole_size;
	// Make a temporary copy of the data.
	_temp.resize (data_size + _random_bytes);
	std::copy (begin, sign_begin, _temp.begin());
	// Append the key, compute signature:
	_temp.insert (_temp.end(), _key.begin(), _key.end());
	Hash hash (_temp);
	// If hashes mismatch, that's parsing error:
	if (!std::equal (sign_begin, sign_end, hash.begin()))
		throw ParseError();

	if (ItemStream::eat (begin, begin + data_size) != begin + data_size)
		throw ParseError();

	return end;
}


Link::Packet::Packet (Link* link, QDomElement& element):
	ItemStream (link, element)
{
	if (!element.hasAttribute ("magic"))
		throw Xefis::Exception ("<packet> needs 'magic' attribute");

	_magic = parse_binary_string (element.attribute ("magic"));
	if (_magic.empty())
		throw Xefis::Exception ("magic value must have at least one byte length");
}


inline Link::Blob const&
Link::Packet::magic() const
{
	return _magic;
}


inline Link::Blob::size_type
Link::Packet::size() const
{
	return ItemStream::size();
}


void
Link::Packet::produce (Blob& blob)
{
	blob.insert (blob.end(), _magic.begin(), _magic.end());
	ItemStream::produce (blob);
}


Link::Link (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager)
{
	auto parse_input_output_config = [](QDomElement& input_output, QString& o_host, int& o_port) -> void
	{
		for (QDomElement& e: input_output)
		{
			if (e == "udp")
			{
				for (QDomElement& z: e)
				{
					if (z == "host")
						o_host = z.text();
					else if (z == "port")
						o_port = z.text().toInt();
					else
						throw Xefis::Exception (("invalid <output>/<udp> child: <" + z.tagName() + ">").toStdString());
				}
			}
			else
				throw Xefis::Exception (("invalid <output> child: <" + e.tagName() + ">").toStdString());
		}
	};

	for (QDomElement& e: config)
	{
		if (e == "properties")
		{
			parse_properties (e, {
				{ "link-valid", _link_valid, true },
				{ "errors", _errors, false },
				{ "valid-packets", _valid_packets, false },
				{ "failsafe.count", _failsafe_count, false },
				{ "failsafe.lost-time", _failsafe_lost_time, false },
				{ "failsafe.acquired-time", _failsafe_acquired_time, false },
			});
		}
		else if (e == "protocol")
			parse_protocol (e);
		else if (e == "input")
		{
			_input_interference = e.hasAttribute ("interference");
			parse_input_output_config (e, _udp_input_host, _udp_input_port);
			_udp_input = new QUdpSocket();
			_udp_input_enabled = true;
			_udp_input->bind (QHostAddress (_udp_input_host), _udp_input_port, QUdpSocket::ShareAddress);
			QObject::connect (_udp_input, SIGNAL (readyRead()), this, SLOT (got_udp_packet()));
		}
		else if (e == "output")
		{
			_output_interference = e.hasAttribute ("interference");
			parse_input_output_config (e, _udp_output_host, _udp_output_port);
			_udp_output = new QUdpSocket();
			_udp_output_enabled = true;
		}
	}

	_input_blob.reserve (2 * size());
	_output_blob.reserve (2 * size());

	_errors.set_default (0);
	_valid_packets.set_default (0);
}


Link::~Link()
{
	delete _udp_input;
	delete _udp_output;
}


void
Link::data_updated()
{
	_output_blob.clear();
	produce (_output_blob);

	if (_udp_output_enabled)
	{
#if XEFIS_LINK_SEND_DEBUG
		std::clog << "io/link:send: " << to_string (_output_blob) << std::endl;
#endif
		_udp_output->writeDatagram (reinterpret_cast<const char*> (&_output_blob[0]), _output_blob.size(), QHostAddress (_udp_output_host), _udp_output_port);
	}
}


void
Link::got_udp_packet()
{
	while (_udp_input->hasPendingDatagrams())
	{
		int datagram_size = _udp_input->pendingDatagramSize();
		if (_input_datagram.size() < datagram_size)
			_input_datagram.resize (datagram_size);

		_udp_input->readDatagram (_input_datagram.data(), datagram_size, nullptr, nullptr);
		_input_blob.insert (_input_blob.end(), _input_datagram.data(), _input_datagram.data() + _input_datagram.size());
	}

	if (_input_interference)
		interfere (_input_blob);

	eat (_input_blob);
}


inline Link::Blob::size_type
Link::size() const
{
	Blob::size_type s = 0;
	for (Packet* p: _packets)
		s += p->size();
	return s;
}


void
Link::produce (Blob& blob)
{
	for (Packet* p: _packets)
		p->produce (blob);

	if (_output_interference)
		interfere (blob);
}


void
Link::eat (Blob& blob)
{
	Blob _tmp_input_magic;
	_tmp_input_magic.resize (_magic_size);

	while (blob.size() > _magic_size + 1)
	{
		try {
			// Find the right magic and packet:
			std::copy (blob.begin(), blob.begin() + _magic_size, _tmp_input_magic.begin());
			PacketMagics::iterator packet_and_magic = _packet_magics.find (_tmp_input_magic);

			// If not found, retry starting with next byte:
			if (packet_and_magic == _packet_magics.end())
				throw ParseError();

			Packet* packet = packet_and_magic->second;
			// Now see if we have enough data in input buffer for this packet type.
			// If not, return and retry when enough data is read.
			if (blob.size() - _magic_size < packet->size())
				return;

			Blob::iterator e = packet->eat (blob.begin() + _magic_size, blob.end());
			blob.erase (blob.begin(), e);
			apply();

			_last_parse_was_valid = true;
			if (_valid_packets.valid())
				_valid_packets.write (*_valid_packets + 1);
		}
		catch (...)
		{
			// Skip one byte and try again:
			if (blob.size() >= 1)
				blob.erase (blob.begin(), blob.begin() + 1);
			if (_last_parse_was_valid && _errors.valid())
				_errors.write (*_errors + 1);
			_last_parse_was_valid = false;
		}
	}
}


void
Link::apply()
{
	for (Packet* p: _packets)
		p->apply();
}


void
Link::parse_protocol (QDomElement const& protocol)
{
	for (QDomElement& e: protocol)
	{
		if (e == "packet")
			_packets.push_back (new Packet (this, e));
	}

	_packet_magics.clear();
	// Ensure all packets have distinct magic values and the same size:
	for (Packet* p: _packets)
	{
		if (_magic_size == 0)
			_magic_size = p->magic().size();
		if (_magic_size != p->magic().size())
			throw Xefis::Exception ("all magic values have to have equal number of bytes");
		if (!_packet_magics.insert ({ p->magic(), p }).second)
			throw Xefis::Exception ("magic " + to_string (p->magic()) + " used for two or more packets");
	}
}


void
Link::interfere (Blob& blob)
{
	if (rand() % 3 == 0)
	{
		// Erase random byte from the input sequence:
		Blob::iterator i = blob.begin() + (rand() % blob.size());
		blob.erase (i, i + 1);
	}
}


Link::Blob
Link::parse_binary_string (QString const& string)
{
	enum State { MSB, LSB, Colon };

	auto from_xdigit = [&string](QChar& c) -> uint8_t
	{
		static HexTable hextable;

		char a = c.toLatin1();
		if (!std::isxdigit (a))
			throw Xefis::Exception ("invalid binary string: " + string.toStdString());
		return hextable[a];
	};

	Blob blob;
	blob.reserve ((string.size() + 1) / 3);
	State state = MSB;
	Blob::value_type val = 0;

	for (QChar c: string)
	{
		switch (state)
		{
			case MSB:
				val = 16 * from_xdigit (c);
				state = LSB;
				break;
			case LSB:
				val += from_xdigit (c);
				blob.push_back (val);
				state = Colon;
				break;
			case Colon:
				// Skip it:
				if (c != ':')
					throw Xefis::Exception ("invalid binary string: " + string.toStdString());
				state = MSB;
				break;
		}
	}
	// Must end with state Colon:
	if (state != Colon)
		throw Xefis::Exception ("invalid binary string: " + string.toStdString());
	return blob;
}


std::string
Link::to_string (Blob const& blob)
{
	std::string s;
	for (auto v: blob)
		s += QString ("%1").arg (v, 2, 16, QChar ('0')).toStdString() + ":";
	s.pop_back();
	return s;
}

