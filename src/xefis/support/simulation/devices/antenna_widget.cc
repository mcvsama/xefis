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
#include "antenna_widget.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/devices/antenna.h>
#include <xefis/support/ui/paint_helper.h>

// Standard:
#include <cstddef>


namespace xf::sim {

AntennaWidget::AntennaWidget (Antenna& antenna):
	ObservationWidget (&antenna),
	_antenna (antenna)
{
	auto const ph = PaintHelper (*this);
	auto const min_side = ph.em_pixels (16.0);

	for (auto* plot: { &_x_axis_plot, &_y_axis_plot, &_z_axis_plot })
	{
		plot->setSizePolicy (QSizePolicy::Ignored, QSizePolicy::Ignored);
		plot->setMinimumSize (min_side, min_side);
		plot->set_antenna_model (_antenna.model());
	}

	_x_axis_plot.set_axis_vector ({ 1.0, 0.0, 0.0 });
	_y_axis_plot.set_axis_vector ({ 0.0, 1.0, 0.0 });
	_z_axis_plot.set_axis_vector ({ 0.0, 0.0, 1.0 });

	_tabs.setContentsMargins (0, 0, 0, 0);
	_tabs.addTab (&_x_axis_plot, "X axis");
	_tabs.addTab (&_y_axis_plot, "Y axis");
	_tabs.addTab (&_z_axis_plot, "Z axis");
	_tabs.setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	add_widget (_tabs);
}


void
AntennaWidget::update_observed_values (rigid_body::Body const* planet_body)
{
	ObservationWidget::update_observed_values (planet_body);
}

} // namespace xf::sim
