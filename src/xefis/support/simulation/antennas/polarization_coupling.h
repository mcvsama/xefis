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

#ifndef XEFIS__SUPPORT__SIMULATION__ANTENNAS__POLARIZATION_COUPLING_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__ANTENNAS__POLARIZATION_COUPLING_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/placement.h>

// Neutrino:
#include <neutrino/numeric.h>

// Standard:
#include <cstddef>
#include <complex>


namespace xf {

class AntennaModel;


/**
 * Orthonormal basis for an electromagnetic wave.
 *
 * `transverse_u` and `transverse_v` are both perpendicular to
 * `propagation_direction` and to each other.
 */
template<math::CoordinateSystem Space = void>
	struct PolarizationBasis
	{
		// Unit vector in the wave propagation direction.
		SpaceVector<double, Space> propagation_direction;

		// First unit vector of the transverse polarization plane.
		SpaceVector<double, Space> transverse_u;

		// Second unit vector of the transverse polarization plane.
		SpaceVector<double, Space> transverse_v;
	};


/**
 * Complex polarization amplitudes in the `{u, v}` transverse basis.
 *
 * Relative phase between `u` and `v` encodes linear/circular/elliptical
 * polarization.
 *
 * `FieldPolarization` does not store basis orientation. It must therefore be
 * interpreted together with the corresponding polarization basis.
 */
struct FieldPolarization
{
	// Complex field amplitude along basis axis `u`.
	std::complex<double> u;

	// Complex field amplitude along basis axis `v`.
	std::complex<double> v;
};


/**
 * Return polarization power coupling between transmitter and receiver states.
 *
 * `tx` and `rx` must be expressed in the same transverse `{u, v}` basis.
 * If bases differ, use the overload that takes explicit TX/RX bases.
 *
 * This is the squared magnitude of the complex inner product:
 * `|conj(rx.u) * tx.u + conj(rx.v) * tx.v|^2`.
 *
 * For normalized `tx` and `rx`, result is in range `[0, 1]`.
 */
[[nodiscard]]
double
polarization_coupling (FieldPolarization const& tx, FieldPolarization const& rx);


/**
 * Return polarization power coupling when TX and RX states are defined in
 * possibly different transverse bases, but in the same coordinate space.
 */
template<math::CoordinateSystem Space>
	[[nodiscard]]
	inline double
	polarization_coupling (FieldPolarization const& tx,
						   PolarizationBasis<Space> const& tx_basis,
						   FieldPolarization const& rx,
						   PolarizationBasis<Space> const& rx_basis)
	{
		auto const uu = dot_product (rx_basis.transverse_u, tx_basis.transverse_u);
		auto const uv = dot_product (rx_basis.transverse_u, tx_basis.transverse_v);
		auto const vu = dot_product (rx_basis.transverse_v, tx_basis.transverse_u);
		auto const vv = dot_product (rx_basis.transverse_v, tx_basis.transverse_v);
		auto const overlap = std::conj (rx.u) * (uu * tx.u + uv * tx.v) +
							 std::conj (rx.v) * (vu * tx.u + vv * tx.v);

		return std::norm (overlap);
	}


/**
 * Return polarization power coupling when TX and RX use different local spaces.
 *
 * Polarization bases are first transformed to `BaseSpace` with placements.
 */
template<math::CoordinateSystem BaseSpace, math::CoordinateSystem TxSpace, math::CoordinateSystem RxSpace>
	[[nodiscard]]
	inline double
	polarization_coupling (FieldPolarization const& tx,
						   PolarizationBasis<TxSpace> const& tx_basis,
						   Placement<BaseSpace, TxSpace> const& tx_placement,
						   FieldPolarization const& rx,
						   PolarizationBasis<RxSpace> const& rx_basis,
						   Placement<BaseSpace, RxSpace> const& rx_placement)
	{
		auto const tx_basis_in_base = PolarizationBasis<BaseSpace> {
			.propagation_direction = tx_placement.rotate_to_base (tx_basis.propagation_direction),
			.transverse_u = tx_placement.rotate_to_base (tx_basis.transverse_u),
			.transverse_v = tx_placement.rotate_to_base (tx_basis.transverse_v),
		};
		auto const rx_basis_in_base = PolarizationBasis<BaseSpace> {
			.propagation_direction = rx_placement.rotate_to_base (rx_basis.propagation_direction),
			.transverse_u = rx_placement.rotate_to_base (rx_basis.transverse_u),
			.transverse_v = rx_placement.rotate_to_base (rx_basis.transverse_v),
		};

		return polarization_coupling (tx, tx_basis_in_base, rx, rx_basis_in_base);
	}


/**
 * Return polarization power coupling for two antenna models and placements.
 *
 * Polarization bases are built in world space from `tx_to_rx_direction` and a
 * derived orthonormal transverse pair, then rotated to each antenna body
 * frame. Each antenna model computes local field polarization for the given
 * frequency, and the resulting states are compared with `polarization_coupling
 * (FieldPolarization const&, FieldPolarization const&)`.
 *
 * If `tx_to_rx_direction` has near-zero length, canonical world axes are used
 * as fallback transverse vectors.
 *
 * \param	tx_antenna_model
 *			Transmitter antenna model used to compute TX polarization state.
 * \param	tx_antenna_placement
 *			Transmitter antenna placement at the moment of signal emission.
 * \param	rx_antenna_model
 *			Receiver antenna model used to compute RX polarization state.
 * \param	rx_antenna_placement
 *			Receiver antenna placement at the moment of signal reception.
 * \param	frequency
 *			Signal frequency passed to both antenna models.
 * \param	tx_to_rx_direction
 *			Normalized direction vector from TX to RX antenna.
 * \returns	Polarization power coupling factor.
 */
[[nodiscard]]
double
polarization_coupling (AntennaModel const& tx_antenna_model,
					   Placement<WorldSpace, BodyOrigin> const& tx_antenna_placement,
					   AntennaModel const& rx_antenna_model,
					   Placement<WorldSpace, BodyOrigin> const& rx_antenna_placement,
					   si::Frequency frequency,
					   SpaceVector<double, WorldSpace> const& tx_to_rx_direction);

} // namespace xf

#endif
