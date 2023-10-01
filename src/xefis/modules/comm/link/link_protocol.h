/* vim:ts=4
 *
 * Copyleft 2012…2023  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__COMM__LINK__PROTOCOL_H__INCLUDED
#define XEFIS__MODULES__COMM__LINK__PROTOCOL_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/assignable_socket.h>
#include <xefis/modules/comm/xle_transceiver.h>

// Neutrino:
#include <neutrino/endian.h>
#include <neutrino/logger.h>
#include <neutrino/memory.h>
#include <neutrino/numeric.h>
#include <neutrino/stdexcept.h>

// Qt:
#include <QtCore/QTimer>

// Standard:
#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <limits>
#include <optional>
#include <random>
#include <variant>
#include <vector>


#define XEFIS_LINK_SEND_DEBUG 0
#define XEFIS_LINK_RECV_DEBUG 0


namespace si = neutrino::si;

class InputLink;


class LinkProtocol
{
  public:
	/**
	 * Thrown on known parse errors.
	 * TODO use std::expect to be faster?
	 */
	class ParseError
	{ };

	/**
	 * Thrown by sub-packets when there's no enough input data
	 * Note that each Envelope's consume() is called when it's known for sure that there's enough data in the input buffer
	 * to cover whole Envelope.
	 */
	class InsufficientDataError
	{ };

	/**
	 * Thrown when one of Envelopes has different unique_prefix string size than the others.
	 */
	class InvalidMagicSize: public xf::Exception
	{
	  public:
		explicit
		InvalidMagicSize():
			Exception ("invalid unique_prefix string length; envelopes' unique_prefix strings must be the same length")
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
		produce (Blob&, xf::Logger const&) = 0;

		/**
		 * Parse data and set temporary variables.
		 * Data will be output when apply() is called.
		 */
		virtual Blob::const_iterator
		consume (Blob::const_iterator, Blob::const_iterator, xf::Logger const&) = 0;

		/**
		 * Apply parsed data to sockets, etc.
		 */
		virtual void
		apply() = 0;

		/**
		 * Set all managed sockets to nil.
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
		produce (Blob&, xf::Logger const&) override;

		Blob::const_iterator
		consume (Blob::const_iterator, Blob::const_iterator, xf::Logger const&) override;

		void
		apply() override;

		void
		failsafe() override;

	  private:
		std::vector<std::shared_ptr<Packet>> _packets;
	};

	/**
	 * Packet that refers to a particular Socket, so it can send/receive value of that module socket.
	 */
	template<uint16_t pBytes, class pValue>
		class Socket: public Packet
		{
		  public:
			using Value = pValue;

			/**
			 * This one is used with integer values.
			 */
			struct IntegerParams
			{
				// True if module input should retain its last value when link is down or corrupted.
				bool					retained	{ false };
				// Value that should be used for nil-values, because integers don't have any special values
				// that could be used as nil.
				// Note this value_if_nil is only used on transmitting side only, if module socket is nil.
				Value					value_if_nil;
			};

			/**
			 * This one is for floating-point and SI values.
			 */
			struct FloatingPointParams
			{
				// True if module input should retain its last value when link is down or corrupted.
				bool					retained	{ false };
				// If used, set to value where most precision is needed. Useful for 2-byte floats.
				std::optional<Value>	offset		{ };
			};

			/**
			 * This one is used with string values.
			 */
			struct StringParams
			{
				// True if module input should retain its last value when link is down or corrupted.
				bool					retained	{ false };
				bool					truncate	{ false };
			};

			static constexpr uint16_t kBytes { pBytes };

			static uint16_t constexpr kTruncatedStringMetaSize = 2;
			static uint16_t constexpr kUntruncatedStringMetaSize = 6;
			static uint16_t constexpr kNilStringSize = 0xffff;
			static uint16_t constexpr kMaxStringSize = kNilStringSize - 1;

			static_assert ((std::integral<Value> && (kBytes == 1 || kBytes == 2 || kBytes == 4 || kBytes == 8)) ||
						   (si::FloatingPointOrQuantity<Value> && (kBytes == 2 || kBytes == 4 || kBytes == 8)) ||
						   (std::is_same_v<Value, std::string> && (kBytes >= 1 && kBytes <= kMaxStringSize)));

		  private:
			/**
			 * Ctor for integer types.
			 */
			explicit
			Socket (xf::Socket<Value>&, xf::AssignableSocket<Value>*, IntegerParams&&)
				requires std::integral<Value>;

			/**
			 * Ctor for floating-point values and SI values.
			 */
			explicit
			Socket (xf::Socket<Value>&, xf::AssignableSocket<Value>*, FloatingPointParams&&)
				requires si::FloatingPointOrQuantity<Value>;

			/**
			 * Ctor for string values.
			 */
			explicit
			Socket (xf::Socket<std::string>&, xf::AssignableSocket<std::string>*, StringParams&&);

		  public:
			/**
			 * Ctor for read-only integral sockets.
			 */
			explicit
			Socket (xf::Socket<Value>& socket, IntegerParams&& params)
				requires std::integral<Value>:
				Socket (socket, nullptr, std::move (params))
			{ }

			/**
			 * Ctor for writable integral sockets.
			 */
			explicit
			Socket (xf::AssignableSocket<Value>& assignable_socket, IntegerParams&& params)
				requires std::integral<Value>:
				Socket (assignable_socket, &assignable_socket, std::move (params))
			{ }

			/**
			 * Ctor for read-only floating-point and SI-value sockets.
			 */
			explicit
			Socket (xf::Socket<Value>& socket, FloatingPointParams&& params)
				requires si::FloatingPointOrQuantity<Value>:
				Socket (socket, nullptr, std::move (params))
			{ }

			/**
			 * Ctor for writable floating-point and SI-value sockets.
			 */
			explicit
			Socket (xf::AssignableSocket<Value>& assignable_socket, FloatingPointParams&& params)
				requires si::FloatingPointOrQuantity<Value>:
				Socket (assignable_socket, &assignable_socket, std::move (params))
			{ }

			/**
			 * Ctor for read-only string sockets.
			 */
			explicit
			Socket (xf::Socket<std::string>& socket, StringParams&& params):
				Socket (socket, nullptr, std::move (params))
			{ }

			/**
			 * Ctor for writable string sockets.
			 */
			explicit
			Socket (xf::AssignableSocket<std::string>& assignable_socket, StringParams&& params):
				Socket (assignable_socket, &assignable_socket, std::move (params))
			{ }

			Blob::size_type
			size() const override;

			void
			produce (Blob& blob, xf::Logger const&) override
				{ _produce (blob); }

			Blob::const_iterator
			consume (Blob::const_iterator const begin, Blob::const_iterator const end, xf::Logger const&) override
				{ return _consume (begin, end); }

			void
			apply() override;

			void
			failsafe() override;

		  private:
			/**
			 * Serialize SourceType and add to Blob (append at the end of the blob).
			 */
			template<class CastType, class SourceType>
				void
				serialize (SourceType, Blob&);

			/**
			 * Unserialize data from Blob and put it to src.
			 */
			template<class CastType>
				[[nodiscard]]
				Blob::const_iterator
				unserialize (Blob::const_iterator begin, Blob::const_iterator end);

		  private:
			xf::Socket<Value>&				_socket;
			xf::AssignableSocket<Value>*	_assignable_socket;
			si::decay_quantity_t<Value>		_value_if_nil		{};
			std::optional<Value>			_value;
			// Retain last valid value on error (when value is NaN or failsafe kicks in):
			bool							_retained;
			std::optional<Value>			_offset;
			std::function<void (Blob&)>		_produce;
			bool							_truncated_string	{ true };
			std::function<Blob::const_iterator (Blob::const_iterator, Blob::const_iterator)>
											_consume;
			size_t							_cycle_number		{ 0 };
			uint16_t						_current_serial		{ 0xffff };
			std::vector<bool>				_received_blocks;
			bool							_recovered			{ false };
			std::string						_recovered_string;
		};

	/**
	 * An packet that contains boolean or limited-width integers.
	 * Refers to multiple boolean/integer sockets.
	 */
	class Bitfield: public Packet
	{
	  public:
		struct BoolParams
		{
			bool		retained		{ false };
			bool		value_if_nil	{ false };

			static constexpr BoolParams make_default()
			{
				return { .retained = false, .value_if_nil = false };
			}
		};

		template<class Value>
			struct UnsignedParams
			{
				uint8_t		bits			{ std::numeric_limits<Value>::digits };
				bool		retained		{ false };
				Value		value_if_nil	{ false };
			};

		template<class Value>
			struct BitSource
			{
				xf::Socket<Value>&				socket;
				xf::AssignableSocket<Value>*	assignable_socket;
				uint8_t							bits;
				bool							retained;
				Value							value_if_nil;
				Value							value {};
			};

		using SourceVariant	= std::variant<BitSource<bool>, BitSource<uint8_t>, BitSource<uint16_t>,
										   BitSource<uint32_t>, BitSource<uint64_t>>;

	  public:
		explicit
		Bitfield (std::initializer_list<SourceVariant>);

		Blob::size_type
		size() const override;

		void
		produce (Blob&, xf::Logger const&) override;

		Blob::const_iterator
		consume (Blob::const_iterator, Blob::const_iterator, xf::Logger const&) override;

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
	 * HMAC is not required since the Signature packets have fixed size, so
	 * length-extension attacks are not possible. Each Signature must use
	 * different Key.
	 */
	class Signature: public Sequence
	{
	  public:
		struct Params
		{
			uint8_t			nonce_bytes;
			uint8_t			signature_bytes;
			Blob			key;
			PacketList&&	packets;
		};

	  public:
		explicit
		Signature (Params&&);

		Blob::size_type
		size() const override;

		void
		produce (Blob&, xf::Logger const&) override;

		Blob::const_iterator
		consume (Blob::const_iterator, Blob::const_iterator, xf::Logger const&) override;

	  private:
		uint8_t			_nonce_bytes		{ 0 };
		uint8_t			_signature_bytes	{ 0 };
		Blob			_key;
		std::mt19937	_rng;
		Blob			_temp;
	};

	/**
	 * A single packet containing a set of packets. Configurable how often should be sent,
	 * also contains unique_prefix to be able to distinguish between different Envelopes
	 * coming from remote end.
	 */
	class Envelope: public Sequence
	{
	  public:
		struct Params
		{
			// Magic is a unique envelope identifier vector:
			Blob							unique_prefix;
			// Only send this envelope every Nth time:
			size_t							send_every		{ 1 };
			// Start sending first packet from send_offset:
			size_t							send_offset		{ 0 };
			// If provided, the envelope will be sent-out if and only if this function returns true.
			// `send_every` is not taken into account while `send_predicate()` returns false.
			std::function<bool()>			send_predicate	{ nullptr };
			// Optional encryption transceiver. If provided, the envelope will not be sent
			// until the transceiver reports connected() == true.
			xf::crypto::xle::Transceiver*	transceiver		{ nullptr };
			// List of packets this envelope will comprise:
			PacketList						packets			{};
		};

	  public:
		// Ctor
		explicit
		Envelope (Params&&);

		Blob const&
		unique_prefix() const;

		Blob::size_type
		size() const override;

		void
		produce (Blob& blob, xf::Logger const&) override;

		Blob::const_iterator
		consume (Blob::const_iterator, Blob::const_iterator, xf::Logger const&) override;

	  private:
		Blob							_unique_prefix;
		uint64_t						_send_every;
		uint64_t						_send_offset;
		uint64_t						_send_pos	{ 0 };
		std::function<bool()>			_send_predicate;
		xf::crypto::xle::Transceiver*	_transceiver;
	};

	using EnvelopeList = std::initializer_list<std::shared_ptr<Envelope>>;

  public:
	// Ctor
	explicit
	LinkProtocol (EnvelopeList);

	Blob::size_type
	size() const;

	void
	produce (Blob&, xf::Logger const&);

	Blob::const_iterator
	consume (Blob::const_iterator begin, Blob::const_iterator end, InputLink*, QTimer* reacquire_timer, QTimer* failsafe_timer, xf::Logger const&);

	void
	failsafe();

  protected:
	/*
	 * Protocol building functions.
	 */

	template<size_t Bytes, std::integral Value>
		static auto
		socket (xf::Socket<Value>& socket, Socket<Bytes, Value>::IntegerParams&& params = {})
		{
			return std::make_shared<Socket<Bytes, Value>> (socket, std::move (params));
		}

	template<size_t Bytes, std::integral Value>
		static auto
		socket (xf::AssignableSocket<Value>& assignable_socket, Socket<Bytes, Value>::IntegerParams&& params = {})
		{
			return std::make_shared<Socket<Bytes, Value>> (assignable_socket, std::move (params));
		}

	template<size_t Bytes, si::FloatingPointOrQuantity Value>
		static auto
		socket (xf::Socket<Value>& socket, Socket<Bytes, Value>::FloatingPointParams&& params = {})
		{
			return std::make_shared<Socket<Bytes, Value>> (socket, std::move (params));
		}

	template<size_t Bytes, si::FloatingPointOrQuantity Value>
		static auto
		socket (xf::AssignableSocket<Value>& assignable_socket, Socket<Bytes, Value>::FloatingPointParams&& params = {})
		{
			return std::make_shared<Socket<Bytes, Value>> (assignable_socket, std::move (params));
		}

	template<size_t Bytes, si::FloatingPointOrQuantity Value, class Offset>
		static auto
		socket (xf::Socket<Value>& socket, Socket<Bytes, Value>::FloatingPointParams&& params = {})
		{
			// Allow Offset to be a Quantity specified over different but compatible Unit (eg. Feet instead of Meters).
			return std::make_shared<Socket<Bytes, Value>> (socket, {
				.retained	= params.retained,
				.offset		= std::optional<Value> (params.offset),
			});
		}

	template<size_t Bytes, si::FloatingPointOrQuantity Value, class Offset>
		static auto
		socket (xf::AssignableSocket<Value>& assignable_socket, Socket<Bytes, Value>::FloatingPointParams&& params = {})
		{
			// Allow Offset to be a Quantity specified over different but compatible Unit (eg. Feet instead of Meters).
			return std::make_shared<Socket<Bytes, Value>> (assignable_socket, {
				.retained	= params.retained,
				.offset		= std::optional<Value> (params.offset),
			});
		}

	template<size_t Bytes>
		static auto
		socket (xf::Socket<std::string>& socket, Socket<Bytes, std::string>::StringParams&& params = {})
		{
			return std::make_shared<Socket<Bytes, std::string>> (socket, std::move (params));
		}

	template<size_t Bytes>
		static auto
		socket (xf::AssignableSocket<std::string>& assignable_socket, Socket<Bytes, std::string>::StringParams&& params = {})
		{
			return std::make_shared<Socket<Bytes, std::string>> (assignable_socket, std::move (params));
		}

	static auto
	bitfield (std::initializer_list<Bitfield::SourceVariant>&& sockets)
	{
		return std::make_shared<Bitfield> (std::forward<std::remove_reference_t<decltype (sockets)>> (sockets));
	}

	// FIXME The = ::make_default() is a workaround for possible bug in GCC, where = {} doesn't work.
	static Bitfield::BitSource<bool>
	bitfield_socket (xf::Socket<bool>& socket, Bitfield::BoolParams&& params = Bitfield::BoolParams::make_default())
	{
		return {
			.socket				= socket,
			.assignable_socket	= nullptr,
			.bits				= 1,
			.retained			= params.retained,
			.value_if_nil		= params.value_if_nil,
			.value				= false,
		};
	}

	static Bitfield::BitSource<bool>
	bitfield_socket (xf::AssignableSocket<bool>& assignable_socket, Bitfield::BoolParams&& params = Bitfield::BoolParams::make_default())
	{
		return {
			.socket				= assignable_socket,
			.assignable_socket	= &assignable_socket,
			.bits				= 1,
			.retained			= params.retained,
			.value_if_nil		= params.value_if_nil,
			.value				= false,
		};
	}

	/**
	 * Note that value_if_nil will be used not only when socket is nil, but also be used when integer value doesn't
	 * fit in given number of bits.
	 */
	template<class Unsigned>
		static Bitfield::BitSource<Unsigned>
		bitfield_socket (xf::Socket<Unsigned>& socket, Bitfield::UnsignedParams<Unsigned>&& params = {})
		{
			if (!fits_in_bits (params.value_if_nil, params.bits))
				throw xf::InvalidArgument ("value_if_nil doesn't fit in given number of bits");

			return {
				.socket				= socket,
				.assignable_socket	= nullptr,
				.bits				= params.bits,
				.retained			= params.retained,
				.value_if_nil		= params.value_if_nil,
				.value				= 0,
			};
		}

	/**
	 * Note that value_if_nil will be used not only when socket is nil, but also be used when integer value doesn't
	 * fit in given number of bits.
	 */
	template<class Unsigned>
		static Bitfield::BitSource<Unsigned>
		bitfield_socket (xf::AssignableSocket<Unsigned>& assignable_socket, Bitfield::UnsignedParams<Unsigned>&& params = {})
		{
			if (!fits_in_bits (params.value_if_nil, params.bits))
				throw xf::InvalidArgument ("value_if_nil doesn't fit in given number of bits");

			return {
				.socket				= assignable_socket,
				.assignable_socket	= &assignable_socket,
				.bits				= params.bits,
				.retained			= params.retained,
				.value_if_nil		= params.value_if_nil,
				.value				= 0,
			};
		}

	static auto
	signature (Signature::Params&& params)
	{
		return std::make_shared<Signature> (std::move (params));
	}

	static auto
	envelope (Envelope::Params&& params)
	{
		return std::make_shared<Envelope> (std::move (params));
	}

  private:
	static constexpr bool
	fits_in_bits (uint_least64_t value, uint8_t bits)
		{ return value == 0 || value < xf::static_pow<uint_least64_t> (2U, bits); }

  private:
	std::vector<std::shared_ptr<Envelope>>		_envelopes;
	std::map<Blob, std::shared_ptr<Envelope>>	_envelope_unique_prefixes;
	Blob::size_type								_unique_prefix_size { 0 };
	Blob										_aux_unique_prefix_buffer;
};


template<uint16_t B, class V>
	inline
	LinkProtocol::Socket<B, V>::Socket (xf::Socket<Value>& socket, xf::AssignableSocket<Value>* assignable_socket, IntegerParams&& params)
		requires std::integral<Value>:
		_socket (socket),
		_assignable_socket (assignable_socket),
		_value_if_nil (params.value_if_nil),
		_retained (params.retained)
	{
		_produce = [this] (Blob& blob) {
			int64_t int_value = _socket
				? *_socket
				: _value_if_nil;

			serialize<xf::int_for_width_t<kBytes>> (int_value, blob);
		};

		_consume = [this] (Blob::const_iterator begin, Blob::const_iterator end) -> Blob::const_iterator {
			return unserialize<xf::int_for_width_t<kBytes>> (begin, end);
		};
	}


template<uint16_t B, class V>
	inline
	LinkProtocol::Socket<B, V>::Socket (xf::Socket<Value>& socket, xf::AssignableSocket<Value>* assignable_socket, FloatingPointParams&& params)
		requires si::FloatingPointOrQuantity<Value>:
		_socket (socket),
		_assignable_socket (assignable_socket),
		_value_if_nil (std::numeric_limits<decltype (_value_if_nil)>::quiet_NaN()),
		_retained (params.retained),
		_offset (params.offset)
	{
		_produce = [this] (Blob& blob) {
			if constexpr (si::is_quantity<Value>())
			{
				typename Value::Value const value = _socket
					? _offset
						? (*_socket - *_offset).base_value()
						: (*_socket).base_value()
					: _value_if_nil;

				serialize<neutrino::float_for_width_t<kBytes>> (value, blob);
			}
			else if constexpr (std::is_floating_point<Value>())
			{
				Value const value = _socket
					? _offset
						? *_socket - *_offset
						: *_socket
					: _value_if_nil;

				serialize<neutrino::float_for_width_t<kBytes>> (value, blob);
			}
		};

		_consume = [this] (Blob::const_iterator begin, Blob::const_iterator end) -> Blob::const_iterator {
			return unserialize<neutrino::float_for_width_t<kBytes>> (begin, end);
		};
	}


template<uint16_t B, class V>
	inline
	LinkProtocol::Socket<B, V>::Socket (xf::Socket<std::string>& socket, xf::AssignableSocket<std::string>* assignable_socket, StringParams&& params):
		_socket (socket),
		_assignable_socket (assignable_socket),
		_retained (params.retained),
		_truncated_string (params.truncate)
	{
		_produce = [this] (Blob& blob) {
			// Only needed to see if the value changed, so modulo 16-bit is good enough:
			_current_serial = _socket.serial() % 0xffff;
			serialize<void> (_socket.get_optional(), blob);
		};

		_consume = [this] (Blob::const_iterator begin, Blob::const_iterator end) -> Blob::const_iterator {
			return unserialize<void> (begin, end);
		};
	}


template<uint16_t B, class V>
	inline Blob::size_type
	LinkProtocol::Socket<B, V>::size() const
	{
		if constexpr (std::is_same_v<Value, std::string>)
		{
			if (_truncated_string)
				return kTruncatedStringMetaSize + kBytes;
			else
				return kUntruncatedStringMetaSize + kBytes;
		}
		else
			return kBytes;
	}


template<uint16_t B, class V>
	inline void
	LinkProtocol::Socket<B, V>::apply()
	{
		if (_assignable_socket)
		{
			if constexpr (std::is_integral<Value>() || std::is_same_v<Value, std::string>)
			{
				if (_value)
					*_assignable_socket = _value;
				else if (!_retained)
					*_assignable_socket = xf::nil;
			}
			else if constexpr (si::FloatingPointOrQuantity<Value>)
			{
				if (_value)
				{
					*_assignable_socket = _offset
						? *_value + *_offset
						: *_value;
				}
				else if (!_retained)
					*_assignable_socket = xf::nil;
			}
			else
				static_assert (false, "missing LinkProtocol::Socket<>::apply() implementation");
		}
	}


template<uint16_t B, class V>
	inline void
	LinkProtocol::Socket<B, V>::failsafe()
	{
		if (_assignable_socket && !_retained)
			*_assignable_socket = xf::nil;
	}


template<uint16_t B, class V>
	template<class CastType, class SourceType>
		inline void
		LinkProtocol::Socket<B, V>::serialize (SourceType src, Blob& blob)
		{
			if constexpr (std::is_same_v<SourceType, std::optional<std::string>>)
			{
				auto const append_pos = blob.size();

				if (_truncated_string)
				{
					auto constexpr kMetaB = kTruncatedStringMetaSize;
					auto constexpr kBufferB = kBytes;

					blob.resize (blob.size() + kMetaB + kBufferB);

					if (src)
					{
						auto& string = *src;
						// Strings longer than kBufferB will be truncated:
						uint16_t const size = std::min<size_t> (kBufferB, string.size());
						uint16_t const le_size = size;
						neutrino::perhaps_native_to_little_inplace (le_size);

						auto append_iterator = blob.begin() + neutrino::to_signed (append_pos);
						append_iterator = neutrino::copy_value_to_memory (le_size, append_iterator);
						append_iterator = std::copy (string.begin(), string.begin() + size, append_iterator);
						std::fill (append_iterator, blob.end(), 0);
					}
					else
					{
						uint16_t const le_nil = kNilStringSize;
						neutrino::perhaps_native_to_little_inplace (le_nil);

						auto const append_iterator = neutrino::copy_value_to_memory (le_nil, blob.begin() + neutrino::to_signed (append_pos));
						std::fill (append_iterator, blob.end(), 0);
					}
				}
				else
				{
					auto constexpr kMetaB = kUntruncatedStringMetaSize;
					auto constexpr kBufferB = kBytes;

					blob.resize (blob.size() + kMetaB + kBufferB);

					auto const append_meta = []<class AppendIterator>(uint16_t serial, uint16_t size, uint16_t block_number, AppendIterator append_iterator)
						-> AppendIterator
					{
						neutrino::perhaps_native_to_little_inplace (serial);
						neutrino::perhaps_native_to_little_inplace (size);
						neutrino::perhaps_native_to_little_inplace (block_number);

						append_iterator = neutrino::copy_value_to_memory (serial, append_iterator);
						append_iterator = neutrino::copy_value_to_memory (size, append_iterator);
						append_iterator = neutrino::copy_value_to_memory (block_number, append_iterator);

						return append_iterator;
					};

					auto append_iterator = blob.begin() + neutrino::to_signed (append_pos);

					if (src)
					{
						auto& string = *src;

						if (string.size() > kMaxStringSize)
							throw xf::Exception (std::format ("LinkProtocol: can't encode string longer than {} bytes", kMaxStringSize));

						if (string.empty())
							append_iterator = append_meta (_current_serial, string.size(), 0, append_iterator);
						else
						{
							uint16_t const block_size = kBufferB;
							// num_blocks will be at least 1:
							auto const num_blocks = (string.size() - 1u) / block_size + 1u;

							uint16_t const block_number = _cycle_number % num_blocks;
							append_iterator = append_meta (_current_serial, string.size(), block_number, append_iterator);

							auto bytes_to_copy = (block_number < num_blocks - 1)
								? block_size
								: string.size() % block_size;

							if (bytes_to_copy == 0)
								bytes_to_copy = block_size;

							auto const copy_begin = string.begin() + block_number * block_size;
							auto const copy_end = copy_begin + neutrino::to_signed (bytes_to_copy);
							append_iterator = std::copy (copy_begin, copy_end, append_iterator);
						}

						// Fill the rest (or maybe the whole) buffer with zeros:
						std::fill (append_iterator, blob.end(), 0);

						++_cycle_number;
					}
					else
					{
						append_iterator = append_meta (_current_serial, kNilStringSize, 0, append_iterator);
						std::fill (append_iterator, blob.end(), 0);
					}
				}
			}
			else
			{
				auto const size = sizeof (CastType);
				auto casted = static_cast<CastType> (src);
				neutrino::perhaps_native_to_little_inplace (casted);
				uint8_t* ptr = reinterpret_cast<uint8_t*> (&casted);
				blob.resize (blob.size() + size);
				std::copy (ptr, ptr + size, &blob[blob.size() - size]);
			}
		}


template<uint16_t B, class V>
	template<class CastType>
		inline Blob::const_iterator
		LinkProtocol::Socket<B, V>::unserialize (Blob::const_iterator begin, Blob::const_iterator end)
		{
			if constexpr (std::is_same_v<Value, std::string>)
			{
				if (_truncated_string)
				{
					auto constexpr kMetaB = kTruncatedStringMetaSize;
					auto constexpr kBufferB = kBytes;

					if (neutrino::to_unsigned (std::distance (begin, end)) < kMetaB + kBufferB)
						throw ParseError();

					uint16_t size;
					auto read_iterator = neutrino::copy_memory_to_value (begin, size);
					neutrino::perhaps_little_to_native_inplace (size);

					if (size == kNilStringSize)
						_value.reset();
					else
					{
						if (size > kBufferB)
							throw ParseError();

						std::string string;
						string.resize (size);
						std::copy (read_iterator, read_iterator + size, string.data());
						_value = std::move (string);
					}

					return begin + kMetaB + kBufferB;
				}
				else
				{
					auto constexpr kMetaB = kUntruncatedStringMetaSize;
					auto constexpr kBufferB = kBytes;
					auto const block_size = kBufferB;

					uint16_t serial;
					uint16_t size;
					uint16_t block_number;

					auto read_iterator = neutrino::copy_memory_to_value (begin, serial);
					read_iterator = neutrino::copy_memory_to_value (read_iterator, size);
					read_iterator = neutrino::copy_memory_to_value (read_iterator, block_number);

					neutrino::perhaps_little_to_native_inplace (serial);
					neutrino::perhaps_little_to_native_inplace (size);
					neutrino::perhaps_little_to_native_inplace (block_number);

					if (size == kNilStringSize)
					{
						if (!_recovered)
						{
							_recovered = true;
							_value.reset();
						}
					}
					else
					{
						if (serial != _current_serial)
						{
							_current_serial = serial;

							if (size > 0)
							{
								_received_blocks.resize ((size - 1u) / block_size + 1u);
								std::ranges::fill (_received_blocks, false);
								_recovered = false;
								_recovered_string.resize (size);
							}
							else
							{
								_received_blocks.clear();
								_recovered = true;
								_value = "";
							}
						}

						if (!_recovered)
						{
							if (size != _recovered_string.size())
								throw ParseError();

							if (block_number >= _received_blocks.size())
								throw ParseError();

							// num_blocks will be at least 1:
							auto const num_blocks = (size - 1u) / block_size + 1u;

							if (!_received_blocks[block_number])
							{
								_received_blocks[block_number] = true;

								auto bytes_to_copy = (block_number < num_blocks - 1)
									? block_size
									: size % block_size;

								if (bytes_to_copy == 0)
									bytes_to_copy = block_size;

								std::copy (read_iterator, read_iterator + bytes_to_copy, &_recovered_string[block_number * block_size]);

								if (std::ranges::all_of (_received_blocks, [](bool value) { return value; }))
								{
									_recovered = true;
									_value = _recovered_string;
								}
							}
						}
					}

					return begin + kMetaB + kBufferB;
				}
			}
			else
			{
				if (neutrino::to_unsigned (std::distance (begin, end)) < sizeof (CastType))
					throw ParseError();

				std::size_t size = sizeof (CastType);
				auto const work_end = begin + neutrino::to_signed (size);
				CastType cast_value;
				std::copy (begin, work_end, reinterpret_cast<uint8_t*> (&cast_value));
				neutrino::perhaps_little_to_native_inplace (cast_value);

				if constexpr (si::FloatingPointOrQuantity<Value>)
				{
					if (std::isnan (cast_value))
						_value.reset();
					else if constexpr (si::is_quantity<Value>())
						_value = Value { cast_value };
					else if constexpr (std::is_floating_point<Value>())
						_value = cast_value;
				}
				else
					_value = cast_value;

				return work_end;
			}
		}

#endif

