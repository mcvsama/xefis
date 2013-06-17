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

#ifndef XEFIS__MODULES__IO__LINK_H__INCLUDED
#define XEFIS__MODULES__IO__LINK_H__INCLUDED

// Standard:
#include <cstddef>
#include <random>
#include <vector>

// Qt:
#include <QtNetwork/QUdpSocket>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>


class Link:
	public QObject,
	public Xefis::Module
{
	Q_OBJECT

	typedef std::vector<uint8_t> Blob;

	class ParseError { };

	/**
	 * This is a helper for decoding hex-encoded values.
	 * It assumes UTF-8/ASCII encoding.
	 */
	class HexTable
	{
	  public:
		HexTable();

		int operator[] (char c) const;

	  private:
		int _table[256];
	};

	class Item
	{
	  public:
		// Dtor
		virtual ~Item() { }

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
	};

	class ItemStream: public Item
	{
	  public:
		typedef std::vector<Item*> Items;

	  public:
		// Ctor
		ItemStream (Link*, QDomElement&);

		// Dtor
		virtual ~ItemStream() { }

		Blob::size_type
		size() const override;

		void
		produce (Blob&) override;

		Blob::iterator
		eat (Blob::iterator, Blob::iterator) override;

		void
		apply() override;

	  private:
		Items	_items;
	};

	class PropertyItem: public Item
	{
		enum class Type
		{
			Unknown,
			Float16,
			Float32,
			Float64,
			Int8,
			Int16,
			Int32,
			Int64,
		};

	  public:
		PropertyItem (Link*, QDomElement&);

		Blob::size_type
		size() const override;

		void
		produce (Blob&) override;

		Blob::iterator
		eat (Blob::iterator, Blob::iterator) override;

		void
		apply() override;

	  private:
		Type						_type;
		Xefis::PropertyFloat		_property;
		Xefis::PropertyFloat::Type	_value;
	};

	class BitfieldItem: public Item
	{
	  public:
		BitfieldItem (Link*, QDomElement&);

		Blob::size_type
		size() const override;

		void
		produce (Blob&) override;

		Blob::iterator
		eat (Blob::iterator, Blob::iterator) override;

		void
		apply() override;
	};

	class SignatureItem: public ItemStream
	{
	  public:
		SignatureItem (Link*, QDomElement&);

		Blob::size_type
		size() const override;

		void
		produce (Blob&) override;

		Blob::iterator
		eat (Blob::iterator, Blob::iterator) override;

	  private:
		unsigned int		_random_bytes		= 0;
		unsigned int		_signature_bytes	= 0;
		Link::Blob			_key;
		std::random_device	_rd;
		std::mt19937		_rng;
		Blob				_temp;
	};

	class Packet: public ItemStream
	{
	  public:
		// Ctor
		Packet (Link*, QDomElement&);

		Blob const&
		magic() const;

		Blob::size_type
		size() const override;

		void
		produce (Blob& blob) override;

	  private:
		Blob	_magic;
	};

	typedef std::vector<Packet*> Packets;
	typedef std::map<Blob, Packet*> PacketMagics;

  public:
	// Ctor
	Link (Xefis::ModuleManager*, QDomElement const& config);

	// Dtor
	~Link();

	void
	data_updated() override;

  private slots:
	/**
	 * Called whenever there's data ready to be read from socket.
	 */
	void
	got_udp_packet();

  private:
	Blob::size_type
	size() const;

	void
	produce (Blob& blob);

	void
	eat (Blob& blob);

	void
	apply();

	void
	parse_protocol (QDomElement const& protocol);

	static Blob
	parse_binary_string (QString const& string);

	static std::string
	to_string (Blob const&);

  private:
	Xefis::PropertyBoolean	_link_valid;
	Xefis::PropertyInteger	_errors;
	Xefis::PropertyInteger	_valid_packets;
	Xefis::PropertyInteger	_failsafe_count;
	Xefis::PropertyTime		_failsafe_lost_time;
	Xefis::PropertyTime		_failsafe_acquired_time;
	bool					_udp_output_enabled		= false;
	QUdpSocket*				_udp_output				= nullptr;
	QString					_udp_output_host;
	int						_udp_output_port		= 0;
	bool					_udp_input_enabled		= false;
	QUdpSocket*				_udp_input				= nullptr;
	QString					_udp_input_host;
	int						_udp_input_port			= 0;
	Packets					_packets;
	PacketMagics			_packet_magics;
	Blob::size_type			_magic_size				= 0;
	QByteArray				_input_datagram;
	Blob					_output_blob;
	Blob					_input_blob;
	Blob					_tmp_input_magic;
	bool					_last_parse_was_valid	= false;
};


inline
Link::HexTable::HexTable()
{
	std::fill (std::begin (_table), std::end (_table), 0);

	for (char c = '0'; c <= '9'; ++c)
		_table[static_cast<uint8_t> (c)] = c - '0';
	for (char c = 'a'; c <= 'f'; ++c)
		_table[static_cast<uint8_t> (c)] = 10 + c - 'a';
	for (char c = 'A'; c <= 'F'; ++c)
		_table[static_cast<uint8_t> (c)] = 10 + c - 'A';
}


inline int
Link::HexTable::operator[] (char c) const
{
	return _table[static_cast<uint8_t> (c)];
}

#endif
