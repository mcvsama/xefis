/* vim:ts=4
 *
 * Copyleft 2012…2018  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__MATH__TAIT_BRYAN_ANGLES_H__INCLUDED
#define XEFIS__SUPPORT__MATH__TAIT_BRYAN_ANGLES_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/math/placement.h>
#include <xefis/support/math/transforms.h>

// Standard:
#include <cstddef>


namespace xf {

// Using Airplane-coordinates.
struct TaitBryanAnglesParams
{
	si::Angle	roll;	// About X in aircraft coordinates (forward)
	si::Angle	pitch;	// About Y in aircraft coordinates (right wing)
	si::Angle	yaw;	// About Z (down)
};


/**
 * Roll, pitch, yaw angles from the NED frame.
 */
struct TaitBryanAngles: public SpaceVector<si::Angle>
{
	using SpaceVector<si::Angle>::SpaceVector;
	using SpaceVector<si::Angle>::operator=;

	// Ctor
	TaitBryanAngles (TaitBryanAnglesParams const& params):
		SpaceVector<si::Angle> (params.roll, params.pitch, params.yaw)
	{ }

	// Ctor
	explicit
	TaitBryanAngles (SpaceVector<si::Angle> const& other):
		SpaceVector<si::Angle> (other)
	{ }

	[[nodiscard]]
	constexpr auto
	roll() const noexcept
	{
		return (*this)[0];
	}

	[[nodiscard]]
	constexpr auto
	pitch() const noexcept
	{
		return (*this)[1];
	}

	[[nodiscard]]
	constexpr auto
	yaw() const noexcept
	{
		return (*this)[2];
	}
};


[[nodiscard]]
inline TaitBryanAngles
tait_bryan_angles (RotationQuaternion<ECEFSpace, AirframeSpace> const& body_rotation, si::LonLat const& position)
{
	auto const q_relative = ecef_to_ned_rotation (position) * body_rotation;
	auto const w = q_relative.w();
	auto const x = q_relative.x();
	auto const y = q_relative.y();
	auto const z = q_relative.z();
	auto const yy = nu::square (y);

	return TaitBryanAngles ({
		.roll = 1_rad * std::atan2 (2.0 * (w * x + y * z), 1.0 - 2.0 * (x * x + yy)),
		.pitch = 1_rad * std::asin (std::clamp (2.0 * (w * y - z * x), -1.0, +1.0)),
		.yaw = 1_rad * std::atan2 (2.0 * (w * z + x * y), 1.0 - 2.0 * (yy + z * z)),
	});
}


[[nodiscard]]
inline TaitBryanAngles
tait_bryan_angles (RotationMatrix<ECEFSpace, AirframeSpace> const& body_coordinates, SpaceLength<ECEFSpace> const& position)
{
	return tait_bryan_angles (RotationQuaternion (body_coordinates), to_polar (position));
}


[[nodiscard]]
inline TaitBryanAngles
tait_bryan_angles (RotationQuaternion<ECEFSpace, AirframeSpace> const& body_rotation, SpaceLength<ECEFSpace> const& position)
{
	return tait_bryan_angles (body_rotation, to_polar (position));
}


[[nodiscard]]
inline TaitBryanAngles
tait_bryan_angles (Placement<ECEFSpace, AirframeSpace> const& placement)
{
	return tait_bryan_angles (placement.body_rotation(), to_polar (placement.position()));
}

} // namespace xf

#endif

