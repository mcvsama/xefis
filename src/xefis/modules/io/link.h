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
#include <variant>
#include <optional>
#include <functional>
#include <type_traits>
#include <initializer_list>

// Qt:
#include <QtCore/QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/module_io.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/v2/actions.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/strong_type.h>
#include <xefis/utility/types.h>


#define XEFIS_LINK_SEND_DEBUG 0
#define XEFIS_LINK_RECV_DEBUG 0


class LinkIO;


class LinkProtocol
{
  public:
	using Bits				= xf::StrongType<uint8_t, struct BitsType>;
	using Magic				= xf::StrongType<Blob, struct MagicType>;
	using Key				= xf::StrongType<Blob, struct KeyType>;
	using SendEvery			= xf::StrongType<std::size_t, struct SendOffsetType>;
	using SendOffset		= xf::StrongType<std::size_t, struct SendOffsetType>;
	using Retained			= xf::StrongType<bool, struct RetainedType>;
	using NonceBytes		= xf::StrongType<uint8_t, struct NonceBytesType>;
	using SignatureBytes	= xf::StrongType<uint8_t, struct SignatureBytesType>;

	/**
	 * Thrown on known parse errors.
	 */
	class ParseError
	{ };

	/**
	 * Thrown by sub-packets when there's no enough input data
	 * Note that each Envelope's eat() is called when it's known for sure that there's enough data in the input buffer
	 * to cover whole Envelope.
	 */
	class InsufficientDataError
	{ };

	/**
	 * Thrown when one of Envelopes has different magic string size than the others.
	 */
	class InvalidMagicSize: public xf::Exception
	{
	  public:
		explicit
		InvalidMagicSize():
			Exception ("invalid magic string length; envelopes' magic strings must be the same length")
		{ }
	};

	/**
	 * An packet of data.
	 */
	class Packet
	{
	  public:
		// Dtor
		virtual
		~Packet() = default;

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
		virtual Blob::const_iterator
		eat (Blob::const_iterator, Blob::const_iterator) = 0;

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

	using PacketList = std::initializer_list<std::shared_ptr<Packet>>;

	/**
	 * A sequence of packets, that is also an packet. Simple.
	 */
	class Sequence: public Packet
	{
	  public:
		// Ctor
		explicit
		Sequence (PacketList);

		Blob::size_type
		size() const override;

		void
		produce (Blob&) override;

		Blob::const_iterator
		eat (Blob::const_iterator, Blob::const_iterator) override;

		void
		apply() override;

		void
		failsafe() override;

	  private:
		std::vector<std::shared_ptr<Packet>> _packets;
	};

	/**
	 * Packet that refers to a particular Property, so it can send/receive value of that property.
	 */
	template<uint8_t pBytes, class pValue>
		class Property: public Packet
		{
		  public:
			using Value = pValue;

			static constexpr uint8_t kBytes { pBytes };

			static_assert ((std::is_integral<Value>() &&
							(kBytes == 1 || kBytes == 2 || kBytes == 4 || kBytes == 8)) ||
						   ((std::is_floating_point<Value>() || si::is_quantity<Value>()) &&
							(kBytes == 2 || kBytes == 4 || kBytes == 8)));

		  public:
			/**
			 * Ctor for integrals
			 *
			 * \param	retained
			 *			True if input property should retain its last value when link is down or corrupted.
			 * \param	fallback_value
			 *			Value that should be used for nil-values, because integers don't have any special values
			 *			that could be used as nil.
			 */
			template<class = std::enable_if_t<std::is_integral_v<Value>>>
				explicit
				Property (v2::Property<Value>&, Retained retained, Value fallback_value);

			/**
			 * Ctor for floating-point values and SI values
			 *
			 * Separate parameter Offset is used to allow conversion from Quantity<Offset> to Quantity<Value>
			 * if quantities differ by, eg. scaling ratio.
			 *
			 * \param	retained
			 *			True if input property should retain its last value when link is down or corrupted.
			 * \param	offset_value
			 *			If used, set to value where most precision is needed. Useful for 2-byte floats.
			 */
			template<class = std::enable_if_t<std::is_floating_point_v<Value> || si::is_quantity_v<Value>>>
				explicit
				Property (v2::Property<Value>&, Retained retained, std::optional<Value> offset = {});

