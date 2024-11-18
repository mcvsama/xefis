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

#ifndef XEFIS__SUPPORT__SIMULATION__DEVICES__WING_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__DEVICES__WING_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/aerodynamics/airfoil.h>
#include <xefis/support/aerodynamics/airfoil_aerodynamic_parameters.h>
#include <xefis/support/simulation/devices/wing_widget.h>
#include <xefis/support/simulation/rigid_body/body.h>
#include <xefis/support/simulation/rigid_body/concepts.h>

// Standard:
#include <cstddef>


namespace xf::sim {

class Wing:
	public rigid_body::Body,
	public HasObservationWidget
{
  public:
	// Ctor
	explicit
	Wing (Airfoil const&, si::Density const material_density);

	/**
	 * Reference to the internal Airfoil object.
	 */
	[[nodiscard]]
	Airfoil const&
	airfoil() const noexcept
		{ return _airfoil; }

	[[nodiscard]]
	std::optional<AirfoilAerodynamicParameters<rigid_body::BodyCOM>> const&
	airfoil_aerodynamic_parameters() const noexcept
		{ return _airfoil_aerodynamic_parameters; }

	// Body API
	void
	update_external_forces (Atmosphere const*) override;

	// HasObservationWidget API
	[[nodiscard]]
	std::unique_ptr<ObservationWidget>
	create_body_widget()
		{ return std::make_unique<WingWidget> (*this); }

  private:
	[[nodiscard]]
	static MassMoments<rigid_body::BodyCOM>
	calculate_body_com_mass_moments (Airfoil const&, si::Density material_density);

  private:
	Airfoil																_airfoil;
	std::optional<AirfoilAerodynamicParameters<rigid_body::BodyCOM>>	_airfoil_aerodynamic_parameters;
};

} // namespace xf::sim

#endif

