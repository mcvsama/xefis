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

#ifndef XEFIS__UTILITY__ROTARY_ENCODER_H__INCLUDED
#define XEFIS__UTILITY__ROTARY_ENCODER_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>


namespace Xefis {

/**
 * Takes two boolean properties and
 * calls 'up' or 'down' callbacks depending
 * on how these boolean values change.
 * TODO RENAME TO RotaryDecoder
 */
class RotaryEncoder
{
  public:
	typedef std::function<void (int delta)> Callback;

  public:
	// Ctor:
	RotaryEncoder (Xefis::PropertyBoolean& property_a, Xefis::PropertyBoolean& property_b, Callback callback);

	/**
	 * Signals that properties have been
	 * updated. May call the callback.
	 */
	void
	data_updated();

	/**
	 * Force callback to be called with given delta value.
	 */
	void
	operator() (int delta);

	/**
	 * Alias for operator().
	 */
	void
	call (int delta);

  private:
	bool					_prev_a;
	bool					_prev_b;
	Xefis::PropertyBoolean&	_property_a;
	Xefis::PropertyBoolean&	_property_b;
	Callback				_callback;
};


inline void
RotaryEncoder::operator() (int delta)
{
	_callback (delta);
}


inline void
RotaryEncoder::call (int delta)
{
	_callback (delta);
}

} // namespace Xefis

#endif

