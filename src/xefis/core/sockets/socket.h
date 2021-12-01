/* vim:ts=4
 *
 * Copyleft 2021  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__SOCKETS__SOCKET_H__INCLUDED
#define XEFIS__CORE__SOCKETS__SOCKET_H__INCLUDED

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
#include <xefis/core/sockets/basic_socket.h>
#include <xefis/core/sockets/common.h>
#include <xefis/core/sockets/exception.h>
#include <xefis/core/sockets/socket_converter.h>


namespace xf {

template<class ObservedValue, class AssignedValue>
	class ConnectableSocket;


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
		 * Create a Socket with no initial value.
		 */
		explicit
		Socket() = default;

		/**
		 * Create a Socket with initial value.
		 */
		explicit
		Socket (Value const& value, std::optional<Value> const& fallback_value = {});

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


#include <xefis/core/sockets/socket_traits.h> // TODO move to top?


namespace xf {

template<class V>
	inline
	Socket<V>::Socket (Value const& value, std::optional<Value> const& fallback_value):
		_value (value),
		_fallback_value (fallback_value)
	{ }


template<class V>
	inline bool
	Socket<V>::operator== (Socket<Value> const& other)
	{
		return (!_value && !other._value)
			|| (_value && other._value && *_value == *other._value);
	}


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

