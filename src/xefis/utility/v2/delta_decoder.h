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

#ifndef XEFIS__UTILITY__V2__DELTA_DECODER_H__INCLUDED
#define XEFIS__UTILITY__V2__DELTA_DECODER_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/property.h>


namespace x2 {

class DeltaDecoder
{
  public:
	typedef std::function<void (int delta)> Callback;

  public:
	// Ctor
	DeltaDecoder (PropertyIn<int64_t>& value_property, Callback callback);

	/**
	 * Signals that properties have been updated. May call the callback.
	 */
	void
	operator()();

	/**
	 * Force callback to be called with given delta value.
	 */
	void
	force_callback (int delta);

  private:
	int64_t						_prev;
	x2::PropertyIn<int64_t>&	_property;
	Callback					_callback;
};


inline void
DeltaDecoder::force_callback (int delta)
{
	_callback (delta);
}

} // namespace x2

#endif

