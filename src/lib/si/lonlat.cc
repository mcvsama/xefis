/* vim:ts=4
 *
 * Copyleft 2012…2015  Michał Gawron
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
#include <xefis/utility/numeric.h>

// Local:
#include "lonlat.h"


namespace SI {

LonLat&
LonLat::rotate (LonLat const& rotation)
{
	// Convert to radians:
	Angle const& rot_lat = rotation.lat();
	Angle const& rot_lon = rotation.lon();
	// Get cartesian from polar coords:
	double const x = -std::cos (lat()) * std::cos (lon());
	double const y = +std::cos (lat()) * std::sin (lon());
	double const z = +std::sin (lat());
	// Lat rotation:
	double const sin_y = std::sin (-rot_lat);
	double const cos_y = std::cos (-rot_lat);
	// Lng rotation:
	double const sin_z = std::sin (rot_lon);
	double const cos_z = std::cos (rot_lon);

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
	return {
		+std::tan (lon()) / (1.f + std::tan (lon()) * std::tan (0.5f * lon())) * std::cos (lat()),
		-std::tan (lat()) / (1.f + std::tan (lat()) * std::tan (0.5f * lat())),
	};
}


LonLat::ValueType::ValueType
LonLat::haversine (LonLat const& other) const
{
	LonLat const& a = *this;
	LonLat const& b = other;

	Angle dlat = b.lat() - a.lat();
	Angle dlon = b.lon() - a.lon();

	ValueType::ValueType latsin = std::sin (dlat / 2.0);
	ValueType::ValueType lonsin = std::sin (dlon / 2.0);

	ValueType::ValueType z = latsin * latsin
						 + lonsin * lonsin
						 * std::cos (a.lat())
						 * std::cos (b.lat());

	return 2.0 * std::atan2 (std::sqrt (z), std::sqrt (1.0 - z));
}


Angle
LonLat::initial_bearing (LonLat const& other) const
{
	Angle dlon = other.lon() - this->lon();
	Angle lat1 = this->lat();
	Angle lat2 = other.lat();

	double y = std::sin (dlon) * std::cos (lat2);
	double x = std::cos (lat1) * std::sin (lat2)
			 - std::sin (lat1) * std::cos (lat2) * std::cos (dlon);

	return 1_rad * std::atan2 (y, x);
}


Angle
LonLat::great_arcs_angle (LonLat const& a, LonLat const& common, LonLat const& b)
{
	LonLat z1 (a.lon() - common.lon(), a.lat() - common.lat());
	LonLat zero (0_deg, 0_deg);
	LonLat z2 (b.lon() - common.lon(), b.lat() - common.lat());

	std::complex<ValueType::ValueType> x1 (z1.lon().deg(), z1.lat().deg());
	std::complex<ValueType::ValueType> x2 (z2.lon().deg(), z2.lat().deg());

	return 1_deg * Xefis::floored_mod<double> ((1_rad * (std::arg (x1) - std::arg (x2))).deg(), 360.0);
}

} // namespace SI

