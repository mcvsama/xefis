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

#ifndef XEFIS__SUPPORT__SIMULATION__ANTENNAS__WHIP_ANTENNA_MODEL_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__ANTENNAS__WHIP_ANTENNA_MODEL_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/coordinate_systems.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/simulation/antennas/antenna_model.h>

// Neutrino:
#include <neutrino/noncopyable.h>
#include <neutrino/nonmovable.h>
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>


namespace xf {

class WhipAntennaModel: public AntennaModel
{
  public:
	/**
	 * \param	antenna_length
	 *			Physical whip length.
	 * \param	frequency_response_sharpness
	 *			Non-negative Gaussian sharpness coefficient used by frequency_response().
	 *			0 means equal response for all frequencies.  Typical value for whip antenna is 25 (default). Practical range: 15-40.
	 */
	explicit
	WhipAntennaModel (si::Length const antenna_length, double frequency_response_sharpness = 25.0);

	[[nodiscard]]
	double
	frequency_response (si::Frequency const frequency) const override;

	[[nodiscard]]
	double
	radiation_pattern (SpaceVector<double, BodyOrigin> direction) const override;

	[[nodiscard]]
	FieldPolarization
	field_polarization (PolarizationBasis<BodyOrigin> const& basis, si::Frequency frequency) const override;

  private:
	si::Length	_antenna_length;
	double		_frequency_response_sharpness;
};

} // namespace xf

#endif
