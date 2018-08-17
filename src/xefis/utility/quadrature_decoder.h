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

#ifndef XEFIS__UTILITY__QUADRATURE_DECODER_H__INCLUDED
#define XEFIS__UTILITY__QUADRATURE_DECODER_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <xefis/utility/actions.h>


namespace xf {

/**
 * Takes two boolean properties and
 * calls 'up' or 'down' callbacks depending
 * on how these boolean values change.
 */
template<class pInteger = int64_t>
	class QuadratureDecoder
	{
		static_assert (std::is_integral<pInteger>());
		static_assert (std::is_signed<pInteger>());

	  public:
		using Integer	= pInteger;
		using Callback	= std::function<void (std::optional<Integer> delta)>;

	  public:
		// Ctor
		explicit
		QuadratureDecoder (Property<bool> const& property_a, Property<bool> const& property_b, Callback callback);

		/**
		 * Signals that properties have been updated. May call the callback.
		 */
		void
		operator()();

		/**
		 * Force callback to be called with given delta value.
		 */
		void
		force_callback (std::optional<Integer> delta) const;

	  private:
		bool					_prev_a;
		bool					_prev_b;
		Property<bool> const&	_property_a;
		Property<bool> const&	_property_b;
		PropChanged<Integer>	_prop_a_changed	{ _property_a };
		PropChanged<Integer>	_prop_b_changed	{ _property_b };
		Callback				_callback;
	};


template<class I>
	inline
	QuadratureDecoder<I>::QuadratureDecoder (Property<bool> const& property_a, Property<bool> const& property_b, Callback callback):
		_prev_a (property_a.value_or (false)),
		_prev_b (property_b.value_or (false)),
		_property_a (property_a),
		_property_b (property_b),
		_callback (callback)
	{ }


template<class I>
	inline void
	QuadratureDecoder<I>::operator()()
	{
		if (_callback)
		{
			if (_prop_a_changed() || _prop_b_changed())
			{
				if (_property_a && _property_b)
				{
					Integer const a = *_property_a;
					Integer const b = *_property_b;
					Integer const da = _prev_a - a;
					Integer const db = _prev_b - b;

					if (da == 0 && db == 0)
						return; // No change.

					if ((da == 1 && b == 0) || (a == 1 && db == 1) || (da == -1 && b == 1) || (a == 0 && db == -1))
						_callback (+1);
					else
						_callback (-1);

					_prev_a = a;
					_prev_b = b;
				}
				else
					_callback (std::nullopt);
			}
		}
	}


template<class I>
	inline void
	QuadratureDecoder<I>::force_callback (std::optional<Integer> delta) const
	{
		_callback (delta);
	}

} // namespace xf

#endif

