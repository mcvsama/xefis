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

// Qt:
#include <QtXml/QDomElement>

// Lib:
#include <boost/endian/conversion.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/qdom_iterator.h>
#include <xefis/utility/hash.h>
#include <xefis/utility/hextable.h>
#include <xefis/utility/blob.h>

// Local:
#include "link.h"


XEFIS_REGISTER_MODULE_CLASS ("io/link", Link)


Link::ItemSequence::ItemSequence (Link* link, QDomElement const& element)
{
	for (QDomElement const& e: xf::iterate_sub_elements (element))
	{
		if (e == "property")
			_items.push_back (new PropertyItem (link, e));
		else if (e == "bitfield")
			_items.push_back (new BitfieldItem (link, e));
		else if (e == "signature")
			_items.push_back (new SignatureItem (link, e));
	}
}


Link::ItemSequence::~ItemSequence()
{
	for (Item* item: _items)
		delete item;
}


inline Blob::size_type
Link::ItemSequence::size() const
{
	Blob::size_type s = 0;
	for (Item* item: _items)
		s += item->size();
	return s;
}


inline void
Link::ItemSequence::produce (Blob& blob)
{
	for (Item* item: _items)
		item->produce (blob);
}


inline Blob::iterator
Link::ItemSequence::eat (Blob::iterator begin, Blob::iterator end)
{
	for (Item* item: _items)
		begin = item->eat (begin, end);
	return begin;
}


void
Link::ItemSequence::apply()
{
	for (Item* item: _items)
		item->apply();
}


void
Link::ItemSequence::failsafe()
{
	for (Item* item: _items)
		item->failsafe();
}


Link::PropertyItem::PropertyItem (Link* link, QDomElement const& element)
{
	if (!element.hasAttribute ("type"))
		throw xf::MissingDomAttribute (element, "type");

	QString type_attr = element.attribute ("type");

	if (type_attr == "integer")
		_type = Type::Integer;
	else if (type_attr == "float")
		_type = Type::Float;
	else if (type_attr == "acceleration")
		_type = Type::Acceleration;
	else if (type_attr == "angle")
		_type = Type::Angle;
	else if (type_attr == "area")
		_type = Type::Area;
	else if (type_attr == "charge")
		_type = Type::Charge;
	else if (type_attr == "current")
		_type = Type::Current;
	else if (type_attr == "density")
		_type = Type::Density;
	else if (type_attr == "energy")
		_type = Type::Energy;
	else if (type_attr == "force")
		_type = Type::Force;
	else if (type_attr == "frequency")
		_type = Type::Frequency;
	else if (type_attr == "angular-velocity")
		_type = Type::AngularVelocity;
	else if (type_attr == "length")
		_type = Type::Length;
	else if (type_attr == "power")
		_type = Type::Power;
	else if (type_attr == "pressure")
		_type = Type::Pressure;
	else if (type_attr == "speed")
		_type = Type::Speed;
	else if (type_attr == "temperature")
		_type = Type::Temperature;
	else if (type_attr == "time")
		_type = Type::Time;
	else if (type_attr == "torque")
		_type = Type::Torque;
	else if (type_attr == "volume")
		_type = Type::Volume;
	else if (type_attr == "mass")
		_type = Type::Mass;

	if (_type == Type::Unknown)
		throw xf::BadDomAttribute (element, "type", "unknown type: " + type_attr);

	_retained = Link::check_retained_attribute (element, false);

	switch (_type)
	{
		case Type::Integer:
		case Type::Float:
		case Type::Acceleration:
		case Type::Angle:
		case Type::Area:
		case Type::Charge:
		case Type::Current:
		case Type::Density:
		case Type::Energy:
		case Type::Force:
		case Type::Frequency:
		case Type::AngularVelocity:
		case Type::Length:
		case Type::Power:
		case Type::Pressure:
		case Type::Speed:
		case Type::Temperature:
		case Type::Time:
		case Type::Torque:
		case Type::Volume:
		case Type::Mass:
			if (!element.hasAttribute ("bytes"))
				throw xf::MissingDomAttribute (element, "bytes");
			break;
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
					throw xf::BadDomAttribute (element, "bytes", QString ("is %1, should be 1, 2, 4 or 8").arg (_bytes));
				break;
			case Type::Float:
			case Type::Acceleration:
			case Type::Angle:
			case Type::Area:
			case Type::Charge:
			case Type::Current:
			case Type::Density:
			case Type::Energy:
			case Type::Force:
			case Type::Frequency:
			case Type::AngularVelocity:
			case Type::Length:
			case Type::Power:
			case Type::Pressure:
			case Type::Speed:
			case Type::Temperature:
			case Type::Time:
			case Type::Torque:
			case Type::Volume:
			case Type::Mass:
				if (_bytes != 2 && _bytes != 4 && _bytes != 8)
					throw xf::BadDomAttribute (element, "bytes", QString ("is %1, should be 2, 4 or 8").arg (_bytes));
				break;
			case Type::Unknown:
				// Impossible.
				;
		}
	}

	if (!element.hasAttribute (link->_path_attribute_name))
		throw xf::MissingDomAttribute (element, link->_path_attribute_name);

	xf::PropertyPath path (element.attribute (link->_path_attribute_name));
	_property_integer.set_path (path);
	_property_float.set_path (path);
	_property_acceleration.set_path (path);
	_property_angle.set_path (path);
	_property_area.set_path (path);
	_property_charge.set_path (path);
	_property_current.set_path (path);
	_property_density.set_path (path);
	_property_energy.set_path (path);
	_property_force.set_path (path);
	_property_power.set_path (path);
	_property_pressure.set_path (path);
	_property_frequency.set_path (path);
	_property_angular_velocity.set_path (path);
	_property_length.set_path (path);
	_property_speed.set_path (path);
	_property_temperature.set_path (path);
	_property_time.set_path (path);
	_property_torque.set_path (path);
	_property_volume.set_path (path);
	_property_mass.set_path (path);
}