			Blob::size_type
			size() const override;

			void
			produce (Blob&) override;

			Blob::const_iterator
			eat (Blob::const_iterator, Blob::const_iterator) override;

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
				static Blob::const_iterator
				unserialize (Blob::const_iterator begin, Blob::const_iterator end, SourceType&);

		  private:
			v2::Property<Value>&		_property;
			si::decay_quantity_t<Value>	_fallback_value {};
			std::optional<Value>		_value;
			// Retain last valid value on error (when value is NaN or failsafe kicks in):
			bool						_retained;
			std::optional<Value>		_offset;
			std::function<void (Blob&)>	_produce;
			std::function<Blob::const_iterator (Blob::const_iterator, Blob::const_iterator)>
										_eat;
		};

	/**
	 * An packet that contains boolean or limited-width integers.
	 * Refers to multiple boolean/integer Properties.
	 */
	class Bitfield: public Packet
	{
	  public:
		template<class Value>
			struct BitSource
			{
				v2::Property<Value>&	property;
				// More than 1 bit only makes sense for integer Values:
				uint8_t					bits			{ 1 };
				bool					retained		{ false };
				Value					fallback_value	{};
				Value					value			{};
			};

		using SourceVariant	= std::variant<BitSource<bool>, BitSource<uint8_t>, BitSource<uint16_t>,
										   BitSource<uint32_t>, BitSource<uint64_t>>;

	  public:
		explicit
		Bitfield (std::initializer_list<SourceVariant>);

		Blob::size_type
		size() const override;

		void
		produce (Blob&) override;

		Blob::const_iterator
		eat (Blob::const_iterator, Blob::const_iterator) override;

		void
		apply() override;

		void
		failsafe() override;

	  private:
		std::vector<SourceVariant>	_bit_sources;
		Blob::size_type				_size;
	};

	/**
	 * A packet that adds or verifies simple digital signature of the contained
	 * packets.
	 * HMAC is not required since the Signature packets have fixed size
	 * (length-extension attacks are not possible).  Each Signature must use
	 * different Key.
	 */
	class Signature: public Sequence
	{
	  public:
		explicit
		Signature (NonceBytes, SignatureBytes, Key, PacketList);

		Blob::size_type
		size() const override;

		void
		produce (Blob&) override;

		Blob::const_iterator
		eat (Blob::const_iterator, Blob::const_iterator) override;

	  private:
		uint8_t				_nonce_bytes		{ 0 };
		uint8_t				_signature_bytes	{ 0 };
		Blob				_key;
		std::random_device	_random_device;
		std::mt19937		_rng;
		Blob				_temp;
	};

	/**
	 * A single packet containing a set of packets. Configurable how often should be sent,
	 * also contains magic bytes to be able to distinguish between different Envelopes
	 * coming from remote end.
	 */
	class Envelope: public Sequence
	{
	  public:
		// Ctor
		explicit
		Envelope (Magic, PacketList);

		// Ctor
		explicit
		Envelope (Magic, SendEvery, SendOffset, PacketList);

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

	using EnvelopeList = std::initializer_list<std::shared_ptr<Envelope>>;

  public:
	// Ctor
	explicit
	LinkProtocol (EnvelopeList);

	Blob::size_type
	size() const;

	void
	produce (Blob&);

	Blob::const_iterator
	eat (Blob::const_iterator begin, Blob::const_iterator end, LinkIO* io, QTimer* reacquire_timer, QTimer* failsafe_timer);

	void
	failsafe();

  protected:
	/*
	 * Protocol building functions.
	 */

	template<size_t Bytes, class Value, class = std::enable_if_t<std::is_integral_v<Value>>>
		static auto
		property (v2::Property<Value>& property, Retained retained, Value fallback_value)
		{
			return std::make_shared<Property<Bytes, Value>> (property, retained, fallback_value);
		}

	template<size_t Bytes, class Value, class = std::enable_if_t<std::is_floating_point_v<Value> || si::is_quantity_v<Value>>>
		static auto
		property (v2::Property<Value>& property, Retained retained)
		{
			return std::make_shared<Property<Bytes, Value>> (property, retained);
		}

