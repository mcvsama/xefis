/* vim:ts=4
 *
 * Copyleft 2012…2017  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__MATH__LONLAT_RADIUS_H__INCLUDED
#define XEFIS__SUPPORT__MATH__LONLAT_RADIUS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * Polar coordinates on Earth.
 */
class LonLatRadius: public si::LonLat
{
  public:
	// Ctor
	explicit constexpr
	LonLatRadius (LonLat const&, si::Length radius);

	// Ctor
	explicit constexpr
	LonLatRadius (si::Angle longitude, si::Angle latitude, si::Length radius);

	[[nodiscard]]
	constexpr si::Length&
	radius() noexcept;

	[[nodiscard]]
	constexpr si::Length
	radius() const noexcept;

  private:
	si::Length _radius;
};


constexpr
LonLatRadius::LonLatRadius (LonLat const& lonlat, si::Length radius):
	LonLat (lonlat),
	_radius (radius)
{ }


constexpr
LonLatRadius::LonLatRadius (si::Angle longitude, si::Angle latitude, si::Length radius):
	LonLat (longitude, latitude),
	_radius (radius)
{ }


constexpr si::Length&
LonLatRadius::radius() noexcept
{
	return _radius;
}


constexpr si::Length
LonLatRadius::radius() const noexcept
{
	return _radius;
}

} // namespace xf

#endif

