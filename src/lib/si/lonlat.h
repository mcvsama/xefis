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

#ifndef SI__LONLAT_H__INCLUDED
#define SI__LONLAT_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtCore/QPointF>

// Local:
#include "constants.h"
#include "angle.h"


namespace SI {

class LonLat
{
  public:
	typedef Angle ValueType;

  public:
	// Ctor
	LonLat();

	/**
	 * \param	longitude Angle between -180_deg and 180_deg.
	 * \param	latitude Angle between -90_deg and 90_deg.
	 */
	LonLat (Angle longitude, Angle latitude);

  public:
	Angle&
	lat();

	Angle const&
	lat() const;

	Angle&
	lon();

	Angle const&
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
	ValueType::ValueType
	haversine (LonLat const& other) const;

	/**
	 * Convenience function.
	 * Compute distance between two sets of coordinates on Earth.
	 * Result is in nautical miles.
	 */
	Length
	haversine_earth (LonLat const& other) const;

	/**
	 * Initial bearing when flying orthodrome (great circle path)
	 * to another point. Final bearing will be: (result + 180_deg) % 360_deg.
	 * Result is in range [-180_deg, +180_deg].
	 */
	Angle
	initial_bearing (LonLat const& other) const;

	/**
	 * Compute angle between two great arcs on a sphere.
	 * Arcs are given by three points, the second one lies on the intersection.
	 * Result is in degrees.
	 */
	static Angle
	great_arcs_angle (LonLat const& a, LonLat const& common, LonLat const& b);

  private:
	Angle	_lon;
	Angle	_lat;
};


inline
LonLat::LonLat():
	LonLat (0_deg, 0_deg)
{ }


inline
LonLat::LonLat (Angle longitude, Angle latitude):
	_lon (longitude),
	_lat (latitude)
{ }


inline Angle&
LonLat::lon()
{
	return _lon;
}


inline Angle const&
LonLat::lon() const
{
	return _lon;
}


inline Angle&
LonLat::lat()
{
	return _lat;
}


inline Angle const&
LonLat::lat() const
{
	return _lat;
}


/**
 * Convenience function.
 * Compute distance between two sets of coordinates on Earth.
 */
inline Length
LonLat::haversine_earth (LonLat const& other) const
{
	return haversine (other) * EARTH_MEAN_RADIUS;
}

} // namespace SI

#endif

