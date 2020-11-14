/* vim:ts=4
 *
 * Copyleft 2020  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__SOCKET_H__INCLUDED
#define XEFIS__CORE__SOCKET_H__INCLUDED

// Standard:
#include <variant>

// Neutrino:
#include <neutrino/noncopyable.h>
#include <neutrino/nonmovable.h>
#include <neutrino/numeric.h>
#include <neutrino/variant.h>
#include <neutrino/utility.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/cycle.h>
#include <xefis/core/socket_converter.h>


namespace xf {

/**
 * Helper type that indicates Nil values for sockets.
 */
class Nil
{ };


/**
 * Helper type that's used to reset data source for a socket.
 */
class NoDataSource
{ };


/**
 * Wrapper for values that are supposed to act as a constant value source for Socket objects.
 * The value provided by this object is stored in Socket class.
 */
template<class Value>
	class ConstantSource
	{
	  public:
		explicit
		ConstantSource (Value const& value):
			value (value)
		{ }

		Value value;
	};


/**
 * Global nil object that when compared to a nil socket, gives true.
 */
static constexpr Nil nil;


/**
 * Global NoDataSource object.
 */
static constexpr NoDataSource no_data_source;


/**
 * Exception object thrown when trying to read a nil socket.
 */
class NilValueException: public Exception
{
  public:
	// Ctor
	explicit
	NilValueException();
};


template<class Value>
	class ConnectableSocket;


/**
 * A value holder.
 * Can source its value from a connected socket.
 * Can be source of a value for other connected sockets.
 *
 * TODO Note: perhaps the *_age() methods should not use the timestamp of the set() call, but some timestamp provided from
 * outside, eg. some souce data sampling timestamp. That would be more proper from digital signal processing
 * perspective, but I guess it's OK enough as it is now.
 */
class BasicSocket:
	private Noncopyable,
	private Nonmovable
{
	template<class Value>
		friend class Socket;

	template<class Value>
		friend class ConnectableSocket;

  public:
	// Used to tell if node value has changed:
	typedef uint64_t Serial;

  public:
	// Ctor
	BasicSocket();

	// Dtor
	virtual
	~BasicSocket();

	// Move operator

	/**
	 * Ensure that socket's value is up to date in this processing loop.
	 */
	void
	fetch (Cycle const&);

	/**
	 * Set no data source for this socket.
	 */
	virtual void
	operator<< (NoDataSource) = 0;

	/**
	 * Return true if socket is nil.
	 * If a fallback-value is set, it will never return true.
	 */
	[[nodiscard]]
	virtual bool
	is_nil() const noexcept = 0;

	/**
	 * Alias for is_nil().
	 */
	[[nodiscard]]
	bool
	operator== (Nil) const
		{ return is_nil(); }

	/**
	 * Valid means not nil. Equivalent to !is_nil().
	 */
	[[nodiscard]]
	virtual bool
	valid() const noexcept = 0;

	/**
	 * Alias for valid().
	 */
	[[nodiscard]]
	operator bool() const noexcept
		{ return valid(); }

	/**
	 * Return the serial value of the socket.
	 * Serial value changes when socket is updated.
	 */
	[[nodiscard]]
	Serial
	serial() const noexcept
		{ return _serial; }

	/**
	 * Return timestamp of the value (time when it was modified).
	 */
	[[nodiscard]]
	si::Time
	modification_timestamp() const noexcept
		{ return _modification_timestamp; }

	/**
	 * Return age of the value (time since it was last modified).
	 */
	[[nodiscard]]
	si::Time
	modification_age() const noexcept
		{ return TimeHelper::now() - modification_timestamp(); }

	/**
	 * Return timestamp of the last non-nil value.
	 */
	[[nodiscard]]
	si::Time
	valid_timestamp() const noexcept
		{ return _valid_timestamp; }

	/**
	 * Return age of the non-nil value (time since it was last set to a non-nil value).
	 * Setting a fallback-value will essentially mean setting not-nil.
	 */
	[[nodiscard]]
	si::Time
	valid_age() const noexcept
		{ return TimeHelper::now() - valid_timestamp(); }

	/**
	 * Use-count for this socket.
	 * This is number of sockets reading value from this socket.
	 */
	[[nodiscard]]
	std::size_t
	use_count() const noexcept
		{ return _targets.size(); }

	/**
	 * Return true if Blob returned by to_blob() is constant size.
	 */
	[[nodiscard]]
	virtual bool
	has_constant_blob_size() const noexcept = 0;

	/**
	 * Return Blob size for this Socket, provided that has_constant_blob_size() is true.
	 * If has_constant_blob_size() gives false, return 0.
	 * TODO or perhaps throw std::logic_error?
	 */
	[[nodiscard]]
	virtual size_t
	constant_blob_size() const noexcept = 0;

	/**
	 * Serialize socket's value to string with given config.
	 */
	[[nodiscard]]
	virtual std::string
	to_string (SocketConversionSettings const& = {}) const = 0;

	/**
	 * Extract numeric value from the socket, if applies.
	 */
	[[nodiscard]]
	virtual std::optional<float128_t>
	to_floating_point (SocketConversionSettings const& = {}) const = 0;

	/**
	 * Serializes socket's value, including nil-flag.
	 */
	[[nodiscard]]
	virtual Blob
	to_blob() const = 0;

  protected:
	/**
	 * Fetch the data from the source unconditionally.
	 */
	virtual void
	do_fetch (Cycle const&) = 0;

	/**
	 * Increase use-count of this socket (listener started listening to value of this socket).
	 */
	void
	inc_use_count (BasicSocket* listener)
		{ _targets.push_back (listener); }

	/**
	 * Decrease use-count of this socket (listener stopped listening to value of this socket).
	 */
	void
	dec_use_count (BasicSocket* listener);

	/**
	 * Set socket to the nil value.
	 */
	virtual void
	protected_set_nil() = 0;

  private:
	si::Time					_modification_timestamp	= 0_s;
	si::Time					_valid_timestamp		= 0_s;
	Serial						_serial					= 0;
	Cycle::Number				_fetched_cycle_number	= 0;
	std::vector<BasicSocket*>	_targets;
};