	template<size_t Bytes, class Value, class Offset, class = std::enable_if_t<std::is_floating_point_v<Value> || si::is_quantity_v<Value>>>
		static auto
		property (v2::Property<Value>& property, Retained retained, Offset offset)
		{
			// Allow Offset to be a Quantity specified over different but compatible Unit (eg. Feet instead of Meters).
			return std::make_shared<Property<Bytes, Value>> (property, retained, std::optional<Value> (offset));
		}

	static auto
	bitfield (std::initializer_list<Bitfield::SourceVariant>&& properties)
	{
		return std::make_shared<Bitfield> (std::forward<std::remove_reference_t<decltype (properties)>> (properties));
	}

	static Bitfield::BitSource<bool>
	bitfield_property (v2::Property<bool>& property, Retained retained, bool fallback_value)
	{
		return { property, 1, *retained, fallback_value, false };
	}

	/**
	 * Note that fallback_value will be used not only when property is nil, but also be used when integer value doesn't
	 * fit in given number of bits.
	 */
	template<class Unsigned>
		static Bitfield::BitSource<Unsigned>
		bitfield_property (v2::Property<Unsigned>& property, Bits bits, Retained retained, Unsigned fallback_value)
		{
			if (!fits_in_bits (fallback_value, bits))
				throw std::invalid_argument ("fallback_value doesn't fit in given number of bits");

			return { property, *bits, *retained, fallback_value, 0 };
		}

	static auto
	signature (NonceBytes nonce_bytes, SignatureBytes signature_bytes, Key key, PacketList&& packets)
	{
		return std::make_shared<Signature> (nonce_bytes, signature_bytes, key, std::forward<PacketList> (packets));
	}

	static auto
	envelope (Magic magic, PacketList&& packets)
	{
		return std::make_shared<Envelope> (magic, std::forward<PacketList> (packets));
	}

	static auto
	envelope (Magic magic, SendEvery send_every, SendOffset send_offset, PacketList&& packets)
	{
		return std::make_shared<Envelope> (magic, send_every, send_offset, std::forward<PacketList> (packets));
	}

  private:
	/**
	 * Convert to user-readable string.
	 * For debugging purposes.
	 */
	static std::string
	to_string (Blob const&);

	static constexpr bool
	fits_in_bits (uint_least64_t value, Bits bits);

  private:
	std::vector<std::shared_ptr<Envelope>>		_envelopes;
	std::map<Blob, std::shared_ptr<Envelope>>	_envelope_magics;
	Blob::size_type								_magic_size		{ 0 };
	Blob										_aux_magic_buffer;
};


class LinkIO: public v2::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	v2::Setting<si::Frequency>		send_frequency			{ this, "send_frequency" };
	v2::Setting<si::Time>			reacquire_after			{ this, "reacquire_after" };
	v2::Setting<si::Time>			failsafe_after			{ this, "failsafe_after" };

	/*
	 * Input
	 */

	v2::PropertyIn<std::string>		link_input				{ this, "/input" };

	/*
	 * Output
	 */

	v2::PropertyOut<std::string>	link_output				{ this, "/output" };
	v2::PropertyOut<bool>			link_valid				{ this, "/link-valid" };
	v2::PropertyOut<int64_t>		link_failsafes			{ this, "/failsafes" };
	v2::PropertyOut<int64_t>		link_reacquires			{ this, "/reacquires" };
	v2::PropertyOut<int64_t>		link_error_bytes		{ this, "/error-bytes" };
	v2::PropertyOut<int64_t>		link_valid_bytes		{ this, "/valid-bytes" };
	v2::PropertyOut<int64_t>		link_valid_envelopes	{ this, "/valid-envelopes" };

