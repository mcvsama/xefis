/* vim:ts=4
 *
 * Copyleft 2008…2018  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__ATMOSPHERE_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__ATMOSPHERE_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/earth/air.h>
#include <xefis/support/math/space.h>


namespace xf::sim {

/**
 * Atmosphere state at some given position.
 */
template<class Frame>
	struct AtmosphereState
	{
		Air									air;
		SpaceVector<si::Velocity, Frame>	wind;
	};


/**
 * General atmosphwere model.
 * TODO use Perlin noise for winds. http://flafla2.github.io/2014/08/09/perlinnoise.html
 * TODO use winds to affect density/pressure at that point.
 * TODO maybe the winds model should be a separate class plugged in into Atmosphere object?
 */
class Atmosphere
{
  public:
	[[nodiscard]]
	Air
	air_at (SpaceVector<si::Length, ECEFFrame> const& position) const;

	[[nodiscard]]
	Air
	air_at_radius (si::Length radius) const;

	[[nodiscard]]
	Air
	air_at_amsl (si::Length amsl_height) const;

	[[nodiscard]]
	SpaceVector<si::Velocity, ECEFFrame>
	wind_at (SpaceVector<si::Length, ECEFFrame> const& position) const;

	[[nodiscard]]
	AtmosphereState<ECEFFrame>
	state_at (SpaceVector<si::Length, ECEFFrame> const& position) const;
};

} // namespace xf::sim

#endif

