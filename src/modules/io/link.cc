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


void
Link::ItemStream::failsafe()
{
	for (Item* item: _items)
		item->failsafe();
}


Link::PropertyItem::PropertyItem (Link*, QDomElement& element)
{
	if (!element.hasAttribute ("type"))
		throw Xefis::Exception ("<property> needs attribute 'type'");

	QString type_attr = element.attribute ("type");

	if (type_attr == "integer")
		_type = Type::Integer;
	else if (type_attr == "float")
		_type = Type::Float;
	else if (type_attr == "angle")
		_type = Type::Angle;
	else if (type_attr == "frequency")
		_type = Type::Frequency;
	else if (type_attr == "length")
		_type = Type::Length;
	else if (type_attr == "pressure")
		_type = Type::Pressure;
	else if (type_attr == "speed")
		_type = Type::Speed;
	else if (type_attr == "time")
		_type = Type::Time;

	if (_type == Type::Unknown)
		throw Xefis::Exception ("unknown type: " + type_attr.toStdString());

	switch (_type)
	{
		case Type::Integer:
		case Type::Float:
		case Type::Angle:
		case Type::Frequency:
		case Type::Length:
		case Type::Pressure:
		case Type::Speed:
		case Type::Time:
			if (!element.hasAttribute ("bytes"))
				throw Xefis::Exception (QString ("<property> of type %1 needs attribute 'bytes'").arg (element.attribute ("type")).toStdString());
		default:
			;
	}

	if (element.hasAttribute ("bytes"))
	{
		_bytes = element.attribute ("bytes").toUInt();
		switch (_type)
		{
			case Type::Integer:
				if (_bytes != 1 && _bytes != 2 && _bytes != 4 && _bytes != 8)
					throw Xefis::Exception (QString ("invalid 'bytes' attribute %1, should be 1, 2, 4 or 8").arg (_bytes).toStdString());
			case Type::Float:
			case Type::Angle:
			case Type::Frequency:
			case Type::Length:
			case Type::Pressure:
			case Type::Speed:
			case Type::Time:
				if (_bytes != 2 && _bytes != 4 && _bytes != 8)
					throw Xefis::Exception (QString ("invalid 'bytes' attribute %1, should be 2, 4 or 8").arg (_bytes).toStdString());
			case Type::Unknown:
				// Impossible.
				;
		}
	}

	if (!element.hasAttribute ("path"))
		throw Xefis::Exception ("<property> needs attribute 'path'");

	std::string path = element.attribute ("path").toStdString();
	_property_integer.set_path (path);
	_property_float.set_path (path);
	_property_angle.set_path (path);
	_property_frequency.set_path (path);
	_property_pressure.set_path (path);
	_property_speed.set_path (path);
	_property_time.set_path (path);
}


inline Link::Blob::size_type
Link::PropertyItem::size() const
{
	return _bytes;
}


