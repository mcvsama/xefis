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

// Local:
#include "whip_antenna_model.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>

// Standard:
#include <cstddef>


namespace xf {

WhipAntennaModel::WhipAntennaModel (si::Length const antenna_length, double const frequency_response_sharpness):
	AntennaModel (1.64), // 1.64 for dipole-like antenna
	_antenna_length (antenna_length),
	_frequency_response_sharpness (frequency_response_sharpness)
{
	if (_frequency_response_sharpness < 0.0)
		throw nu::InvalidArgument ("whip antenna frequency_response_sharpness must be non-negative");
}


double
WhipAntennaModel::frequency_response (si::Frequency const frequency) const
{
	auto const wavelength = kSpeedOfLight / frequency;
	auto const resonant_length = wavelength / 4.0;
	auto const ratio = _antenna_length / resonant_length;
	auto const x = double (ratio - 1.0);
	return std::exp (-_frequency_response_sharpness * nu::square (x));
}


double
WhipAntennaModel::radiation_pattern (SpaceVector<double, BodyOrigin> direction) const
{
	auto const antenna_axis = SpaceVector<double, BodyOrigin> { 0.0, 0.0, 1.0 };
	auto const cosine = std::clamp (dot_product (antenna_axis, direction.normalized()), -1.0, 1.0);
	auto const pattern = 1.0 - nu::square (cosine);
	return std::max (0.0, pattern);
}


FieldPolarization
WhipAntennaModel::field_polarization (PolarizationBasis<BodyOrigin> const& basis, [[maybe_unused]] si::Frequency const frequency) const
{
	auto const antenna_axis = SpaceVector<double, BodyOrigin> { 0.0, 0.0, 1.0 };
	auto const transverse_field = normalized_direction_or_zero (
		antenna_axis - dot_product (antenna_axis, basis.propagation_direction) * basis.propagation_direction
	);

	return FieldPolarization {
		.u = dot_product (transverse_field, basis.transverse_u),
		.v = dot_product (transverse_field, basis.transverse_v),
	};
}

} // namespace xf
