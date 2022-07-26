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

#ifndef XEFIS__SUPPORT__SOCKETS__SOCKET_VALUE_CHANGED_H__INCLUDED
#define XEFIS__SUPPORT__SOCKETS__SOCKET_VALUE_CHANGED_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/socket.h>
#include <xefis/support/sockets/socket_changed.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * Checks if socket's value changed since last check.
 */
template<class pValue>
	class SocketValueChanged: public SocketChanged
	{
	  public:
		using Value = pValue;

	  public:
		// Ctor
		explicit
		SocketValueChanged (Socket<Value>& socket);

		/**
		 * Return true if socket's value changed since last check.
		 */
		[[nodiscard]]
		bool
		value_changed();

		/**
		 * Return true if socket's value changed to given value since last check.
		 */
		[[nodiscard]]
		bool
		value_changed_to (std::optional<Value> const& expected_value)
			{ return value_changed() && _socket.get_optional() == expected_value; }

		[[nodiscard]]
		Socket<Value>&
		socket() noexcept
			{ return _socket; }

		[[nodiscard]]
		Socket<Value> const&
		socket() const noexcept
			{ return _socket; }

	  protected:
		bool
		perhaps_shift_cycles() override;

	  private:
		Socket<Value>&			_socket;
		std::optional<Value>	_prev_value;
		std::optional<Value>	_curr_value;
	};


template<class V>
	inline
	SocketValueChanged<V>::SocketValueChanged (Socket<Value>& socket):
		SocketChanged (socket),
		_socket (socket)
	{ }


template<class V>
	inline bool
	SocketValueChanged<V>::value_changed()
	{
		return perhaps_shift_cycles() && _prev_value != _curr_value;
	}


template<class V>
	inline bool
	SocketValueChanged<V>::perhaps_shift_cycles()
	{
		auto const shifted = SocketChanged::perhaps_shift_cycles();

		if (shifted)
		{
			_prev_value = _curr_value;
			_curr_value = _socket.get_optional();
		}

		return shifted;
	}

} // namespace xf

#endif

