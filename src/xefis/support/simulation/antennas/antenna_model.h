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

#ifndef XEFIS__SUPPORT__SIMULATION__ANTENNAS__ANTENNA_MODEL_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__ANTENNAS__ANTENNA_MODEL_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/coordinate_systems.h>
#include <xefis/support/nature/constants.h>
#include <xefis/support/simulation/antennas/polarization_coupling.h>

// Neutrino:
#include <neutrino/noncopyable.h>
#include <neutrino/nonmovable.h>
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>


namespace xf {

class AntennaModel:
	public nu::Noncopyable,
	public nu::Nonmovable
{
  public:
	/**
	 * \param	peak_directional_gain
	 *			Maximum gain at ideal frequency and ideal direction.
	 */
	explicit
	AntennaModel (double const peak_directional_gain):
		_peak_directional_gain (peak_directional_gain)
	{ }

	// Dtor
	virtual
	~AntennaModel() = default;

	/**
	 * Return absolute antenna gain for given frequency and direction.
	 *
	 * Combines peak gain with normalized directional and frequency terms:
	 * `peak_directional_gain * radiation_pattern (direction) * frequency_response (frequency)`.
	 *
	 * \param	frequency
	 *			Signal frequency.
	 * \param	direction
	 *			Transmit-style direction in body coordinates (antenna → far-field point).
	 * \return	Gain factor for Friis-style power calculations.
	 */
	[[nodiscard]]
	double
	gain (si::Frequency const frequency, SpaceVector<double, BodyOrigin> direction) const
		{ return _peak_directional_gain * radiation_pattern (direction) * frequency_response (frequency); }

	/**
	 * Return normalized directional factor, in range [0, 1].
	 * Uses transmit-style direction vector (antenna → far-field point).
	 */
	[[nodiscard]]
	virtual double
	radiation_pattern (SpaceVector<double, BodyOrigin> direction) const = 0;

	/**
	 * Return normalized frequency response factor, in range [0, 1].
	 */
	[[nodiscard]]
	virtual double
	frequency_response (si::Frequency const frequency) const = 0;

	/**
	 * Return electric-field polarization state for given wave basis and frequency.
	 *
	 * `basis` must be expressed in antenna body coordinates.
	 */
	[[nodiscard]]
	virtual FieldPolarization
	field_polarization (PolarizationBasis<BodyOrigin> const& basis, si::Frequency frequency) const = 0;

  private:
	double const _peak_directional_gain;
};

} // namespace xf

#endif
