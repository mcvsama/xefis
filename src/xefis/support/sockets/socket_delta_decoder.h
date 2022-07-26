/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SOCKETS__SOCKET_DELTA_DECODER_H__INCLUDED
#define XEFIS__SUPPORT__SOCKETS__SOCKET_DELTA_DECODER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/socket.h>
#include <xefis/support/sockets/socket_value_changed.h>

// Standard:
#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>


namespace xf {

template<class T>
	concept DeltaDecoderValueConcept = std::signed_integral<T>;


template<DeltaDecoderValueConcept pInteger = int64_t>
	class SocketDeltaDecoder
	{
	  public:
		using Integer	= pInteger;
		using Action	= std::function<void (std::optional<Integer> delta)>;

	  public:
		// Ctor
		explicit
		SocketDeltaDecoder (Socket<Integer>& socket, Action action, Integer initial_value = {});

		/**
		 * Signals that sockets have been updated. May call the action.
		 */
		void
		process();

		/**
		 * Force action to be called with given delta value, but don't change internal state of the decoder.
		 */
		void
		call_action (std::optional<Integer> delta) const;

	  private:
		Integer						_previous;
		Socket<Integer>&			_value_socket;
		SocketValueChanged<Integer>	_socket_value_changed	{ _value_socket };
		Action						_action;
	};


template<DeltaDecoderValueConcept I>
	inline
	SocketDeltaDecoder<I>::SocketDeltaDecoder (Socket<Integer>& socket, Action const action, Integer const initial_value):
		_previous (initial_value),
		_value_socket (socket),
		_action (action)
	{ }


template<DeltaDecoderValueConcept I>
	inline void
	SocketDeltaDecoder<I>::process()
	{
		if (_action && _socket_value_changed.value_changed())
		{
			if (_value_socket)
			{
				auto const current = *_value_socket;
				_action (current - _previous);
				_previous = current;
			}
			else
				_action (std::nullopt);
		}
	}


template<DeltaDecoderValueConcept I>
	inline void
	SocketDeltaDecoder<I>::call_action (std::optional<Integer> delta) const
	{
		_action (delta);
	}

} // namespace xf

#endif

