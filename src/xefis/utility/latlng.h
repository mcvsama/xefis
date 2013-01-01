/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__UTILITY__LATLNG_H__INCLUDED
#define XEFIS__UTILITY__LATLNG_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>


class LatLng
{
  public:
	// Ctor
	LatLng();

	// Ctor
	LatLng (Degrees latitude, Degrees longitude);

  public:
	Degrees&
	lat();

	Degrees const&
	lat() const;

	Degrees&
	lng();

	Degrees const&
	lng() const;

  private:
	Degrees	_lat;
	Degrees	_lng;
};


inline
LatLng::LatLng():
	LatLng (0.f, 0.f)
{ }


inline
LatLng::LatLng (Degrees latitude, Degrees longitude):
	_lat (latitude),
	_lng (longitude)
{ }


inline Degrees&
LatLng::lat()
{
	return _lat;
}


inline Degrees const&
LatLng::lat() const
{
	return _lat;
}


inline Degrees&
LatLng::lng()
{
	return _lng;
}


inline Degrees const&
LatLng::lng() const
{
	return _lng;
}

#endif

