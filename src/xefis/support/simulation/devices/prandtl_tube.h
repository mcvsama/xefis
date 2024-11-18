/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__DEVICES__PRANDTL_TUBE_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__DEVICES__PRANDTL_TUBE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/earth/air/atmosphere.h>
#include <xefis/support/simulation/rigid_body/body.h>
#include <xefis/support/ui/observation_widget.h>

// Standard:
#include <cstddef>


namespace xf::sim {

struct PrandtlTubeParameters
{
	si::Mass	mass;
	si::Length	length;
	si::Length	diameter;
};


/**
 * Also called Pitot-static tube, measures both dynamic and static pressures.
 * The body's X-axis points into the wind (X-axis is sensor's surface normal vector).
 */
class PrandtlTube:
	public rigid_body::Body,
	public HasObservationWidget
{
  public:
	/**
	 * \param	Atmosphere
	 *			PrandtlTube can't outlive the Atmosphere.
	 */
	explicit
	PrandtlTube (Atmosphere const&, PrandtlTubeParameters const&);

	[[nodiscard]]
	si::Pressure
	static_pressure() const;

	[[nodiscard]]
	si::Pressure
	total_pressure() const;

  private:
	Atmosphere const* _atmosphere;
};

} // namespace xf::sim

#endif

