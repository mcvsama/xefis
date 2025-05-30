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
#include <xefis/support/ui/paint_helper.h>

// Neutrino:
#include <neutrino/range.h>

// Qt:
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>

// Standard:
#include <cstddef>
#include <chrono>
#include <memory>
#include <functional>


namespace xf {

template<class Value>
	void
	load_to_spinbox (QDoubleSpinBox* const spinbox, Value const value)
	{
		auto const signals_blocker = QSignalBlocker (spinbox);
		spinbox->setValue (value / Value (1));
	}


CameraControls::CameraControls (RigidBodyViewer& viewer, QWidget* parent):
	QWidget (parent),
	_rigid_body_viewer (viewer)
{

	auto const ph = PaintHelper (*this);
	_coordinates = std::make_shared<EarthCoordinates>();

	auto const new_spinbox = [this]<class Value, class ControlledValue> (neutrino::Range<Value> const range, Value const step, uint8_t const decimals, ControlledValue& controlled_value) {
		auto const one = Value (1);
		auto* spinbox = new QDoubleSpinBox (this);
		spinbox->setRange (range.min() / one, range.max() / one);
		spinbox->setDecimals (decimals);
		spinbox->setSuffix (QString::fromStdString (si::unit_suffix<Value>()));
		spinbox->setSingleStep (step / one);
		// For now they're read only. See comment in update_rigid_body_viewer_camera_position().
		spinbox->setEnabled (false);

		QObject::connect (spinbox, static_cast<void (QDoubleSpinBox::*)(double)> (&QDoubleSpinBox::valueChanged), [&controlled_value, one] (double const double_value) {
			controlled_value = one * double_value;
		});

		return spinbox;
	};

	auto const ecef_range = Range { -20'000_km, 20'000_km };
	auto const ecef_step = 1_m;
	auto const ecef_decimals = 3;

	_ecef_x = new_spinbox.operator()<si::Length> (ecef_range, ecef_step, ecef_decimals, _coordinates->ecef.x());
	_ecef_y = new_spinbox.operator()<si::Length> (ecef_range, ecef_step, ecef_decimals, _coordinates->ecef.y());
	_ecef_z = new_spinbox.operator()<si::Length> (ecef_range, ecef_step, ecef_decimals, _coordinates->ecef.z());

	auto const polar_step = 1e-3_deg;
	auto const polar_decimals = 6;

	// Note: order of creation is important for tab-order:
	_polar_lat = new_spinbox.operator()<si::Quantity<si::Degree>> ({ -90_deg, +90_deg }, polar_step, polar_decimals, _coordinates->polar.lat());
	_polar_lon = new_spinbox.operator()<si::Quantity<si::Degree>> ({ -180_deg, +180_deg }, polar_step, polar_decimals, _coordinates->polar.lon());
	_polar_radius = new_spinbox.operator()<si::Length> (ecef_range, ecef_step, ecef_decimals, _coordinates->polar.radius());

	auto* const cockpit_view = new QRadioButton ("Cockpit view", this);
	auto* const chase_view = new QRadioButton ("Chase view", this);
	auto* const rc_pilot_view = new QRadioButton ("RC pilot view", this);
	auto* const fixed_view = new QRadioButton ("Manual view", this);

	auto* const mode_group_box = new QGroupBox (this);
	auto* const mode_layout = new QVBoxLayout (mode_group_box);
	mode_layout->setContentsMargins (ph.group_box_margins());
	mode_layout->addWidget (cockpit_view);
	mode_layout->addWidget (chase_view);
	mode_layout->addWidget (rc_pilot_view);
	mode_layout->addWidget (fixed_view);
	mode_layout->addItem (ph.new_expanding_vertical_spacer());

	// TODO Hide for now, unhide when support is ready:
	rc_pilot_view->hide();
	fixed_view->hide();

	auto* const reset_position = new QPushButton ("↺", this);
	reset_position->setToolTip ("Reset postion and rotation to default");
	QObject::connect (reset_position, &QPushButton::clicked, [this] {
		_rigid_body_viewer.reset_camera_position();
	});

	auto* const fov = new QSpinBox (this);
	fov->setRange (30, 90);
	fov->setSuffix ("°");
	QObject::connect (fov, &QSpinBox::valueChanged, [this] (int value) {
		_rigid_body_viewer.set_fov (1_deg * value);
	});
	fov->setValue (40);

	auto* const fov_group_box = new QGroupBox (this);
	auto* const fov_layout = new QVBoxLayout (fov_group_box);
	fov_layout->addWidget (new QLabel ("View FOV:"));
	fov_layout->addWidget (fov);
	fov_layout->addItem (ph.new_expanding_vertical_spacer());

	auto* const position_group_box = new QGroupBox (this);
	auto* const position_layout = new QGridLayout (position_group_box);
	{
		auto column = 0;
		position_layout->addWidget (new QLabel ("ECEF position:"), 0, column);
		position_layout->addWidget (reset_position, 2, column);
		++column;
		position_layout->addItem (ph.new_fixed_horizontal_spacer (0.5), 0, column);
		++column;
		position_layout->addWidget (align_right (new QLabel ("X:")), 0, column);
		position_layout->addWidget (align_right (new QLabel ("Y:")), 1, column);
		position_layout->addWidget (align_right (new QLabel ("Z:")), 2, column);
		++column;
		position_layout->addWidget (_ecef_x, 0, column);
		position_layout->addWidget (_ecef_y, 1, column);
		position_layout->addWidget (_ecef_z, 2, column);
		++column;
		position_layout->addItem (ph.new_fixed_horizontal_spacer (1.0), 0, column);
		++column;
		position_layout->addWidget (new QLabel ("Polar position:"), 0, column);
		++column;
		position_layout->addItem (ph.new_fixed_horizontal_spacer (0.5), 0, column);
		++column;
		position_layout->addWidget (align_right (new QLabel ("Latitude:")), 0, column);
		position_layout->addWidget (align_right (new QLabel ("Longitude:")), 1, column);
		position_layout->addWidget (align_right (new QLabel ("Radius:")), 2, column);
		++column;
		position_layout->addWidget (_polar_lat, 0, column);
		position_layout->addWidget (_polar_lon, 1, column);
		position_layout->addWidget (_polar_radius, 2, column);
	}

	auto* const layout = new QHBoxLayout (this);
	layout->addWidget (mode_group_box);
	layout->addWidget (fov_group_box);
	layout->addWidget (position_group_box);
	layout->addItem (ph.new_expanding_horizontal_spacer());

	// -- Behavior --

	auto const on_value_change = []<class Function> (QDoubleSpinBox* spinbox, Function const callback) {
		QObject::connect (spinbox, static_cast<void (QDoubleSpinBox::*)(double)> (&QDoubleSpinBox::valueChanged), [callback] ([[maybe_unused]] double value) {
			callback();
		});
	};

	auto const ecef_changed = [this] {
		update_polar_from_ecef();
		update_rigid_body_viewer_camera_position();
	};

	on_value_change (_ecef_x, ecef_changed);
	on_value_change (_ecef_y, ecef_changed);
	on_value_change (_ecef_z, ecef_changed);

	auto const polar_changed = [this] {
		update_ecef_from_polar();
		update_rigid_body_viewer_camera_position();
	};

	on_value_change (_polar_lat, polar_changed);
	on_value_change (_polar_lon, polar_changed);
	on_value_change (_polar_radius, polar_changed);

	QObject::connect (cockpit_view, &QRadioButton::clicked, [this](bool) {
		_rigid_body_viewer.set_camera_mode (RigidBodyPainter::CockpitView);
	});
	QObject::connect (chase_view, &QRadioButton::clicked, [this](bool) {
		_rigid_body_viewer.set_camera_mode (RigidBodyPainter::ChaseView);
	});
	QObject::connect (rc_pilot_view, &QRadioButton::clicked, [this](bool) {
		_rigid_body_viewer.set_camera_mode (RigidBodyPainter::RCPilotView);
	});
	QObject::connect (fixed_view, &QRadioButton::clicked, [this](bool) {
		_rigid_body_viewer.set_camera_mode (RigidBodyPainter::FixedView);
	});

	cockpit_view->setChecked (true);

	_rigid_body_viewer.set_camera_position_callback ([this] (SpaceLength<WorldSpace> const camera_position) {
		set_camera_position (camera_position);
	});
}


void
CameraControls::set_camera_position (SpaceLength<WorldSpace> const position)
{
	auto const signals_blocker_x = QSignalBlocker (_ecef_x);
	auto const signals_blocker_y = QSignalBlocker (_ecef_y);
	auto const signals_blocker_z = QSignalBlocker (_ecef_z);

	_ecef_x->setValue (position.x().in<si::Meter>());
	_ecef_y->setValue (position.y().in<si::Meter>());
	_ecef_z->setValue (position.z().in<si::Meter>());

	update_coordinates_from_ecef();
	update_polar_from_ecef();
}


void
CameraControls::update_polar_from_ecef()
{
	_coordinates->polar = to_polar (_coordinates->ecef);
	load_to_spinbox (_polar_lon, _coordinates->polar.lon().to<si::Degree>());
	load_to_spinbox (_polar_lat, _coordinates->polar.lat().to<si::Degree>());
	load_to_spinbox (_polar_radius, _coordinates->polar.radius());
};


void
CameraControls::update_ecef_from_polar()
{
	_coordinates->ecef = to_cartesian (_coordinates->polar);
	load_to_spinbox (_ecef_x, _coordinates->ecef.x());
	load_to_spinbox (_ecef_y, _coordinates->ecef.y());
	load_to_spinbox (_ecef_z, _coordinates->ecef.z());
};


void
CameraControls::update_coordinates_from_ecef()
{
	_coordinates->ecef.x() = 1_m * _ecef_x->value();
	_coordinates->ecef.y() = 1_m * _ecef_y->value();
	_coordinates->ecef.z() = 1_m * _ecef_z->value();
}


void
CameraControls::update_rigid_body_viewer_camera_position()
{
	// TODO This is not so easy, and may be not implemented at all.
	// What needs to be done is the inverse of RigidBodyPainter::calculate_camera_transform()
	// to get offset/rotation used in RigidBodyViewer.
	// For now the camera position widgets are set to read-only.
}

} // namespace xf

