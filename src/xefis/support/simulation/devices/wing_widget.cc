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

// Local:
#include "wing_widget.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/devices/wing.h>
#include <xefis/support/ui/paint_helper.h>

// Qt:
#include <QGridLayout>

// Standard:
#include <cstddef>


namespace xf::sim {

WingWidget::WingWidget (Wing& wing):
	ObservationWidget (&wing),
	_wing (wing)
{
	setup_airfoil_info_widget();
	add_widget (_airfoil_frame);
	add_observable ("True air speed", _true_air_speed);
	add_observable ("Static air temperature", _static_air_temperature);
	add_observable ("Air density", _air_density);
	add_observable ("Dynamic viscosity:", _dynamic_viscosity);
	add_observable ("Reynolds number:", _reynolds_number);
}


void
WingWidget::setup_airfoil_info_widget()
{
	auto const ph = PaintHelper (*this);

	_airfoil_spline_widget.setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	_airfoil_spline_widget.setMinimumSize (ph.em_pixels (20), ph.em_pixels (10)); // TODO

	_airfoil_frame.setFrameStyle (QFrame::StyledPanel | QFrame::Sunken);

	auto* airfoil_frame_layout = new QVBoxLayout (&_airfoil_frame);
	airfoil_frame_layout->addWidget (&_airfoil_spline_widget);
	airfoil_frame_layout->setMargin (0);
}


void
WingWidget::update_observed_values()
{
	_airfoil_spline_widget.set_airfoil (_wing.airfoil());

	if (auto const aerodynamic_parameters = _wing.airfoil_aerodynamic_parameters())
	{
		auto const& air = aerodynamic_parameters->air;

		_true_air_speed = std::format ("{:.3f}", aerodynamic_parameters->true_air_speed);
		_static_air_temperature = std::format ("{:.1f}", air.temperature.to<si::Celsius>());
		_air_density = std::format ("{:.3f}", air.density);
		_dynamic_viscosity = std::format ("{:.4g}", air.dynamic_viscosity);
		_reynolds_number = std::format ("{:.0f}", *aerodynamic_parameters->reynolds_number);

		// Airfoil spline widget:
		{
			auto const xy = []<class Value, class Space> (SpaceVector<Value, Space> const& vector3) {
				return PlaneVector<Value, Space> (vector3.x(), vector3.y());
			};

			auto const& forces = aerodynamic_parameters->forces;
			auto const center_of_pressure_3d = forces.center_of_pressure / _wing.airfoil().chord_length();

			_airfoil_spline_widget.set_center_of_pressure_position (xy (center_of_pressure_3d));
			_airfoil_spline_widget.set_lift_force (xy (forces.lift));
			_airfoil_spline_widget.set_drag_force (xy (forces.drag));
			_airfoil_spline_widget.set_pitching_moment (xy (forces.pitching_moment));
		}
	}
	else
	{
		for (auto* str: { &_true_air_speed, &_static_air_temperature, &_air_density, &_dynamic_viscosity, &_reynolds_number })
			*str = "–";
	}

	ObservationWidget::update_observed_values();
}

} // namespace xf::sim

