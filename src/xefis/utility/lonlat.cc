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

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QTransform>

// Xefis:
#include <xefis/config/all.h>
// XXX needed for conversions:
#include <xefis/utility/numeric.h>

// Local:
#include "lonlat.h"


LonLat&
LonLat::rotate (LonLat const& rotation)
{
	// Convert to radians:
	float const lat_rad = deg_to_rad (lat());
	float const lon_rad = deg_to_rad (lon());
	float const rot_lat_rad = deg_to_rad (rotation.lat());
	float const rot_lon_rad = deg_to_rad (rotation.lon());
	// Get cartesian from polar coords:
	float const x = -std::cos (lat_rad) * std::cos (lon_rad);
	float const y = +std::cos (lat_rad) * std::sin (lon_rad);
	float const z = +std::sin (lat_rad);
	// Lat rotation:
	float const sin_y = std::sin (-rot_lat_rad);
	float const cos_y = std::cos (-rot_lat_rad);
	// Lng rotation:
	float const sin_z = std::sin (rot_lon_rad);
	float const cos_z = std::cos (rot_lon_rad);

	QTransform rz = {
		+cos_z, -sin_z, 0.f,
		+sin_z, +cos_z, 0.f,
		   0.f,    0.f, 1.f
	};
	QTransform ry = {
		+cos_y, 0.f, +sin_y,
		   0.f, 1.f,    0.f,
		-sin_y, 0.f, +cos_y
	};
	QTransform r = ry * rz;

	// Matrix * vector:
	float const nx = x * r.m11() + y * r.m12() + z * r.m13();
	float const ny = x * r.m21() + y * r.m22() + z * r.m23();
	float const nz = x * r.m31() + y * r.m32() + z * r.m33();

	// Back to LonLat:
	float mag = std::sqrt (nx * nx + ny * ny);
	_lat = rad_to_deg (std::atan2 (nz, mag));
	_lon = rad_to_deg (std::atan2 (ny, nx));

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
	float const lat_rad = deg_to_rad (lat());
	float const lon_rad = deg_to_rad (lon());

	return {
		+std::tan (lon_rad) / (1.f + std::tan (lon_rad) * std::tan (0.5f * lon_rad)) * std::cos (lat_rad),
		-std::tan (lat_rad) / (1.f + std::tan (lat_rad) * std::tan (0.5f * lat_rad)),
	};
}


LonLat::ValueType
LonLat::haversine (LonLat const& other) const
{
	LonLat const& a = *this;
	LonLat const& b = other;

	ValueType dlat = deg_to_rad (b.lat() - a.lat());
	ValueType dlon = deg_to_rad (b.lon() - a.lon());

	ValueType latsin = std::sin (dlat / 2.0);
	ValueType lonsin = std::sin (dlon / 2.0);

	ValueType z = latsin * latsin
				+ lonsin * lonsin
				* std::cos (deg_to_rad (a.lat()))
				* std::cos (deg_to_rad (b.lat()));

	return 2.0 * std::atan2 (std::sqrt (z), std::sqrt (1.0 - z));
}

