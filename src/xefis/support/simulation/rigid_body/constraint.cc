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
#include "constraint.h"

// Xefis:
#include <xefis/support/simulation/rigid_body/system.h>


namespace xf::rigid_body {

double
Constraint::baumgarte_factor() const noexcept
{
	if (_baumgarte_factor_override)
		return *_baumgarte_factor_override;
	else if (_system)
		return _system->default_baumgarte_factor();
	else
		return 0.0;
}

double
Constraint::constraint_force_mixing_factor() const noexcept
{
	if (_constraint_force_mixing_factor_override)
		return *_constraint_force_mixing_factor_override;
	else if (_system)
		return _system->default_constraint_force_mixing_factor();
	else
		return 0.0;
}

double
Constraint::friction_factor() const noexcept
{
	if (_friction_factor_override)
		return *_friction_factor_override;
	else if (_system)
		return _system->default_friction_factor();
	else
		return 0.0;
}

} // namespace xf::rigid_body
