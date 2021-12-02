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

#ifndef XEFIS__SUPPORT__SOCKETS__SOCKET_QUADRATURE_DECODER_H__INCLUDED
#define XEFIS__SUPPORT__SOCKETS__SOCKET_QUADRATURE_DECODER_H__INCLUDED

// Standard:
#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/socket.h>
#include <xefis/support/sockets/socket_changed.h>


namespace xf {

template<class T>
	concept QuadratureDecoderValueConcept = std::signed_integral<T>;


/**
 * Takes two boolean sockets and calls 'up' or 'down' callbacks depending on how these boolean values change.
 */
template<QuadratureDecoderValueConcept pInteger = int64_t>
	class SocketQuadratureDecoder
	{
	  public:
		using Integer	= pInteger;
		using Action	= std::function<void (std::optional<Integer> delta)>;

	  public:
		// Ctor
		explicit
		SocketQuadratureDecoder (Socket<bool>& socket_a, Socket<bool>& socket_b, Action);

		/**
		 * Signals that sockets have been updated. May call the callback.
		 */
		void
		process();

		/**
		 * Force callback to be called with given delta value, but don't change the internal state of the decoder.
		 */
		void
		call_action (std::optional<Integer> delta) const;

	  private:
		bool			_prev_a;
		bool			_prev_b;
		Socket<bool>&	_socket_a;
		Socket<bool>&	_socket_b;
		SocketChanged	_socket_a_changed	{ _socket_a };
		SocketChanged	_socket_b_changed	{ _socket_b };
		Action			_action;
	};


template<QuadratureDecoderValueConcept I>
	inline
	SocketQuadratureDecoder<I>::SocketQuadratureDecoder (Socket<bool>& socket_a, Socket<bool>& socket_b, Action const action):
		_prev_a (socket_a.value_or (false)),
		_prev_b (socket_b.value_or (false)),
		_socket_a (socket_a),
		_socket_b (socket_b),
		_action (action)
	{ }


template<QuadratureDecoderValueConcept I>
	inline void
	SocketQuadratureDecoder<I>::process()
	{
		if (_action)
		{
			auto const a_changed = _socket_a_changed.serial_changed();
			auto const b_changed = _socket_b_changed.serial_changed();

			if (a_changed || b_changed)
			{
				if (_socket_a && _socket_b)
				{
					Integer const a = *_socket_a;
					Integer const b = *_socket_b;
					Integer const da = a - _prev_a;
					Integer const db = b - _prev_b;

					// If nothing changed… nothing changed - do nothing:
					if (da == 0 && db == 0)
						return; // No change.

					// If both inputs changed (it should not happen), don't call the callback:
					if (da == 0 || db == 0)
					{
						if ((da == 1 && b == 0) || (a == 1 && db == 1) || (da == -1 && b == 1) || (a == 0 && db == -1))
							_action (-1);
						else
							_action (+1);
					}
					else
						_action (std::nullopt);

					_prev_a = a;
					_prev_b = b;
				}
				else
					_action (std::nullopt);
			}
		}
	}


template<QuadratureDecoderValueConcept I>
	inline void
	SocketQuadratureDecoder<I>::call_action (std::optional<Integer> const delta) const
	{
		_action (delta);
	}

} // namespace xf

#endif

