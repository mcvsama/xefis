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
 * Takes two boolean properties and calls 'up' or 'down' callbacks depending on how these boolean values change.
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
		 * Force callback to be called with given delta value, but don't change the internal state of the decoder.
		 */
		void
		force_callback (std::optional<Integer> delta) const;

	  private:
		bool					_prev_a;
		bool					_prev_b;
		Property<bool> const&	_property_a;
		Property<bool> const&	_property_b;
		PropChanged<bool>		_prop_a_changed	{ _property_a };
		PropChanged<bool>		_prop_b_changed	{ _property_b };
		Callback				_callback;
	};


/**
 * QuadratureDecoder with internal counter.
 */
template<class pInteger = int64_t>
	class QuadratureCounter: public QuadratureDecoder<pInteger>
	{
		using typename QuadratureDecoder<pInteger>::Integer;
		using Callback = std::function<void (std::optional<Integer> delta, Integer total)>;

	  public:
		// Ctor
		explicit
		QuadratureCounter (Property<bool> const& property_a, Property<bool> const& property_b, Integer initial_value, Callback callback = [](auto, auto){});

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
			// Both PropChanged need to be called, to save new state:
			auto const a_changed = _prop_a_changed();
			auto const b_changed = _prop_b_changed();

			if (a_changed || b_changed)
			{
				if (_property_a && _property_b)
				{
					Integer const a = *_property_a;
					Integer const b = *_property_b;
					Integer const da = a - _prev_a;
					Integer const db = b - _prev_b;

					// If nothing changed… nothing changed - do nothing:
					if (da == 0 && db == 0)
						return; // No change.

					// If both inputs changed (it should not happen), don't call the callback:
					if (da == 0 || db == 0)
					{
						if ((da == 1 && b == 0) || (a == 1 && db == 1) || (da == -1 && b == 1) || (a == 0 && db == -1))
							_callback (-1);
						else
							_callback (+1);
					}
					else
						_callback (std::nullopt);

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
	QuadratureDecoder<I>::force_callback (std::optional<Integer> const delta) const
	{
		_callback (delta);
	}


template<class I>
	inline
	QuadratureCounter<I>::QuadratureCounter (Property<bool> const& property_a, Property<bool> const& property_b, Integer initial_value, Callback callback):
		QuadratureDecoder<I> (property_a, property_b, [this] (auto delta) { decoder_callback (delta); }),
		_total (initial_value),
		_callback (callback)
	{ }


template<class I>
	inline auto
	QuadratureCounter<I>::value() const noexcept -> Integer
	{
		return _total;
	}


template<class I>
	inline void
	QuadratureCounter<I>::decoder_callback (std::optional<Integer> const delta)
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

