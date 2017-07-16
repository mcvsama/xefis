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

#ifndef XEFIS__MODULES__IO__LINK_H__INCLUDED
#define XEFIS__MODULES__IO__LINK_H__INCLUDED

// Standard:
#include <cstddef>
#include <memory>
#include <random>
#include <vector>

// Qt:
#include <QtCore/QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/module.h>
#include <xefis/core/v1/property.h>


#define XEFIS_LINK_SEND_DEBUG 0
#define XEFIS_LINK_RECV_DEBUG 0


class Link:
	public QObject,
	public v1::Module
{
	Q_OBJECT

	class ParseError
	{ };

	/**
	 * An item that can produce or parse data to be sent/received wirelessly. May be a package of
	 * subitems.
	 */
	class Item
	{
	  public:
		// Dtor
		virtual
		~Item() = default;

		/**
		 * Return size of the data which will be produced/consumed.
		 */
		virtual Blob::size_type
		size() const = 0;

		/**
		 * Serialize data and add it to the blob.
		 */
		virtual void
		produce (Blob&) = 0;

		/**
		 * Parse data and set temporary variables.
		 * Data will be output when apply() is called.
		 */
		virtual Blob::iterator
		eat (Blob::iterator, Blob::iterator) = 0;

		/**
		 * Apply parsed data to properties, etc.
		 */
		virtual void
		apply() = 0;

		/**
		 * Set all managed properties to nil.
		 */
		virtual void
		failsafe() = 0;
	};

	/**
	 * A sequence of items, that is also an item. Simple.
	 */
	class ItemSequence: public Item
	{
	  public:
		typedef std::vector<Item*> Items;

	  public:
		// Ctor
		ItemSequence (Link*, QDomElement const&);

		// Dtor
		virtual
		~ItemSequence();

		Blob::size_type
		size() const override;

		void
		produce (Blob&) override;

		Blob::iterator
		eat (Blob::iterator, Blob::iterator) override;

		void
		apply() override;

		void
		failsafe() override;

	  private:
		Items	_items;
	};

	/**
	 * Item that refers to a particular Property, so it can send/receive value of that property.
	 */
	class PropertyItem: public Item
	{
		enum class Type
		{
			Unknown,
			Integer,
			Float,
			Acceleration,
			Angle,
			Area,
			Charge,
			Current,
			Density,
			Energy,
			Force,
			Power,
			Pressure,
			Frequency,
			AngularVelocity,
			Length,
			Speed,
			Temperature,
			Time,
			Torque,
			Volume,
			Mass,
		};

	  public:
		PropertyItem (Link*, QDomElement const&);

		Blob::size_type
		size() const override;

		void
		produce (Blob&) override;

		Blob::iterator
		eat (Blob::iterator, Blob::iterator) override;

		void
		apply() override;

		void
		failsafe() override;

	  private:
		/**
		 * Serialize SourceType and add to Blob.
		 */
		template<class CastType, class SourceType>
			static void
			serialize (Blob&, SourceType);

		/**
		 * Unserialize data from Blob and put it to src.
		 */
		template<class CastType, class SourceType>
			static Blob::iterator
			unserialize (Blob::iterator begin, Blob::iterator end, SourceType&);

	  private:
		Type						_type				= Type::Unknown;
		uint8_t						_bytes				= 0;
		bool						_retained			= false;
		v1::PropertyInteger			_property_integer;
		v1::PropertyFloat			_property_float;
		v1::PropertyAcceleration	_property_acceleration;
		v1::PropertyAngle			_property_angle;
		v1::PropertyArea			_property_area;
		v1::PropertyCharge			_property_charge;
		v1::PropertyCurrent			_property_current;
		v1::PropertyDensity			_property_density;
		v1::PropertyEnergy			_property_energy;
		v1::PropertyForce			_property_force;
		v1::PropertyPower			_property_power;
		v1::PropertyPressure		_property_pressure;
		v1::PropertyFrequency		_property_frequency;
		v1::Property<AngularVelocity> _property_angular_velocity;
		v1::PropertyLength			_property_length;
		v1::PropertySpeed			_property_speed;
		v1::PropertyTemperature		_property_temperature;
		v1::PropertyTime			_property_time;
		v1::PropertyTorque			_property_torque;
		v1::PropertyVolume			_property_volume;
		v1::PropertyMass			_property_mass;
		v1::PropertyInteger::Type	_integer_value;
		v1::PropertyFloat::Type		_float_value;
	};

	/**
	 * An item that contains boolean or limited-width integers.
	 * Refers to multiple boolean/integer Properties.
	 */
	class BitfieldItem: public Item
	{
		// TODO have a fallback value for integers that don't fit in bitwidth.
	  public:
		struct BitSource
		{
			bool						is_boolean			= false;
			bool						retained			= false;
			uint8_t						bits				= 0;
			v1::PropertyBoolean			property_boolean;
			v1::PropertyInteger			property_integer;
			v1::PropertyBoolean::Type	boolean_value;
			v1::PropertyInteger::Type	integer_value;
		};

		typedef std::vector<BitSource> BitSources;

	  public:
		BitfieldItem (Link*, QDomElement const&);

		Blob::size_type
		size() const override;

		void
		produce (Blob&) override;

		Blob::iterator
		eat (Blob::iterator, Blob::iterator) override;

		void
		apply() override;

		void
		failsafe() override;

	  private:
		BitSources		_bit_sources;
		Blob::size_type	_size = 0;
	};

	/**
	 * And item that adds or verifies simple digital signature of the contained items.
	 */
	class SignatureItem: public ItemSequence
	{
	  public:
		SignatureItem (Link*, QDomElement const&);

		Blob::size_type
		size() const override;

		void
		produce (Blob&) override;

		Blob::iterator
		eat (Blob::iterator, Blob::iterator) override;

	  private:
		unsigned int		_random_bytes		= 0;
		unsigned int		_signature_bytes	= 0;
		Blob				_key;
		std::random_device	_rd;
		std::mt19937		_rng;
		Blob				_temp;
	};

	/**
	 * A single packet containing a set of items. Configurable how often should be sent.
	 */
	class Packet: public ItemSequence
	{
	  public:
		// Ctor
		Packet (Link*, QDomElement const&);

		Blob const&
		magic() const;

		Blob::size_type
		size() const override;

		void
		produce (Blob& blob) override;

	  private:
		Blob		_magic;
		uint64_t	_send_every		= 1;
		uint64_t	_send_offset	= 0;
		uint64_t	_send_pos		= 0;
	};

	typedef std::vector<Shared<Packet>>		Packets;
	typedef std::map<Blob, Shared<Packet>>	PacketMagics;

  public:
	// Ctor
	Link (v1::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private slots:
	/**
	 * Called by output timer.
	 */
	void
	send_output();

	/**
	 * Called by failsafe timer.
	 */
	void
	failsafe();

	/**
	 * Called by reacquire timer.
	 */
	void
	reacquire();

  private:
	Blob::size_type
	size() const;

	void
	produce (Blob& blob);

	void
	eat (Blob& blob);

	void
	parse_protocol (QDomElement const& protocol);

	static std::string
	to_string (Blob const&);

	static bool
	check_retained_attribute (QDomElement const& element, bool default_value);

  private:
	QTimer*				_failsafe_timer;
	QTimer*				_reacquire_timer;
	QTimer*				_output_timer;
	bool				_link_valid				= false;
	v1::PropertyString	_input;
	v1::PropertyString	_output;
	v1::PropertyBoolean	_link_valid_prop;
	v1::PropertyInteger	_failsafes;
	v1::PropertyInteger	_reacquires;
	v1::PropertyInteger	_error_bytes;
	v1::PropertyInteger	_valid_bytes;
	v1::PropertyInteger	_valid_packets;
	Packets				_packets;
	PacketMagics		_packet_magics;
	Blob::size_type		_magic_size				= 0;
	Blob				_output_blob;
	Blob				_input_blob;
	Blob				_tmp_input_magic;
	// When set, all <property> and similar elements should use property path provided
	// in attribute denoted by _path_attribute_name, not the default "path" attribute. Restriction:
	// _path_attribute_name should start with "path-" prefix.
	QString				_path_attribute_name;
};

#endif
