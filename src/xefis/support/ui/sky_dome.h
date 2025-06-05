/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__UI__SKY_DOME_H__INCLUDED
#define XEFIS__SUPPORT__UI__SKY_DOME_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/atmosphere/atmospheric_scattering.h>
#include <xefis/support/color/spaces.h>
#include <xefis/support/shapes/shape.h>
#include <xefis/support/universe/sun_position.h>

// Neutrino:
#include <neutrino/work_performer.h>

// Standard:
#include <cstddef>
#include <memory>
#include <future>


namespace xf {

struct SunPosition
{
	si::Angle						hour_angle;
	si::Angle						declination;
	HorizontalCoordinates			horizontal_coordinates;
	SpaceVector<double>				cartesian_horizontal_coordinates;
};


struct SkyDomeParameters
{
	AtmosphericScattering const&	atmospheric_scattering;
	si::LonLatRadius<>				observer_position;
	HorizontalCoordinates			sun_position;
	si::Length						earth_radius;
	std::shared_ptr<QOpenGLTexture>	earth_texture;
	float							sky_alpha			{ 1.0f };
	float							ground_haze_alpha	{ 0.5f };
};


[[nodiscard]]
SunPosition
calculate_sun_position (si::LonLat const observer_position, si::Time const);


[[nodiscard]]
constexpr SpaceVector<double>
calculate_cartesian_horizontal_coordinates (HorizontalCoordinates const& horizontal_coordinates)
{
	return to_cartesian<void> (si::LonLat (-horizontal_coordinates.azimuth + 180_deg, horizontal_coordinates.altitude));
}


[[nodiscard]]
SpaceVector<float, RGBSpace>
calculate_sun_light_color (si::LonLatRadius<> const observer_position, SpaceVector<double> const sun_position, AtmosphericScattering const&);


[[nodiscard]]
Shape
calculate_ground_shape (si::LonLatRadius<> const observer_position,
						si::Length const earth_radius,
						std::shared_ptr<QOpenGLTexture> earth_texture);


[[nodiscard]]
Shape
calculate_sky_dome_shape (SkyDomeParameters const& params, neutrino::WorkPerformer* work_performer = nullptr);

} // namespace xf

#endif

