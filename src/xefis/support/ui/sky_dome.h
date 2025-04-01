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
#include <xefis/support/color/spaces.h>
#include <xefis/support/earth/air/atmospheric_scattering.h>
#include <xefis/support/simulation/rigid_body/shape.h>
#include <xefis/support/universe/sun_position.h>

// Neutrino:
#include <neutrino/work_performer.h>

// Standard:
#include <cstddef>
#include <memory>
#include <future>


namespace xf {

/**
 * Shapes for sky and ground haze. Also sun stuff useful for some world drawing.
 */
class SkyDome
{
  public:
	struct SunPosition
	{
		si::Angle				hour_angle;
		si::Angle				declination;
		HorizontalCoordinates	horizontal_coordinates;
		SpaceVector<double>		cartesian_coordinates;
	};

  public:
	rigid_body::Shape				ground_shape;
	rigid_body::Shape				atmospheric_dome_shape;
	SunPosition						sun_position;
	SpaceVector<float, RGBSpace>	sun_light_color;
};


struct SkyDomeParameters
{
	AtmosphericScattering const&	atmospheric_scattering;
	si::LonLatRadius<>				observer_position;
	si::Length						earth_radius;
	si::Time						unix_time;
	float							ground_haze_alpha	{ 0.5f };
};


SkyDome
calculate_sky_dome (SkyDomeParameters const& params, neutrino::WorkPerformer* work_performer = nullptr);

} // namespace xf

#endif

