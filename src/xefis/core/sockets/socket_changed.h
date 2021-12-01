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

#ifndef XEFIS__CORE__SOCKETS__SOCKET_CHANGED_H__INCLUDED
#define XEFIS__CORE__SOCKETS__SOCKET_CHANGED_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

/**
 * Allow checking if socket's value/serial number changed since previous
 * loop cycle.
 */
template<class pValue>
	class SocketChanged
	{
	  public:
		using Value = pValue;

	  public:
		// Ctor
		explicit
		SocketChanged (Socket<Value>& socket);

		/**
		 * Return true if socket's value changed since last cycle.
		 */
		[[nodiscard]]
		bool
		value_changed (Cycle const&);

		/**
		 * Return true if socket's value changed to given value since last cycle.
		 */
		[[nodiscard]]
		bool
		value_changed_to (std::optional<Value> const& expected_value, Cycle const& cycle)
			{ return value_changed (cycle) && _socket->get_optional() == expected_value; }

		/**
		 * Return true if socket's serial number changed since last cycle.
		 */
		[[nodiscard]]
		bool
		serial_changed (Cycle const& cycle);

		[[nodiscard]]
		Socket<Value>&
		socket() noexcept
			{ return *_socket; }

		[[nodiscard]]
		Socket<Value> const&
		socket() const noexcept
			{ return *_socket; }

	  private:
		void
		perhaps_shift_cycles (Cycle const& cycle);

	  private:
		Socket<Value>*			_socket;
		std::optional<Value>	_prev_value;
		Cycle::Number			_prev_cycle_number;
		Socket<Value>::Serial	_prev_serial;
		std::optional<Value>	_curr_value;
		Cycle::Number			_curr_cycle_number;
		Socket<Value>::Serial	_curr_serial;
	};


template<class V>
	inline
	SocketChanged<V>::SocketChanged (Socket<Value>& socket):
		_socket (&socket)
	{ }


template<class V>
	inline bool
	SocketChanged<V>::value_changed (Cycle const& cycle)
	{
		perhaps_shift_cycles (cycle);
		return _prev_value != _curr_value;
	}


template<class V>
	inline bool
	SocketChanged<V>::serial_changed (Cycle const& cycle)
	{
		perhaps_shift_cycles (cycle);
		return _prev_serial != _curr_serial;
	}


template<class V>
	void
	SocketChanged<V>::perhaps_shift_cycles (Cycle const& cycle)
	{
		if (cycle.number() > _curr_cycle_number)
		{
			_prev_value = _curr_value;
			_prev_cycle_number = _curr_cycle_number;
			_curr_value = _socket->get_optional();
			_curr_cycle_number = cycle.number();
		}
	}

} // namespace xf

#endif

