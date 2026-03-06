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
#include "has_observation_widget.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/group.h>
#include <xefis/support/simulation/rigid_body/body.h>
#include <xefis/support/simulation/rigid_body/constraint.h>

// Standard:
#include <cstddef>


namespace xf {

std::unique_ptr<ObservationWidget>
HasObservationWidget::create_observation_widget()
{
	if (rigid_body::Group* group = dynamic_cast<rigid_body::Group*> (this))
		return std::make_unique<ObservationWidget> (group);
	else if (rigid_body::Body* body = dynamic_cast<rigid_body::Body*> (this))
		return std::make_unique<ObservationWidget> (body);
	else if (rigid_body::Constraint* constraint = dynamic_cast<rigid_body::Constraint*> (this))
		return std::make_unique<ObservationWidget> (constraint);
	else
		return std::make_unique<ObservationWidget>();
}

} // namespace xf