inline Blob::size_type
Link::PropertyItem::size() const
{
	return _bytes;
}


void
Link::PropertyItem::produce (Blob& blob)
{
	enum class Kind { Integer, Float };

	Kind kind = Kind::Integer;
	xf::PropertyInteger::Type integer_value = 0;
	xf::PropertyFloat::Type float_value = 0.f;

#define XEFIS_CASE_FLOAT(type, property) \
	case Type::type: \
		kind = Kind::Float; \
		if (_property_##property.is_nil()) \
			float_value = std::numeric_limits<decltype (float_value)>::quiet_NaN(); \
		else \
			float_value = _property_##property.read().base_quantity(); \
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

		XEFIS_CASE_FLOAT (Acceleration, acceleration);
		XEFIS_CASE_FLOAT (Angle, angle);
		XEFIS_CASE_FLOAT (Area, area);
		XEFIS_CASE_FLOAT (Charge, charge);
		XEFIS_CASE_FLOAT (Current, current);
		XEFIS_CASE_FLOAT (Density, density);
		XEFIS_CASE_FLOAT (Energy, energy);
		XEFIS_CASE_FLOAT (Force, force);
		XEFIS_CASE_FLOAT (Frequency, frequency);
		XEFIS_CASE_FLOAT (AngularVelocity, angular_velocity);
		XEFIS_CASE_FLOAT (Length, length);
		XEFIS_CASE_FLOAT (Power, power);
		XEFIS_CASE_FLOAT (Pressure, pressure);
		XEFIS_CASE_FLOAT (Speed, speed);
		XEFIS_CASE_FLOAT (Temperature, temperature);
		XEFIS_CASE_FLOAT (Time, time);
		XEFIS_CASE_FLOAT (Torque, torque);
		XEFIS_CASE_FLOAT (Volume, volume);
		XEFIS_CASE_FLOAT (Mass, mass);

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


Blob::iterator
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
		case Type::Acceleration:
		case Type::Angle:
		case Type::Area:
		case Type::Charge:
		case Type::Current:
		case Type::Density:
		case Type::Energy:
		case Type::Force:
		case Type::Frequency:
		case Type::AngularVelocity:
		case Type::Length:
		case Type::Power:
		case Type::Pressure:
		case Type::Speed:
		case Type::Temperature:
		case Type::Time:
		case Type::Torque:
		case Type::Volume:
		case Type::Mass:
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
		{ \
			if (!_retained) \
				_property_##property.set_nil(); \
		} \
		else \
			_property_##property.write (type (_float_value)); \
		break;

	switch (_type)
	{
		case Type::Integer:
			_property_integer.write (_integer_value);
			break;

		case Type::Float:
			if (std::isnan (_float_value))
			{
				if (!_retained)
					_property_float.set_nil();
			}
			else
				_property_float.write (_float_value);
			break;

		XEFIS_CASE_FLOAT (Acceleration, acceleration);
		XEFIS_CASE_FLOAT (Angle, angle);
		XEFIS_CASE_FLOAT (Area, area);
		XEFIS_CASE_FLOAT (Charge, charge);
		XEFIS_CASE_FLOAT (Current, current);
		XEFIS_CASE_FLOAT (Density, density);
		XEFIS_CASE_FLOAT (Energy, energy);
		XEFIS_CASE_FLOAT (Force, force);
		XEFIS_CASE_FLOAT (Frequency, frequency);
		XEFIS_CASE_FLOAT (AngularVelocity, angular_velocity);
		XEFIS_CASE_FLOAT (Length, length);
		XEFIS_CASE_FLOAT (Power, power);
		XEFIS_CASE_FLOAT (Pressure, pressure);
		XEFIS_CASE_FLOAT (Speed, speed);
		XEFIS_CASE_FLOAT (Temperature, temperature);
		XEFIS_CASE_FLOAT (Time, time);
		XEFIS_CASE_FLOAT (Torque, torque);
		XEFIS_CASE_FLOAT (Volume, volume);
		XEFIS_CASE_FLOAT (Mass, mass);

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

	if (!_retained)
	{
		switch (_type)
		{
			case Type::Integer:
				_property_integer.set_nil();
				break;

			case Type::Float:
				_property_float.set_nil();
				break;

			XEFIS_CASE_FLOAT (Acceleration, acceleration);
			XEFIS_CASE_FLOAT (Angle, angle);
			XEFIS_CASE_FLOAT (Area, area);
			XEFIS_CASE_FLOAT (Charge, charge);
			XEFIS_CASE_FLOAT (Current, current);
			XEFIS_CASE_FLOAT (Density, density);
			XEFIS_CASE_FLOAT (Energy, energy);
			XEFIS_CASE_FLOAT (Force, force);
			XEFIS_CASE_FLOAT (Frequency, frequency);
			XEFIS_CASE_FLOAT (AngularVelocity, angular_velocity);
			XEFIS_CASE_FLOAT (Length, length);
			XEFIS_CASE_FLOAT (Power, power);
			XEFIS_CASE_FLOAT (Pressure, pressure);
			XEFIS_CASE_FLOAT (Speed, speed);
			XEFIS_CASE_FLOAT (Temperature, temperature);
			XEFIS_CASE_FLOAT (Time, time);
			XEFIS_CASE_FLOAT (Torque, torque);
			XEFIS_CASE_FLOAT (Volume, volume);
			XEFIS_CASE_FLOAT (Mass, mass);

			case Type::Unknown:
				// Impossible.
				;
		}
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
	inline Blob::iterator
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


Link::BitfieldItem::BitfieldItem (Link* link, QDomElement const& element)
{
	for (QDomElement const& e: xf::iterate_sub_elements (element))
	{
		if (e == "property")
		{
			if (!e.hasAttribute ("type"))
				throw xf::MissingDomAttribute (e, "type");
			if (!e.hasAttribute (link->_path_attribute_name))
				throw xf::MissingDomAttribute (e, link->_path_attribute_name);

			BitSource bs;

			QString s_type = e.attribute ("type");
			QString s_path = e.attribute (link->_path_attribute_name);

			bs.retained = check_retained_attribute (e, false);

			if (s_type == "boolean")
			{
				bs.is_boolean = true;
				bs.bits = 1;
				bs.property_boolean.set_path (xf::PropertyPath (s_path));
			}
			else if (s_type == "integer")
			{
				if (!e.hasAttribute ("bits"))
					throw xf::MissingDomAttribute (e, "bits");

				bs.is_boolean = false;
				bs.property_integer.set_path (xf::PropertyPath (s_path));
				bs.bits = e.attribute ("bits").toUInt();
			}
			else
				throw xf::BadDomAttribute (e, "type", "must be 'boolean' or 'integer'");

			_bit_sources.push_back (bs);
		}
	}

	Blob::size_type total_bits = 0;
	for (BitSource const& bs: _bit_sources)
		total_bits += bs.bits;

	_size = (total_bits + 7) / 8;
}


inline Blob::size_type
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
			xf::PropertyInteger::Type value = *bs.property_integer;
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


Blob::iterator
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
			xf::PropertyInteger::Type value = 0;
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
		if (!bs.retained)
		{
			if (bs.is_boolean)
				bs.property_boolean.set_nil();
			else
				bs.property_integer.set_nil();
		}
	}
}


Link::SignatureItem::SignatureItem (Link* link, QDomElement const& element):
	ItemSequence (link, element),
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
		_key = xf::parse_hex_string (element.attribute ("key"));
	else
		_key = { 0 };

	_temp.reserve (size());
}


inline Blob::size_type
Link::SignatureItem::size() const
{
	return ItemSequence::size() + _random_bytes + _signature_bytes;
}


void
Link::SignatureItem::produce (Blob& blob)
{
	_temp.clear();
	ItemSequence::produce (_temp);

	// Append random bytes:
	_temp.reserve (_temp.size() + _random_bytes);
	std::uniform_int_distribution<uint8_t> dist;
	for (unsigned int i = 0; i < _random_bytes; ++i)
		_temp.push_back (dist (_rng));
	// Append the key, compute signature:
	_temp.insert (_temp.end(), _key.begin(), _key.end());
	xf::Hash hash (_temp);
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


Blob::iterator
Link::SignatureItem::eat (Blob::iterator begin, Blob::iterator end)
{
	Blob::size_type data_size = ItemSequence::size();
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
	xf::Hash hash (_temp);
	// If hashes mismatch, that's parsing error:
	if (!std::equal (sign_begin, sign_end, hash.begin()))
		throw ParseError();

	if (ItemSequence::eat (begin, begin + data_size) != begin + data_size)
		throw ParseError();

	return begin + whole_size;
}


Link::Packet::Packet (Link* link, QDomElement const& element):
	ItemSequence (link, element)
{
	if (!element.hasAttribute ("magic"))
		throw xf::MissingDomAttribute (element, "magic");

	_magic = xf::parse_hex_string (element.attribute ("magic"));
	if (_magic.empty())
		throw xf::BadDomAttribute (element, "magic", "value must be at least one byte long");

	if (element.hasAttribute ("send-every"))
		_send_every = element.attribute ("send-every").toUInt();

	if (element.hasAttribute ("send-offset"))
		_send_offset = element.attribute ("send-offset").toUInt();
}


inline Blob const&
Link::Packet::magic() const
{
	return _magic;
}


inline Blob::size_type
Link::Packet::size() const
{
	return ItemSequence::size();
}


void
Link::Packet::produce (Blob& blob)
{
	if (_send_pos % _send_every == _send_offset)
	{
		blob.insert (blob.end(), _magic.begin(), _magic.end());
		ItemSequence::produce (blob);
	}
	++_send_pos;
}


Link::Link (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	Frequency output_frequency = 1_Hz;
	Time failsafe_after = 1_ms;
	Time reacquire_after = 1_ms;

	parse_properties (config, {
		{ "input", _input, false },
		{ "output", _output, false },
		{ "link-valid", _link_valid_prop, false },
		{ "failsafes", _failsafes, false },
		{ "reacquires", _reacquires, false },
		{ "error-bytes", _error_bytes, false },
		{ "valid-bytes", _valid_bytes, false },
		{ "valid-packets", _valid_packets, false },
	});

	parse_settings (config, {
		{ "failsafe-after", failsafe_after, false },
		{ "reacquire-after", reacquire_after, false },
		{ "frequency", output_frequency, false },
	});

	for (QDomElement const& e: xf::iterate_sub_elements (config))
	{
		if (e == "protocol")
			parse_protocol (e);
	}

	_input_blob.reserve (2 * size());
	_output_blob.reserve (2 * size());

	_link_valid_prop.set_default (false);
	_failsafes.set_default (0);
	_reacquires.set_default (0);
	_error_bytes.set_default (0);
	_valid_bytes.set_default (0);
	_valid_packets.set_default (0);

	_failsafe_timer = new QTimer (this);
	_failsafe_timer->setSingleShot (true);
	_failsafe_timer->setInterval (failsafe_after.quantity<Millisecond>());
	QObject::connect (_failsafe_timer, SIGNAL (timeout()), this, SLOT (failsafe()));

	_reacquire_timer = new QTimer (this);
	_reacquire_timer->setSingleShot (true);
	_reacquire_timer->setInterval (reacquire_after.quantity<Millisecond>());
	QObject::connect (_reacquire_timer, SIGNAL (timeout()), this, SLOT (reacquire()));

	_output_timer = new QTimer (this);
	_output_timer->setSingleShot (false);
	_output_timer->setInterval (1000 / output_frequency.quantity<Hertz>());
	QObject::connect (_output_timer, SIGNAL (timeout()), this, SLOT (send_output()));
	_output_timer->start();
}


void
Link::data_updated()
{
	try {
		if (_input.valid() && _input.fresh())
		{
			std::string data = *_input;
			_input_blob.insert (_input_blob.end(), data.begin(), data.end());
			eat (_input_blob);
		}
	}
	catch (ParseError const&)
	{
		log() << "Packet parse error. Couldn't synchronize." << std::endl;
	}
}


void
Link::send_output()
{
	if (_output.configured())
	{
		_output_blob.clear();
		produce (_output_blob);

		std::string s;
		s.insert (s.end(), _output_blob.begin(), _output_blob.end());
		_output.write (s);
	}
}


void
Link::failsafe()
{
	if (_link_valid_prop.configured())
		_link_valid_prop.write (false);
	_link_valid = false;
	if (_failsafes.configured())
		_failsafes.write (*_failsafes + 1);
	for (auto p: _packets)
		p->failsafe();
}


void
Link::reacquire()
{
	if (_link_valid_prop.configured())
		_link_valid_prop.write (true);
	_link_valid = true;
	if (_reacquires.configured())
		_reacquires.write (*_reacquires + 1);
}


inline Blob::size_type
Link::size() const
{
	Blob::size_type s = 0;
	for (auto p: _packets)
		s += p->size();
	return s;
}


void
Link::produce (Blob& blob)
{
	for (auto p: _packets)
		p->produce (blob);

#if XEFIS_LINK_SEND_DEBUG
	log() << "Send: " << to_string (_output_blob) << std::endl;
#endif
}


void
Link::eat (Blob& blob)
{
#if XEFIS_LINK_RECV_DEBUG
	log() << "Recv: " << to_string (blob) << std::endl;
#endif

	Blob _tmp_input_magic;
	_tmp_input_magic.resize (_magic_size);

	while (blob.size() > _magic_size + 1)
	{
		auto skip_byte_and_retry = [&]() -> void
		{
			// Skip one byte and try again:
			if (blob.size() >= 1)
				blob.erase (blob.begin(), blob.begin() + 1);
			if (_error_bytes.configured())
				_error_bytes.write (*_error_bytes + 1);

			// Since there was an error, stop reacquire timer:
			_reacquire_timer->stop();
		};

		bool return_from_function = false;

		xf::Exception::guard ([&] {
			try {
				// Find the right magic and packet:
				std::copy (blob.begin(), blob.begin() + _magic_size, _tmp_input_magic.begin());
				PacketMagics::iterator packet_and_magic = _packet_magics.find (_tmp_input_magic);

				// If not found, retry starting with next byte:
				if (packet_and_magic == _packet_magics.end())
					throw ParseError();

				auto packet = packet_and_magic->second;
				// Now see if we have enough data in input buffer for this packet type.
				// If not, return and retry when enough data is read.
				if (blob.size() - _magic_size < packet->size())
				{
					return_from_function = true;
					return;
				}

				Blob::iterator e = packet->eat (blob.begin() + _magic_size, blob.end());
				Blob::size_type valid_bytes = std::distance (blob.begin(), e);
				blob.erase (blob.begin(), e);
				packet->apply();

				if (_valid_packets.configured())
					_valid_packets.write (*_valid_packets + 1);

				if (_valid_bytes.configured())
					_valid_bytes.write (*_valid_bytes + valid_bytes);

				// Restart failsafe timer:
				_failsafe_timer->start();

				// If link is not valid, and we got valid packet,
				// start reacquire timer:
				if (!_link_valid && !_reacquire_timer->isActive())
					_reacquire_timer->start();
			}
			catch (ParseError&)
			{
				skip_byte_and_retry();
			}
			catch (...)
			{
				skip_byte_and_retry();
				throw;
			}
		});

		if (return_from_function)
			return;
	}
}


void
Link::parse_protocol (QDomElement const& protocol)
{
	_path_attribute_name = protocol.attribute ("path-attribute-name");
	if (!_path_attribute_name.isEmpty())
	{
		if (!_path_attribute_name.startsWith ("path-"))
			throw xf::BadConfiguration ("if used, the 'path-attribute-name' attribute must start with 'path-' prefix");
	}
	else
		_path_attribute_name = "path";

	for (QDomElement const& e: xf::iterate_sub_elements (protocol))
	{
		if (e == "packet")
			_packets.push_back (std::make_shared<Packet> (this, e));
	}

	_packet_magics.clear();
	// Ensure all packets have distinct magic values and the same size:
	for (auto p: _packets)
	{
		if (_magic_size == 0)
			_magic_size = p->magic().size();
		if (_magic_size != p->magic().size())
			throw xf::BadConfiguration ("all magic values have to have equal number of bytes");
		if (!_packet_magics.insert ({ p->magic(), p }).second)
			throw xf::BadConfiguration ("same magic value " + to_string (p->magic()) + " used for two or more packets");
	}

	if (_packets.empty())
		throw xf::BadConfiguration ("protocol must not be empty");
}


std::string
Link::to_string (Blob const& blob)
{
	if (blob.empty())
		return "";
	std::string s;
	for (auto v: blob)
		s += QString ("%1").arg (v, 2, 16, QChar ('0')).toStdString() + ":";
	s.pop_back();
	return s;
}


bool
Link::check_retained_attribute (QDomElement const& element, bool default_value)
{
	QString s_retained = element.attribute ("retained");

	if (element.hasAttribute ("retained"))
	{
		if (s_retained == "true")
			default_value = true;
		else if (s_retained != "false")
			throw xf::BadDomAttribute (element, "retained", "must be 'true' or 'false'");
	}

	return default_value;
}