/**
 * Holds the actual value, fallback value.
 */
template<class pValue>
	class Socket: virtual public BasicSocket
	{
	  public:
		using Value = pValue;

	  public:
		/**
		 * Compare current values with another Socket, nil-value included.
		 * Nothing else is compared (eg. fallback value), only the current value.
		 */
		[[nodiscard]]
		bool
		operator== (Socket<Value> const& other);

		/**
		 * Return contained value.
		 * Throw exception NilValueException if value is nil and no fallback-value is set.
		 */
		[[nodiscard]]
		Value const&
		get() const;

		/**
		 * Alias for get().
		 */
		[[nodiscard]]
		Value const&
		operator*() const
			{ return get(); }

		/**
		 * Return std::optional that has value or is empty, if this socket is nil.
		 * If fallback-value is set, the returned std::optional will contain the fall-back value, and will never be empty.
		 */
		[[nodiscard]]
		std::optional<Value>
		get_optional() const;

		/**
		 * Return socket's value or argument if socket is nil.
		 * If socket has a fallback-value set, then value_or will never return its argument, it will fall back to the
		 * fallback-value first.
		 */
		[[nodiscard]]
		Value
		value_or (Value fallback) const;

		/**
		 * Pointer accessor if contained object is pointer-dereferencable.
		 * Throws the same exception as get() under the same conditions.
		 *
		 * Note that only the const accessor is available to prevent accidental mutation of the inner value without Socket knowing it
		 * (and updating modification timestamps, etc).
		 */
		[[nodiscard]]
		Value const*
		operator->() const
			{ return &get(); }

		/**
		 * Set fallback-value to use when this socket isn't connected to any other socket or its value is nil.
		 * Socket with a fallback-value will essentially be seen as it's never nil.
		 *
		 * Affects value-retrieving methods and their aliases: get(), get_optional(), is_nil(), *_timestamp(), *_age(),
		 * valid(), serial().
		 *
		 * Pass empty std::optional (or std::nullopt) to remove the fallback-value.
		 */
		void
		set_fallback (std::optional<Value>);

		// BasicSocket API
		[[nodiscard]]
		bool
		is_nil() const noexcept override
			{ return !_value && !_fallback_value; }

		// BasicSocket API
		[[nodiscard]]
		bool
		valid() const noexcept override
			{ return !is_nil(); }

		// BasicSocket API
		[[nodiscard]]
		bool
		has_constant_blob_size() const noexcept override;

		// BasicSocket API
		[[nodiscard]]
		size_t
		constant_blob_size() const noexcept override;

		// BasicSocket API
		[[nodiscard]]
		std::string
		to_string (SocketConversionSettings const& = {}) const override;

		// BasicSocket API
		[[nodiscard]]
		std::optional<float128_t>
		to_floating_point (SocketConversionSettings const& = {}) const override;

		// BasicSocket API
		[[nodiscard]]
		Blob
		to_blob() const override;

	  protected:
		// BasicSocket API
		void
		protected_set_nil() override;

		/**
		 * Copy value (or nil-state) from other proprety.
		 */
		void
		protected_set (Socket<Value> const&);

		/**
		 * Set new value or set to nil, of std::optional is empty.
		 */
		void
		protected_set (std::optional<Value> const&);

		/**
		 * Set new value.
		 */
		void
		protected_set_value (Value const&);

	  private:
		std::optional<Value>	_value;
		std::optional<Value>	_fallback_value;
	};

} // namespace xf


