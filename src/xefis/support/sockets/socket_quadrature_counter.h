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

#ifndef XEFIS__SUPPORT__SOCKETS__SOCKET_QUADRATURE_COUNTER_H__INCLUDED
#define XEFIS__SUPPORT__SOCKETS__SOCKET_QUADRATURE_COUNTER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/sockets/socket.h>
#include <xefis/support/sockets/socket_quadrature_decoder.h>

// Standard:
#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>


namespace xf {

/**
 * SocketQuadratureDecoder with internal counter.
 */
template<QuadratureDecoderValueConcept pInteger = int64_t>
	class SocketQuadratureCounter: public SocketQuadratureDecoder<pInteger>
	{
		using typename SocketQuadratureDecoder<pInteger>::Integer;
		using Callback = std::function<void (std::optional<Integer> delta, Integer total)>;

	  public:
		// Ctor
		explicit
		SocketQuadratureCounter (Socket<bool>& socket_a, Socket<bool>& socket_b, Integer initial_value, Callback callback = [](auto, auto){});

		/**
		 * Return stored counted value.
		 */
		Integer
		value() const noexcept;

	  private:
		void
		decoder_callback (std::optional<Integer> delta);

	  private:
		Integer		_total;
		Callback	_callback;
	};


template<QuadratureDecoderValueConcept I>
	inline
	SocketQuadratureCounter<I>::SocketQuadratureCounter (Socket<bool>& socket_a, Socket<bool>& socket_b, Integer initial_value, Callback callback):
		SocketQuadratureDecoder<I> (socket_a, socket_b, [this] (auto delta) { decoder_callback (delta); }),
		_total (initial_value),
		_callback (callback)
	{ }


template<QuadratureDecoderValueConcept I>
	inline auto
	SocketQuadratureCounter<I>::value() const noexcept -> Integer
	{
		return _total;
	}


template<QuadratureDecoderValueConcept I>
	inline void
	SocketQuadratureCounter<I>::decoder_callback (std::optional<Integer> const delta)
	{
		if (delta)
		{
			_total += *delta;
			_callback (delta, _total);
		}
		else
			_callback (std::nullopt, _total);
	}

} // namespace xf

#endif

