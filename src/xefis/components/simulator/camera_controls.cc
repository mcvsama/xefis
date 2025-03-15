/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
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
#include "camera_controls.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/coordinate_systems.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/math/geometry_types.h>
#include <xefis/support/math/lonlat_radius.h>
#include <xefis/support/ui/paint_helper.h>

// Neutrino:
#include <neutrino/range.h>

// Qt:
#include <QLabel>
#include <QGridLayout>

// Standard:
#include <cstddef>
#include <chrono>
#include <memory>
#include <functional>


namespace xf {

CameraControls::CameraControls()
{
	struct EarthCoordinates
	{
		SpaceLength<ECEFSpace>	ecef	{ 0_m, 0_m, 0_m };
		LonLatRadius			polar	{ 0_deg, 0_deg, 0_m };
	};

	auto const ph = PaintHelper (*this);
	auto coordinates = std::make_shared<EarthCoordinates>();

	auto const new_spinbox = [this, coordinates]<class Value, class ControlledValue> (neutrino::Range<Value> const range, Value const step, uint8_t const decimals, ControlledValue& controlled_value) {
		auto const one = Value (1);
		auto* spinbox = new QDoubleSpinBox (this);
		spinbox->setRange (range.min() / one, range.max() / one);
		spinbox->setDecimals (decimals);
		spinbox->setSuffix (QString::fromStdString (si::unit_suffix<Value>()));
		spinbox->setSingleStep (step / one);
		QObject::connect (spinbox, static_cast<void (QDoubleSpinBox::*)(double)> (&QDoubleSpinBox::valueChanged), [&controlled_value, one, coordinates] (double const double_value) {
			controlled_value = one * double_value;
		});
		return spinbox;
	};

	auto const on_value_change = []<class Function> (QDoubleSpinBox* spinbox, Function const callback) {
		QObject::connect (spinbox, static_cast<void (QDoubleSpinBox::*)(double)> (&QDoubleSpinBox::valueChanged), [callback] ([[maybe_unused]] double value) {
			callback();
		});
	};

	auto const ecef_range = Range { -2000_km, 2000_km };
	auto const ecef_step = 1_m;
	auto const ecef_decimals = 3;

	auto* ecef_x = new_spinbox.operator()<si::Length> (ecef_range, ecef_step, ecef_decimals, coordinates->ecef.x());
	auto* ecef_y = new_spinbox.operator()<si::Length> (ecef_range, ecef_step, ecef_decimals, coordinates->ecef.y());
	auto* ecef_z = new_spinbox.operator()<si::Length> (ecef_range, ecef_step, ecef_decimals, coordinates->ecef.z());

	auto const polar_step = 1e-3_deg;
	auto const polar_decimals = 6;

	// Note: order of creation is important for tab-order:
	auto* polar_lat = new_spinbox.operator()<si::Quantity<si::Degree>> ({ -90_deg, +90_deg }, polar_step, polar_decimals, coordinates->polar.lat());
	auto* polar_lon = new_spinbox.operator()<si::Quantity<si::Degree>> ({ -180_deg, +180_deg }, polar_step, polar_decimals, coordinates->polar.lon());
	auto* polar_radius = new_spinbox.operator()<si::Length> (ecef_range, ecef_step, ecef_decimals, coordinates->polar.radius());

	auto const load_to_spinbox = []<class Value> (QDoubleSpinBox* const spinbox, Value const value) {
		auto const signals_blocker = QSignalBlocker (spinbox);
		spinbox->setValue (value / Value (1));
	};

	auto const update_polar_from_ecef = [=] {
		coordinates->polar = to_polar (coordinates->ecef);
		load_to_spinbox (polar_lon, coordinates->polar.lon().to<si::Degree>());
		load_to_spinbox (polar_lat, coordinates->polar.lat().to<si::Degree>());
		load_to_spinbox (polar_radius, coordinates->polar.radius());
	};

	auto const update_ecef_from_polar = [=] {
		coordinates->ecef = to_cartesian (coordinates->polar);
		load_to_spinbox (ecef_x, coordinates->ecef.x());
		load_to_spinbox (ecef_y, coordinates->ecef.y());
		load_to_spinbox (ecef_z, coordinates->ecef.z());
	};

	on_value_change (ecef_x, update_polar_from_ecef);
	on_value_change (ecef_y, update_polar_from_ecef);
	on_value_change (ecef_z, update_polar_from_ecef);

	on_value_change (polar_lat, update_ecef_from_polar);
	on_value_change (polar_lon, update_ecef_from_polar);
	on_value_change (polar_radius, update_ecef_from_polar);

	auto* layout = new QGridLayout (this);
	auto column = 0;
	layout->addWidget (new QLabel ("ECEF position:"), 0, column);
	++column;
	layout->addItem (ph.new_fixed_horizontal_spacer (1.0), 0, column);
	++column;
	layout->addWidget (align_right (new QLabel ("X")), 0, column);
	layout->addWidget (align_right (new QLabel ("Y")), 1, column);
	layout->addWidget (align_right (new QLabel ("Z")), 2, column);
	++column;
	layout->addWidget (ecef_x, 0, column);
	layout->addWidget (ecef_y, 1, column);
	layout->addWidget (ecef_z, 2, column);
	++column;
	layout->addItem (ph.new_fixed_horizontal_spacer (2.0), 0, column);
	++column;
	layout->addWidget (new QLabel ("Polar position:"), 0, column);
	++column;
	layout->addItem (ph.new_fixed_horizontal_spacer (1.0), 0, column);
	++column;
	layout->addWidget (align_right (new QLabel ("Latitude")), 0, column);
	layout->addWidget (align_right (new QLabel ("Longitude")), 1, column);
	layout->addWidget (align_right (new QLabel ("Radius")), 2, column);
	++column;
	layout->addWidget (polar_lat, 0, column);
	layout->addWidget (polar_lon, 1, column);
	layout->addWidget (polar_radius, 2, column);
	++column;
	layout->addItem (ph.new_expanding_horizontal_spacer(), 0, column);
}

} // namespace xf

