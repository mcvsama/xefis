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
	using std::sin;
	using std::cos;
	using std::atan2;

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
	double mag = sqrt (nx * nx + ny * ny);
	_lat = 1_rad * atan2 (nz, mag);
	_lon = 1_rad * atan2 (ny, nx);

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
	using std::tan;
	using std::cos;

	return {
		+tan (lon()) / (1.f + tan (lon()) * tan (0.5f * lon())) * cos (lat()),
		-tan (lat()) / (1.f + tan (lat()) * tan (0.5f * lat())),
	};
}


LonLat::ValueType::ValueType
LonLat::haversine (LonLat const& other) const
{
	using std::sin;
	using std::cos;
	using std::atan2;
	using std::sqrt;

	LonLat const& a = *this;
	LonLat const& b = other;

	Angle dlat = b.lat() - a.lat();
	Angle dlon = b.lon() - a.lon();

	ValueType::ValueType latsin = sin (dlat / 2.0);
	ValueType::ValueType lonsin = sin (dlon / 2.0);

	ValueType::ValueType z = latsin * latsin
						 + lonsin * lonsin
						 * cos (a.lat())
						 * cos (b.lat());

	return 2.0 * atan2 (sqrt (z), sqrt (1.0 - z));
}


Angle
LonLat::initial_bearing (LonLat const& other) const
{
	using std::sin;
	using std::cos;
	using std::atan2;

	Angle dlon = other.lon() - this->lon();
	Angle lat1 = this->lat();
	Angle lat2 = other.lat();

	double y = sin (dlon) * cos (lat2);
	double x = cos (lat1) * sin (lat2)
			 - sin (lat1) * cos (lat2) * cos (dlon);

	return 1_rad * atan2 (y, x);
}


Angle
LonLat::great_arcs_angle (LonLat const& a, LonLat const& common, LonLat const& b)
{
	using std::arg;

	LonLat z1 (a.lon() - common.lon(), a.lat() - common.lat());
	LonLat zero (0_deg, 0_deg);
	LonLat z2 (b.lon() - common.lon(), b.lat() - common.lat());

	std::complex<ValueType::ValueType> x1 (z1.lon().deg(), z1.lat().deg());
	std::complex<ValueType::ValueType> x2 (z2.lon().deg(), z2.lat().deg());

	return 1_deg * Xefis::floored_mod<double> ((1_rad * (arg (x1) - arg (x2))).deg(), 360.0);
}

} // namespace SI