  public:
	void
	verify_settings() override;
};


class Link:
	public QObject,
	public v2::Module<LinkIO>
{
	Q_OBJECT

  public:
	// Ctor
	explicit
	Link (std::unique_ptr<LinkIO>, std::unique_ptr<LinkProtocol>, std::string const& instance = {});

	void
	process (v2::Cycle const&) override;

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
	QTimer*							_failsafe_timer		{ nullptr };
	QTimer*							_reacquire_timer	{ nullptr };
	QTimer*							_output_timer		{ nullptr };
	Blob							_input_blob;
	Blob							_output_blob;
	std::unique_ptr<LinkProtocol>	_protocol;
	v2::PropChanged<std::string>	_input_changed		{ io.link_input };
};


template<uint8_t B, class V>
	template<class>
		inline
		LinkProtocol::Property<B, V>::Property (v2::Property<Value>& property, Retained retained, Value fallback_value):
			_property (property),
			_fallback_value (fallback_value),
			_retained (*retained)
		{
			_produce = [this](Blob& blob) {
				int64_t int_value = _property
					? *_property
					: _fallback_value;

				serialize<xf::int_for_width_t<kBytes>> (blob, int_value);
			};

			_eat = [this](Blob::const_iterator begin, Blob::const_iterator end) -> Blob::const_iterator {
				Value value;
				auto result = unserialize<xf::int_for_width_t<kBytes>> (begin, end, value);
				_value = value;
				return result;
			};
		}


template<uint8_t B, class V>
	template<class>
		inline
		LinkProtocol::Property<B, V>::Property (v2::Property<Value>& property, Retained retained, std::optional<Value> offset):
			_property (property),
			_fallback_value (std::numeric_limits<decltype (_fallback_value)>::quiet_NaN()),
			_retained (*retained),
			_offset (offset)
		{
			_produce = [this](Blob& blob) {
				if constexpr (si::is_quantity<Value>())
				{
					typename Value::Value const value = _property
						? _offset
							? (*_property - *_offset).base_quantity()
							: (*_property).base_quantity()
						: _fallback_value;

					serialize<xf::float_for_width_t<kBytes>> (blob, value);
				}
				else if constexpr (std::is_floating_point<Value>())
				{
					Value const value = _property
						? _offset
							? *_property - *_offset
							: *_property
						: _fallback_value;

					serialize<xf::float_for_width_t<kBytes>> (blob, value);
				}
			};

			_eat = [this](Blob::const_iterator begin, Blob::const_iterator end) -> Blob::const_iterator {
				xf::float_for_width_t<kBytes> float_value;

				auto result = unserialize<xf::float_for_width_t<kBytes>> (begin, end, float_value);

				if (std::isnan (float_value))
					_value.reset();
				else
				{
					if constexpr (si::is_quantity<Value>())
						_value = Value { float_value };
					else if constexpr (std::is_floating_point<Value>())
						_value = float_value;
				}

				return result;
			};
		}


template<uint8_t B, class V>
	inline Blob::size_type
	LinkProtocol::Property<B, V>::size() const
	{
		return kBytes;
	}


template<uint8_t B, class V>
	inline void
	LinkProtocol::Property<B, V>::produce (Blob& blob)
	{
		_produce (blob);
	}


template<uint8_t B, class V>
	inline Blob::const_iterator
	LinkProtocol::Property<B, V>::eat (Blob::const_iterator begin, Blob::const_iterator end)
	{
		return _eat (begin, end);
	}


template<uint8_t B, class V>
	inline void
	LinkProtocol::Property<B, V>::apply()
	{
		if constexpr (std::is_integral<Value>())
		{
			if (_value)
				_property = _value;
			else if (!_retained)
				_property.set_nil();
		}
		else if constexpr (std::is_floating_point<Value>() || si::is_quantity<Value>())
		{
			if (_value)
			{
				_property = _offset
					? *_value + *_offset
					: *_value;
			}
			else if (!_retained)
				_property.set_nil();
		}
	}


template<uint8_t B, class V>
	inline void
	LinkProtocol::Property<B, V>::failsafe()
	{
		if (!_retained)
			_property.set_nil();
	}


template<uint8_t B, class V>
	template<class CastType, class SourceType>
		inline void
		LinkProtocol::Property<B, V>::serialize (Blob& blob, SourceType src)
		{
			std::size_t size = sizeof (CastType);
			CastType casted (src);
			boost::endian::native_to_little (casted);
			uint8_t* ptr = reinterpret_cast<uint8_t*> (&casted);
			blob.resize (blob.size() + size);
			std::copy (ptr, ptr + size, &blob[blob.size() - size]);
		}


template<uint8_t B, class V>
	template<class CastType, class SourceType>
		inline Blob::const_iterator
		LinkProtocol::Property<B, V>::unserialize (Blob::const_iterator begin, Blob::const_iterator end, SourceType& src)
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


constexpr bool
LinkProtocol::fits_in_bits (uint_least64_t value, Bits bits)
{
	return value < xf::static_pow (2U, *bits);
}

#endif

