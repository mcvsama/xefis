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

#ifndef XEFIS__SUPPORT__SIMULATION__DEVICES__WING_WIDGET_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__DEVICES__WING_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/airfoil_spline_widget.h>
#include <xefis/support/ui/observation_widget.h>

// Qt:
#include <QLabel>
#include <QWidget>

// Standard:
#include <cstddef>
#include <string>


namespace xf::sim {

class Wing;

class WingWidget: public ObservationWidget
{
  public:
	// Ctor
	explicit
	WingWidget (Wing& wing);

	// ObservationWidget API
	void
	update_observed_values() override;

  private:
	void
	setup_airfoil_info_widget();

  private:
	Wing&				_wing;
	QFrame				_airfoil_frame;
	AirfoilSplineWidget	_airfoil_spline_widget;
	std::string			_true_air_speed;
	std::string			_static_air_temperature;
	std::string			_air_density;
	std::string			_dynamic_viscosity;
	std::string			_reynolds_number;
};

} // namespace xf::sim

#endif

