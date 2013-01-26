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

#ifndef XEFIS__UTILITY__LONLAT_H__INCLUDED
#define XEFIS__UTILITY__LONLAT_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QPointF>

// Xefis:
#include <xefis/config/all.h>


class LonLat
{
  public:
	typedef Degrees ValueType;

  public:
	// Ctor
	LonLat();

	// Ctor
	LonLat (Degrees longitude, Degrees latitude);

  public:
	Degrees&
	lat();

	Degrees const&
	lat() const;

	Degrees&
	lon();

	Degrees const&
	lon() const;

	LonLat&
	rotate (LonLat const& rotation);

	LonLat
	rotated (LonLat const& rotation) const;

	QPointF
	project_flat() const;

	/**
	 * Compute distance between two sets of coordinates on Earth.
	 * Result is in sphere radius units.
	 */
	ValueType
	haversine (LonLat const& other) const;

	/**
	 * Convenience function.
	 * Compute distance between two sets of coordinates on Earth.
	 * Result is in kilometers.
	 */
	ValueType
	haversine_km (LonLat const& other) const;

	/**
	 * Convenience function.
	 * Compute distance between two sets of coordinates on Earth.
	 * Result is in nautical miles.
	 */
	ValueType
	haversine_nm (LonLat const& other) const;

  private:
	Degrees	_lon;
	Degrees	_lat;
};


inline
LonLat::LonLat():
	LonLat (0.f, 0.f)
{ }


inline
LonLat::LonLat (Degrees longitude, Degrees latitude):
	_lon (longitude),
	_lat (latitude)
{ }


inline Degrees&
LonLat::lon()
{
	return _lon;
}


inline Degrees const&
LonLat::lon() const
{
	return _lon;
}


inline Degrees&
LonLat::lat()
{
	return _lat;
}


inline Degrees const&
LonLat::lat() const
{
	return _lat;
}


/**
 * Convenience function.
 * Compute distance between two sets of coordinates on Earth.
 * Result is in kilometers.
 */
inline LonLat::ValueType
LonLat::haversine_km (LonLat const& other) const
{
	return haversine (other) * EARTH_MEAN_RADIUS_KM;
}


/**
 * Convenience function.
 * Compute distance between two sets of coordinates on Earth.
 * Result is in nautical miles.
 */
inline LonLat::ValueType
LonLat::haversine_nm (LonLat const& other) const
{
	return haversine (other) * EARTH_MEAN_RADIUS_NM;
}

#endif