void
Link::PropertyItem::produce (Blob& blob)
{
	enum class Kind { Integer, Float };

	Kind kind = Kind::Integer;
	Xefis::PropertyInteger::Type integer_value = 0;
	Xefis::PropertyFloat::Type float_value = 0.f;

#define XEFIS_CASE_FLOAT(type, property) \
	case Type::type: \
		kind = Kind::Float; \
		if (_property_##property.is_nil()) \
			float_value = std::numeric_limits<decltype (float_value)>::quiet_NaN(); \
		else \
			float_value = _property_##property.read().internal(); \
		break;

	switch (_type)
	{
		case Type::Integer:
			kind = Kind::Integer;
			integer_value = *_property_integer;
			break;

		case Type::Float:
			kind = Kind::Float;
			if (_property_float.is_nil())
				float_value = std::numeric_limits<decltype (float_value)>::quiet_NaN();
			else
				float_value = *_property_float;
			break;

		XEFIS_CASE_FLOAT (Angle, angle);
		XEFIS_CASE_FLOAT (Frequency, frequency);
		XEFIS_CASE_FLOAT (Length, length);
		XEFIS_CASE_FLOAT (Pressure, pressure);
		XEFIS_CASE_FLOAT (Speed, speed);
		XEFIS_CASE_FLOAT (Time, time);

		case Type::Unknown:
			// Impossible.
			break;
	}

#undef XEFIS_CASE_FLOAT

	switch (kind)
	{
		case Kind::Integer:
			if (_bytes == 1)
				serialize<int8_t> (blob, integer_value);
			else if (_bytes == 2)
				serialize<int16_t> (blob, integer_value);
			else if (_bytes == 4)
				serialize<int32_t> (blob, integer_value);
			else if (_bytes == 8)
				serialize<int64_t> (blob, integer_value);
			break;

		case Kind::Float:
			if (_bytes == 2)
				serialize<float16_t> (blob, float_value);
			else if (_bytes == 4)
				serialize<float32_t> (blob, float_value);
			else if (_bytes == 8)
				serialize<float64_t> (blob, float_value);
			break;
	}
}


Link::Blob::iterator
Link::PropertyItem::eat (Blob::iterator begin, Blob::iterator end)
{
	Blob::iterator result = begin;

	enum class Kind { Integer, Float };

	Kind kind = Kind::Integer;

	switch (_type)
	{
		case Type::Integer:
			kind = Kind::Integer;
			break;

		case Type::Float:
		case Type::Angle:
		case Type::Frequency:
		case Type::Length:
		case Type::Pressure:
		case Type::Speed:
		case Type::Time:
			kind = Kind::Float;
			break;

		case Type::Unknown:
			throw std::logic_error ("impossible (1)");
	}

	switch (kind)
	{
		case Kind::Integer:
			if (_bytes == 1)
				result = unserialize<int8_t> (begin, end, _integer_value);
			else if (_bytes == 2)
				result = unserialize<int16_t> (begin, end, _integer_value);
			else if (_bytes == 4)
				result = unserialize<int32_t> (begin, end, _integer_value);
			else if (_bytes == 8)
				result = unserialize<int64_t> (begin, end, _integer_value);
			break;

		case Kind::Float:
			if (_bytes == 2)
				result = unserialize<float16_t> (begin, end, _float_value);
			else if (_bytes == 4)
				result = unserialize<float32_t> (begin, end, _float_value);
			else if (_bytes == 8)
				result = unserialize<float64_t> (begin, end, _float_value);
			break;
	}

	return result;
}


void
Link::PropertyItem::apply()
{
#define XEFIS_CASE_FLOAT(type, property) \
	case Type::type: \
		if (std::isnan (_float_value)) \
			_property_##property.set_nil(); \
		else \
			_property_##property.write (si_from_internal<type> (_float_value)); \
		break;

	switch (_type)
	{
		case Type::Integer:
			_property_integer.write (_integer_value);
			break;

		case Type::Float:
			if (std::isnan (_float_value))
				_property_float.set_nil();
			else
				_property_float.write (_float_value);
			break;

		XEFIS_CASE_FLOAT (Angle, angle);
		XEFIS_CASE_FLOAT (Frequency, frequency);
		XEFIS_CASE_FLOAT (Length, length);
		XEFIS_CASE_FLOAT (Pressure, pressure);
		XEFIS_CASE_FLOAT (Speed, speed);
		XEFIS_CASE_FLOAT (Time, time);

		case Type::Unknown:
			// Impossible.
			;
	}

#undef XEFIS_CASE_FLOAT
}


void
Link::PropertyItem::failsafe()
{
#define XEFIS_CASE_FLOAT(type, property) \
	case Type::type: \
		_property_##property.set_nil(); \
		break;

	switch (_type)
	{
		case Type::Integer:
			_property_integer.set_nil();
			break;

		case Type::Float:
			_property_float.set_nil();
			break;

		XEFIS_CASE_FLOAT (Angle, angle);
		XEFIS_CASE_FLOAT (Frequency, frequency);
		XEFIS_CASE_FLOAT (Length, length);
		XEFIS_CASE_FLOAT (Pressure, pressure);
		XEFIS_CASE_FLOAT (Speed, speed);
		XEFIS_CASE_FLOAT (Time, time);

		case Type::Unknown:
			// Impossible.
			;
	}

#undef XEFIS_CASE_FLOAT
}


template<class CastType, class SourceType>
	inline void
	Link::PropertyItem::serialize (Blob& blob, SourceType src)
	{
		std::size_t size = sizeof (CastType);
		CastType casted (src);
		boost::endian::native_to_little (casted);
		uint8_t* ptr = reinterpret_cast<uint8_t*> (&casted);
		blob.resize (blob.size() + size);
		std::copy (ptr, ptr + size, &blob[blob.size() - size]);
	}


template<class CastType, class SourceType>
	inline Link::Blob::iterator
	Link::PropertyItem::unserialize (Blob::iterator begin, Blob::iterator end, SourceType& src)
	{
		if (static_cast<std::size_t> (std::distance (begin, end)) < sizeof (CastType))
			throw ParseError();
		std::size_t size = sizeof (CastType);
		CastType casted;
		std::copy (begin, begin + size, reinterpret_cast<uint8_t*> (&casted));
		boost::endian::little_to_native (casted);
		src = casted;
		return begin + size;
	}


template<class SIType>
	inline SIType
	Link::PropertyItem::si_from_internal (Xefis::PropertyFloat::Type float_value)
	{
		SIType t;
		t.internal() = float_value;
		return t;
	}


Link::BitfieldItem::BitfieldItem (Link*, QDomElement& element)
{
	for (QDomElement& e: element)
	{
		if (e == "property")
		{
			if (!e.hasAttribute ("type"))
				throw Xefis::Exception ("<property> needs attribute 'type'");
			if (!e.hasAttribute ("path"))
				throw Xefis::Exception ("<property> needs attribute 'path'");

			BitSource bs;

			QString s_type = e.attribute ("type");
			QString s_path = e.attribute ("path");

			if (s_type == "boolean")
			{
				bs.is_boolean = true;
				bs.bits = 1;
				bs.property_boolean.set_path (e.attribute ("path").toStdString());
			}
			else if (s_type == "integer")
			{
				if (!e.hasAttribute ("bits"))
					throw Xefis::Exception ("<property> of type 'integer' needs attribute 'bits'");

				bs.is_boolean = false;
				bs.property_integer.set_path (e.attribute ("path").toStdString());
				bs.bits = e.attribute ("bits").toUInt();
			}
			else
				throw Xefis::Exception ("attribute 'type' of <property> must be 'boolean' or 'integer'");

			_bit_sources.push_back (bs);
		}
	}

	Blob::size_type total_bits = 0;
	for (BitSource const& bs: _bit_sources)
		total_bits += bs.bits;

	_size = (total_bits + 7) / 8;
}


inline Link::Blob::size_type
Link::BitfieldItem::size() const
{
	return _size;
}


void
Link::BitfieldItem::produce (Blob& blob)
{
	std::vector<bool> bits;
	bits.reserve (8 * size());

	for (BitSource const& bs: _bit_sources)
	{
		if (bs.is_boolean)
			bits.push_back (*bs.property_boolean);
		else
		{
			Xefis::PropertyInteger::Type value = *bs.property_integer;
			for (uint8_t b = 0; b < bs.bits; ++b)
				bits.push_back ((value >> b) & 1);
		}
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


Link::Blob::iterator
Link::BitfieldItem::eat (Blob::iterator begin, Blob::iterator end)
{
	if (std::distance (begin, end) < static_cast<Blob::difference_type> (size()))
		throw ParseError();

	std::vector<bool> bits;
	bits.reserve (8 * size());

	for (Blob::iterator cur = begin; cur < begin + size(); ++cur)
		for (uint8_t b = 0; b < 8; ++b)
			bits.push_back ((*cur >> b) & 1);

	std::vector<bool>::iterator bit = bits.begin();
	for (BitSource& bs: _bit_sources)
	{
		if (bs.is_boolean)
			bs.boolean_value = *bit;
		else
		{
			Xefis::PropertyInteger::Type value = 0;
			for (uint8_t b = 0; b < bs.bits; ++b)
				if (*(bit + b))
					value |= 1 << b;
			bs.integer_value = value;
		}

		std::advance (bit, bs.bits);
	}

	return begin + size();
}


void
Link::BitfieldItem::apply()
{
	for (BitSource& bs: _bit_sources)
	{
		if (bs.is_boolean)
			bs.property_boolean.write (bs.boolean_value);
		else
			bs.property_integer.write (bs.integer_value);
	}
}


void
Link::BitfieldItem::failsafe()
{
	for (BitSource& bs: _bit_sources)
	{
		if (bs.is_boolean)
			bs.property_boolean.set_nil();
		else
			bs.property_integer.set_nil();
	}
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

	return begin + whole_size;
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

	Frequency output_frequency = 1_Hz;
	Time failsafe_after = 1_ms;
	Time reacquire_after = 1_ms;

	for (QDomElement& e: config)
	{
		if (e == "properties")
		{
			parse_properties (e, {
				{ "link-valid", _link_valid_prop, false },
				{ "failsafes", _failsafes, false },
				{ "reacquires", _reacquires, false },
				{ "error-bytes", _error_bytes, false },
				{ "valid-packets", _valid_packets, false },
			});
		}
		else if (e == "protocol")
			parse_protocol (e);
		else if (e == "input")
		{
			if (!e.hasAttribute ("failsafe-after"))
				throw Xefis::Exception ("<input> needs attribute 'failsafe-after'");
			failsafe_after.parse (e.attribute ("failsafe-after").toStdString());

			if (!e.hasAttribute ("reacquire-after"))
				throw Xefis::Exception ("<input> needs attribute 'reacquire-after'");
			reacquire_after.parse (e.attribute ("reacquire-after").toStdString());

			_input_interference = e.hasAttribute ("interference");

			parse_input_output_config (e, _udp_input_host, _udp_input_port);
			_udp_input = new QUdpSocket();
			_udp_input_enabled = true;
			_udp_input->bind (QHostAddress (_udp_input_host), _udp_input_port, QUdpSocket::ShareAddress);
			QObject::connect (_udp_input, SIGNAL (readyRead()), this, SLOT (got_udp_packet()));
		}
		else if (e == "output")
		{
			if (!e.hasAttribute ("frequency"))
				throw Xefis::Exception ("<output> needs attribute 'frequency'");
			output_frequency.parse (e.attribute ("frequency").toStdString());

			_output_interference = e.hasAttribute ("interference");

			parse_input_output_config (e, _udp_output_host, _udp_output_port);
			_udp_output = new QUdpSocket();
			_udp_output_enabled = true;
		}
	}

	_input_blob.reserve (2 * size());
	_output_blob.reserve (2 * size());

	_link_valid_prop.set_default (false);
	_failsafes.set_default (0);
	_reacquires.set_default (0);
	_error_bytes.set_default (0);
	_valid_packets.set_default (0);

	_failsafe_timer = new QTimer (this);
	_failsafe_timer->setSingleShot (true);
	_failsafe_timer->setInterval (failsafe_after.ms());
	QObject::connect (_failsafe_timer, SIGNAL (timeout()), this, SLOT (failsafe()));

	_reacquire_timer = new QTimer (this);
	_reacquire_timer->setSingleShot (true);
	_reacquire_timer->setInterval (reacquire_after.ms());
	QObject::connect (_reacquire_timer, SIGNAL (timeout()), this, SLOT (reacquire()));

	_output_timer = new QTimer (this);
	_output_timer->setSingleShot (false);
	_output_timer->setInterval (1000 / output_frequency.Hz());
	QObject::connect (_output_timer, SIGNAL (timeout()), this, SLOT (send_output()));
	_output_timer->start();
}


Link::~Link()
{
	delete _udp_input;
	delete _udp_output;
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

#if XEFIS_LINK_RECV_DEBUG
		std::clog << "io/link:recv: " << to_string (_input_blob) << std::endl;
#endif

	eat (_input_blob);
}


void
Link::send_output()
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
Link::failsafe()
{
	if (_link_valid_prop.valid())
		_link_valid_prop.write (false);
	_link_valid = false;
	if (_failsafes.valid())
		_failsafes.write (*_failsafes + 1);
	for (Packet* p: _packets)
		p->failsafe();
}


void
Link::reacquire()
{
	if (_link_valid_prop.valid())
		_link_valid_prop.write (true);
	_link_valid = true;
	if (_reacquires.valid())
		_reacquires.write (*_reacquires + 1);
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

	bool applied = false;
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
			packet->apply();
			applied = true;

			if (_valid_packets.valid())
				_valid_packets.write (*_valid_packets + 1);

			// Restart failsafe timer:
			_failsafe_timer->start();

			// If link is not valid, and we got valid packet,
			// start reacquire timer:
			if (!_link_valid && !_reacquire_timer->isActive())
				_reacquire_timer->start();
		}
		catch (...)
		{
			// Skip one byte and try again:
			if (blob.size() >= 1)
				blob.erase (blob.begin(), blob.begin() + 1);
			if (_error_bytes.valid())
				_error_bytes.write (*_error_bytes + 1);

			// Since there was an error, stop reacquire timer:
			_reacquire_timer->stop();
		}
	}

	if (applied)
		signal_data_updated();
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

	if (_packets.empty())
		throw Xefis::Exception ("protocol must not be empty");
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

