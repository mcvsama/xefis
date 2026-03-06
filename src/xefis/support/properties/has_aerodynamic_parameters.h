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

#ifndef XEFIS__SUPPORT__PROPERTIES__HAS_AERODYNAMIC_PARAMETERS_H__INCLUDED
#define XEFIS__SUPPORT__PROPERTIES__HAS_AERODYNAMIC_PARAMETERS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/aerodynamics/aerodynamic_parameters.h>

// Standard:
#include <cstddef>
#include <optional>


namespace xf {

class HasAerodynamicParameters
{
  public:
	[[nodiscard]]
	std::optional<AerodynamicParameters<BodyCOM>> const&
	aerodynamic_parameters() const noexcept
		{ return _aerodynamic_parameters; }

  protected:
	void
	set_aerodynamic_parameters (AerodynamicParameters<BodyCOM> const parameters)
		{ _aerodynamic_parameters = parameters; }

	void
	set_aerodynamic_parameters (std::optional<AerodynamicParameters<BodyCOM>> const parameters)
		{ _aerodynamic_parameters = parameters; }

  private:
	std::optional<AerodynamicParameters<BodyCOM>> _aerodynamic_parameters;
};

} // namespace xf

#endif
