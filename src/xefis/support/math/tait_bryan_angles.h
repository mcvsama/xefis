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
#include <xefis/support/math/euler_angles.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/math/placement.h>
#include <xefis/support/math/transforms.h>

// Standard:
#include <cstddef>


namespace xf {

struct TaitBryanAnglesParams
{
	si::Angle	pitch;
	si::Angle	roll;
	si::Angle	yaw;
};


/**
 * Pitch, roll, yaw angles from the NED frame.
 */
struct TaitBryanAngles: public SpaceVector<si::Angle>
{
	using SpaceVector<si::Angle>::SpaceVector;
	using SpaceVector<si::Angle>::operator=;

	// Ctor
	TaitBryanAngles (TaitBryanAnglesParams const& params):
		SpaceVector<si::Angle> (params.pitch, params.roll, params.yaw)
	{ }

	// Ctor
	explicit
	TaitBryanAngles (SpaceVector<si::Angle> const& other):
		SpaceVector<si::Angle> (other)
	{ }

	[[nodiscard]]
	constexpr auto
	pitch() const noexcept
	{
		return (*this)[0];
	}

	[[nodiscard]]
	constexpr auto
	roll() const noexcept
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
tait_bryan_angles (RotationMatrix<ECEFSpace, AirframeSpace> const& rotation, si::LonLat const& position)
{
	auto const diff = euler_angle_difference (RotationMatrix<> { ecef_to_ned_rotation (position).array() },
											  RotationMatrix<> { rotation.array() });

	return TaitBryanAngles (diff);
}


[[nodiscard]]
inline TaitBryanAngles
tait_bryan_angles (RotationMatrix<ECEFSpace, AirframeSpace> const& rotation, SpaceVector<si::Length, ECEFSpace> const& position)
{
	return tait_bryan_angles (rotation, polar (position));
}


[[nodiscard]]
inline TaitBryanAngles
tait_bryan_angles (Placement<ECEFSpace, AirframeSpace> const& pr)
{
	return tait_bryan_angles (pr.body_to_base_rotation(), polar (pr.position()));
}

} // namespace xf

#endif

