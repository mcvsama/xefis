/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__DEVICES__WHIP_ANTENNA_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__DEVICES__WHIP_ANTENNA_H__INCLUDED

// Local:
#include "antenna.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/antennas/whip_antenna_model.h>

// Standard:
#include <cstddef>


namespace xf::sim {

struct WhipAntennaParameters
{
	si::Mass		mass							{ 50_g };
	si::Length		antenna_length					{ 220_mm }; // Electrical (internal) antenna length used by the RF model.
	si::Length		body_length						{ 220_mm }; // Physical whip shape length.
	si::Length		base_length						{ 40_mm };
	si::Length		shaft_diameter					{ 8_mm };
	si::Length		base_diameter					{ 18_mm };
	std::size_t		num_faces						{ 24 };
	double			frequency_response_sharpness	{ 25.0 };
};


class WhipAntennaBase
{
  public:
	// Ctor
	explicit
	WhipAntennaBase (WhipAntennaParameters const& params):
		_antenna_model (params.antenna_length, params.frequency_response_sharpness)
	{ }

  protected:
	[[nodiscard]]
	WhipAntennaParameters const&
	params() const noexcept
		{ return _params; }

	[[nodiscard]]
	xf::WhipAntennaModel const&
	antenna_model() const noexcept
		{ return _antenna_model; }

  private:
	WhipAntennaParameters	_params;
	xf::WhipAntennaModel	_antenna_model;
};


class WhipAntenna:
	private WhipAntennaBase,
	public Antenna
{
  public:
	// Ctor
	explicit
	WhipAntenna (xf::AntennaSystem&,
				 WhipAntennaParameters const& = {},
				 xf::Antenna::SignalReceptionCallback = {});

  private:
	[[nodiscard]]
	static MassMomentsAtArm<BodyCOM>
	compute_body_com_mass_moments (WhipAntennaParameters const&);

	[[nodiscard]]
	static Shape
	make_shape (WhipAntennaParameters const&);
};

} // namespace xf::sim

#endif
