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

#ifndef XEFIS__CORE__SOCKETS__ASSIGNABLE_SOCKET_H__INCLUDED
#define XEFIS__CORE__SOCKETS__ASSIGNABLE_SOCKET_H__INCLUDED

// Standard:
#include <cstddef>

// Neutrino:
#include <neutrino/blob.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/basic_assignable_socket.h>
#include <xefis/core/sockets/socket.h>
#include <xefis/utility/is_optional.h>


namespace xf {

/**
 * A Socket to which user can assign value directly.
 */
template<class pValue>
	class AssignableSocket:
		virtual public BasicAssignableSocket,
		public Socket<pValue>
	{
	  public:
		using Value = pValue;

	  public:
		using Socket<Value>::Socket;

		/**
		 * Alias for AssignableSocket::protected_set_nil().
		 */
		void
		operator= (Nil) override
			{ this->protected_set_nil(); }

		/**
		 * Copies other socket's held value, not entire state.
		 */
		AssignableSocket<Value> const&
		operator= (Socket<Value> const&);

		/**
		 * Alias for AssignableSocket::protected_set_value (Value const&)
		 */
		AssignableSocket const&
		operator= (Value const&);

		/**
		 * Alias for AssignableSocket::protected_set (std::optional<Value>)
		 */
		template<class OptionalType>
			requires (is_optional_v<OptionalType>)
			AssignableSocket const&
			operator= (OptionalType const&);

		// BasicAssignableSocket API
		void
		from_string (std::string_view const& str, SocketConversionSettings const& settings = {}) override;

		// BasicAssignableSocket API
		void
		from_blob (BlobView const& blob) override;
	};

} // namespace xf


// Xefis:
#include <xefis/core/sockets/socket_traits.h>


namespace xf {

template<class V>
	inline AssignableSocket<V> const&
	AssignableSocket<V>::operator= (Socket<Value> const& other)
	{
		this->protected_set (other);
		return *this;
	}


template<class V>
	inline AssignableSocket<V> const&
	AssignableSocket<V>::operator= (Value const& value)
	{
		this->protected_set_value (value);
		return *this;
	}


template<class V>
	template<class OptionalType>
		requires (is_optional_v<OptionalType>)
		inline AssignableSocket<V> const&
		AssignableSocket<V>::operator= (OptionalType const& optional)
		{
			this->protected_set (optional);
			return *this;
		}


template<class V>
	inline void
	AssignableSocket<V>::from_string (std::string_view const& str, SocketConversionSettings const& settings)
	{
		SocketTraits<Value>::from_string (*this, str, settings);
	}


template<class V>
	inline void
	AssignableSocket<V>::from_blob (BlobView const& blob)
	{
		SocketTraits<Value>::from_blob (*this, blob);
	}

} // namespace xf

#endif

