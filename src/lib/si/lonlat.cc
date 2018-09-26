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

// Standard:
#include <cstddef>
#include <cmath>

// Qt:
#include <QtGui/QTransform>

// Local:
#include "utils.h"
#include "lonlat.h"


namespace si {

LonLat&
LonLat::rotate (LonLat const& rotation)
{
	// Convert to radians:
	Angle const& rot_lat = rotation.lat();
	Angle const& rot_lon = rotation.lon();
	// Get cartesian from polar coords:
	double const x = -cos (lat()) * cos (lon());
	double const y = +cos (lat()) * sin (lon());
	double const z = +sin (lat());
	// Lat rotation:
	double const sin_y = sin (-rot_lat);
	double const cos_y = cos (-rot_lat);
	// Lng rotation:
	double const sin_z = sin (rot_lon);
	double const cos_z = cos (rot_lon);

	QTransform rz = {
		+cos_z, -sin_z, 0.0,
		+sin_z, +cos_z, 0.0,
		   0.0,    0.0, 1.0
	};
	QTransform ry = {
		+cos_y, 0.0, +sin_y,
		   0.0, 1.0,    0.0,
		-sin_y, 0.0, +cos_y
	};
	QTransform r = ry * rz;

	// Matrix * vector:
	double const nx = x * r.m11() + y * r.m12() + z * r.m13();
	double const ny = x * r.m21() + y * r.m22() + z * r.m23();
	double const nz = x * r.m31() + y * r.m32() + z * r.m33();

	// Back to LonLat:
	double mag = std::sqrt (nx * nx + ny * ny);
	_lat = 1_rad * std::atan2 (nz, mag);
	_lon = 1_rad * std::atan2 (ny, nx);

	return *this;
}


LonLat
LonLat::rotated (LonLat const& rotation) const
{
	return LonLat (*this).rotate (rotation);
}


QPointF
LonLat::project_flat() const
{
	auto const tan_lon = tan (lon());
	auto const tan_lat = tan (lat());

	return {
		+tan_lon / (1.0 + tan_lon * tan (0.5 * lon())) * cos (lat()),
		-tan_lat / (1.0 + tan_lat * tan (0.5 * lat())),
	};
}

} // namespace si