#include <xefis/core/socket_traits.h>


namespace xf {

/*
 * NilValueException
 */


inline
NilValueException::NilValueException():
	Exception ("tried to read a nil socket")
{ }


/*
 * BasicSocket
 */


inline
BasicSocket::BasicSocket()
{
	_targets.reserve (8);
}


inline
BasicSocket::~BasicSocket()
{
	for (auto* target: clone (_targets))
		(*target) << no_data_source;
}


inline void
BasicSocket::fetch (Cycle const& cycle)
{
	if (_fetched_cycle_number < cycle.number())
	{
		_fetched_cycle_number = cycle.number();
		do_fetch (cycle);
	}
}


inline void
BasicSocket::dec_use_count (BasicSocket* listener)
{
	auto new_end = std::remove (_targets.begin(), _targets.end(), listener);
	_targets.resize (neutrino::to_unsigned (std::distance (_targets.begin(), new_end)));
}


/*
 * Socket<V>
 */


template<class V>
	inline typename Socket<V>::Value const&
	Socket<V>::get() const
	{
		if (_value)
			return *_value;
		else if (_fallback_value)
			return *_fallback_value;
		else
			throw NilValueException();
	}


template<class V>
	bool
	Socket<V>::operator== (Socket<Value> const& other)
	{
		return (!_value && !other._value)
			|| (_value && other._value && *_value == *other._value);
	}


template<class V>
	inline std::optional<typename Socket<V>::Value>
	Socket<V>::get_optional() const
	{
		if (_value)
			return _value;
		else
			return _fallback_value;
	}


template<class V>
	inline typename Socket<V>::Value
	Socket<V>::value_or (Value fallback) const
	{
		if (_value)
			return *_value;
		else if (_fallback_value)
			return *_fallback_value;
		else
			return fallback;
	}


template<class V>
	inline void
	Socket<V>::set_fallback (std::optional<Value> fallback_value)
	{
		if (_fallback_value != fallback_value)
		{
			_modification_timestamp = TimeHelper::now();
			_valid_timestamp = _modification_timestamp;
			_fallback_value = fallback_value;
			++_serial;
		}
	}


template<class V>
	inline bool
	Socket<V>::has_constant_blob_size() const noexcept
	{
		return SocketTraits<Value>::has_constant_blob_size();
	}


template<class V>
	inline size_t
	Socket<V>::constant_blob_size() const noexcept
	{
		return SocketTraits<Value>::constant_blob_size();
	}


template<class V>
	inline std::string
	Socket<V>::to_string (SocketConversionSettings const& settings) const
	{
		return SocketTraits<Value>::to_string (*this, settings);
	}


template<class V>
	inline std::optional<float128_t>
	Socket<V>::to_floating_point (SocketConversionSettings const& settings) const
	{
		return SocketTraits<Value>::to_floating_point (*this, settings);
	}


template<class V>
	inline Blob
	Socket<V>::to_blob() const
	{
		return SocketTraits<Value>::to_blob (*this);
	}


template<class V>
	inline void
	Socket<V>::protected_set_nil()
	{
		if (_value)
		{
			_modification_timestamp = TimeHelper::now();
			_value.reset();
			++_serial;
		}
	}


template<class V>
	inline void
	Socket<V>::protected_set (Socket<Value> const& value)
	{
		if (value)
			protected_set_value (*value);
		else
			protected_set_nil();
	}


template<class V>
	inline void
	Socket<V>::protected_set (std::optional<Value> const& value)
	{
		if (value)
			protected_set_value (*value);
		else
			protected_set_nil();
	}


template<class V>
	inline void
	Socket<V>::protected_set_value (Value const& value)
	{
		if (!_value || *_value != value)
		{
			_modification_timestamp = TimeHelper::now();
			_valid_timestamp = _modification_timestamp;
			_value = value;
			++_serial;
		}
	}

} // namespace xf

#endif

