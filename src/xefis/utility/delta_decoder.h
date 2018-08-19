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

#ifndef XEFIS__UTILITY__DELTA_DECODER_H__INCLUDED
#define XEFIS__UTILITY__DELTA_DECODER_H__INCLUDED

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

template<class pInteger = int64_t>
	class DeltaDecoder
	{
		static_assert (std::is_integral<pInteger>());
		static_assert (std::is_signed<pInteger>());

	  public:
		using Integer	= pInteger;
		using Callback	= std::function<void (std::optional<Integer> delta)>;

	  public:
		// Ctor
		explicit
		DeltaDecoder (Property<Integer> const& property, Callback callback, Integer initial_value = {});

		/**
		 * Signals that properties have been updated. May call the callback.
		 */
		void
		operator()();

		/**
		 * Force callback to be called with given delta value, but don't change internal state of the decoder.
		 */
		void
		force_callback (std::optional<Integer> delta) const;

	  private:
		Integer						_previous;
		Property<Integer> const&	_value_property;
		PropChanged<Integer>		_property_changed	{ _value_property };
		Callback					_callback;
	};


template<class I>
	inline
	DeltaDecoder<I>::DeltaDecoder (Property<Integer> const& property, Callback callback, Integer initial_value):
		_previous (initial_value),
		_value_property (property),
		_callback (callback)
	{ }


template<class I>
	inline void
	DeltaDecoder<I>::operator()()
	{
		if (_callback && _property_changed())
		{
			if (_value_property)
			{
				auto const current = *_value_property;
				_callback (current - _previous);
				_previous = current;
			}
			else
				_callback (std::nullopt);
		}
	}


template<class I>
	inline void
	DeltaDecoder<I>::force_callback (std::optional<Integer> delta) const
	{
		_callback (delta);
	}

} // namespace xf

#endif

