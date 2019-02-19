/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef NEUTRINO__SI__QUANTITIES__LONLAT_H__INCLUDED
#define NEUTRINO__SI__QUANTITIES__LONLAT_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QPointF>

// Local:
#include "standard_quantities.h"
#include "standard_literals.h"


namespace si {

using namespace literals;
using quantities::Angle;
using quantities::Length;


/**
 * ECEF coordinates (Earth-centered Earth-fixed).
 */
class LonLat
{
  public:
	// Ctor
	constexpr LonLat() noexcept;

	/**
	 * \param	longitude Angle between -180_deg and 180_deg.
	 * \param	latitude Angle between -90_deg and 90_deg.
	 */
	constexpr LonLat (Angle longitude, Angle latitude) noexcept;

  public:
	constexpr Angle&
	lat() noexcept
		{ return _lat; }

	constexpr Angle const&
	lat() const noexcept
		{ return _lat; }

	constexpr Angle&
	lon() noexcept
		{ return _lon; }

	constexpr Angle const&
	lon() const noexcept
		{ return _lon; }

	/**
	 * Change position of this point on a sphere by adding new angles.
	 */
	LonLat&
	rotate (LonLat const& rotation);

	/**
	 * Like rotate(), but not in-place.
	 */
	LonLat
	rotated (LonLat const& rotation) const;

	QPointF
	project_flat() const;

  private:
	Angle	_lon;
	Angle	_lat;
};


static_assert (std::is_literal_type<LonLat>(), "LonLat must be a literal type");


constexpr
LonLat::LonLat() noexcept:
	LonLat (0_deg, 0_deg)
{ }


constexpr
LonLat::LonLat (Angle longitude, Angle latitude) noexcept:
	_lon (longitude),
	_lat (latitude)
{ }

} // namespace si

#endif

