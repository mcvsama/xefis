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

#ifndef XEFIS__SUPPORT__SIMULATION__DEVICES__ANTENNA_WIDGET_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__DEVICES__ANTENNA_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/observation_widget.h>
#include <xefis/support/ui/radiation_plot.h>

// Qt:
#include <QTabWidget>

// Standard:
#include <cstddef>


namespace xf::sim {

class Antenna;


class AntennaWidget: public ObservationWidget
{
  public:
	// Ctor
	explicit
	AntennaWidget (Antenna&);

	// ObservationWidget API
	void
	update_observed_values (rigid_body::Body const* planet_body) override;

  private:
	Antenna&		_antenna;
	QTabWidget		_tabs;
	RadiationPlot	_x_axis_plot;
	RadiationPlot	_y_axis_plot;
	RadiationPlot	_z_axis_plot;
};

} // namespace xf::sim

#endif
