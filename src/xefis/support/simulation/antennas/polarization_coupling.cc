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
#include "polarization_coupling.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/simulation/antennas/antenna_model.h>

// Standard:
#include <cstddef>


namespace xf {

double
polarization_coupling (FieldPolarization const& tx, FieldPolarization const& rx)
{
	auto const overlap = std::conj (rx.u) * tx.u + std::conj (rx.v) * tx.v;
	return std::norm (overlap);
}


[[nodiscard]]
double
polarization_coupling (AntennaModel const& tx_antenna_model,
					   Placement<WorldSpace, BodyOrigin> const& tx_antenna_placement,
					   AntennaModel const& rx_antenna_model,
					   Placement<WorldSpace, BodyOrigin> const& rx_antenna_placement,
					   si::Frequency const frequency,
					   SpaceVector<double, WorldSpace> const& tx_to_rx_direction)
{
	auto transverse_u_in_world = SpaceVector<double, WorldSpace> { 1.0, 0.0, 0.0 };
	auto transverse_v_in_world = SpaceVector<double, WorldSpace> { 0.0, 1.0, 0.0 };

	if (abs (tx_to_rx_direction) > 1e-12)
	{
		// Shouldn't u/v be somehow related to antenna orientation?
		transverse_u_in_world = normalized_direction_or_zero (arbitrary_orthogonal (tx_to_rx_direction));
		transverse_v_in_world = normalized_direction_or_zero (cross_product (tx_to_rx_direction, transverse_u_in_world));
	}

	auto const tx_polarization_basis = PolarizationBasis<BodyOrigin> {
		.propagation_direction = tx_antenna_placement.rotate_to_body (+tx_to_rx_direction),
		.transverse_u = tx_antenna_placement.rotate_to_body (transverse_u_in_world),
		.transverse_v = tx_antenna_placement.rotate_to_body (transverse_v_in_world),
	};
	auto const rx_polarization_basis = PolarizationBasis<BodyOrigin> {
		.propagation_direction = rx_antenna_placement.rotate_to_body (-tx_to_rx_direction),
		.transverse_u = rx_antenna_placement.rotate_to_body (transverse_u_in_world),
		.transverse_v = rx_antenna_placement.rotate_to_body (transverse_v_in_world),
	};
	auto const tx_polarization_state = tx_antenna_model.field_polarization (tx_polarization_basis, frequency);
	auto const rx_polarization_state = rx_antenna_model.field_polarization (rx_polarization_basis, frequency);

	return polarization_coupling (tx_polarization_state, rx_polarization_state);
}

} // namespace xf
