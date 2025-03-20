/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
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
#include "rigid_body_viewer.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/machine.h>

// Qt:
#include <QCoreApplication>
#include <QMenu>
#include <QScreen>
#include <QShortcut>

// Standard:
#include <cstddef>
#include <functional>


namespace xf {

RigidBodyViewer::RigidBodyViewer (QWidget* parent, RefreshRate const refresh_rate):
	GLAnimationWidget (parent, refresh_rate, std::bind (&RigidBodyViewer::draw, this, std::placeholders::_1)),
	_rigid_body_painter (si::PixelDensity (screen()->physicalDotsPerInch()))
{
	setWindowTitle("Xefis rigid body viewer");

	{
		auto* esc = new QShortcut (this);
		esc->setKey (Qt::Key_Escape);
		QObject::connect (esc, &QShortcut::activated, [this] {
			if (_machine)
				_machine->show_configurator();
		});
	}

	forward_camera_translation();
	forward_camera_rotation();
}


void
RigidBodyViewer::toggle_pause()
{
	switch (_playback)
	{
		case Playback::Paused:
		case Playback::Stepping:
			_playback = Playback::Running;
			break;

		case Playback::Running:
			_playback = Playback::Paused;
			break;
	}
}


void
RigidBodyViewer::step()
{
	_playback = Playback::Stepping;
	_steps_to_do += 1;
}


void
RigidBodyViewer::mousePressEvent (QMouseEvent* event)
{
	switch (event->button())
	{
		case kRotationButton:
			event->accept();
			_changing_rotation = true;
			break;

		case kTranslationButton:
			event->accept();
			_changing_translation = true;
			break;

		case kResetViewButton:
			event->accept();
			_camera_translation = kDefaultCameraTranslation;
			forward_camera_translation();
			_camera_rotation = kDefaultCameraRotation;
			forward_camera_rotation();
			break;

		default:
			break;
	}

	if (event->button() == Qt::RightButton)
		_mouse_moved_since_press = false;

	_prevent_menu_reappear = false;
	_last_pos = event->pos();
}


void
RigidBodyViewer::mouseReleaseEvent (QMouseEvent* event)
{
	switch (event->button())
	{
		case kRotationButton:
			event->accept();
			_changing_rotation = false;
			break;

		case kTranslationButton:
			event->accept();
			_changing_translation = false;
			break;

		default:
			break;
	}

	if (event->button() == Qt::RightButton)
		if (!_prevent_menu_reappear)
			if (!_mouse_moved_since_press)
				if (!display_menu())
					_prevent_menu_reappear = true;

	_last_pos = event->pos();
}


void
RigidBodyViewer::mouseMoveEvent (QMouseEvent* event)
{
	auto const delta = event->pos() - _last_pos;
	_last_pos = event->pos();
	_mouse_moved_since_press = true;

	auto const pixel_density = screen()->physicalDotsPerInch() / 1_in;

	if (_changing_rotation)
	{
		auto const scale = kRotationScale / pixel_density;
		_camera_rotation.x() += scale * delta.y();
		_camera_rotation.x() = clamped<si::Angle> (_camera_rotation.x(), { -90_deg, +90_deg });
		_camera_rotation.y() = -floored_mod (-_camera_rotation.y() - scale * delta.x(), 360_deg);
		forward_camera_rotation();
	}

	if (_changing_translation)
	{
		auto const scale = precision() * kTranslationScale / pixel_density;
		_camera_translation[0] += scale * -delta.x();
		_camera_translation[1] += scale * +delta.y();
		forward_camera_translation();
	}
}


void
RigidBodyViewer::wheelEvent (QWheelEvent* event)
{
	auto const scale = precision() * 5_cm / 1_deg;
	_camera_translation[2] += scale * 1_deg * (-event->angleDelta().y() / 8.0);
	forward_camera_translation();
}


void
RigidBodyViewer::keyPressEvent (QKeyEvent* event)
{
	if (event->key() == Qt::Key_Space)
		toggle_pause();
	else if (event->key() == Qt::Key_Period)
		step();
}


void
RigidBodyViewer::draw (QOpenGLPaintDevice& canvas)
{
	if (_on_redraw)
	{
		switch (_playback)
		{
			case Playback::Paused:
				break;

			case Playback::Stepping:
				if (_steps_to_do > 0)
				{
					_steps_to_do -= 1;
					_on_redraw (std::nullopt);
				}
				break;

			case Playback::Running:
				_on_redraw (1 / refresh_rate());
				break;
		}
	}

	if (_rigid_body_system)
		_rigid_body_painter.paint (*_rigid_body_system, canvas);
}


bool
RigidBodyViewer::display_menu()
{
	QMenu menu;

	// "Show constraints"
	if (_rigid_body_system && !_rigid_body_system->constraints().empty())
	{
		auto* action = menu.addAction ("Show &constraints", [&] {
			_rigid_body_painter.set_constraints_visible (!_rigid_body_painter.constraints_visible());
		});
		action->setCheckable (true);
		action->setChecked (_rigid_body_painter.constraints_visible());
	}

	// "Show gravity"
	{
		auto* action = menu.addAction ("Show &gravity", [&] {
			_rigid_body_painter.set_gravity_visible (!_rigid_body_painter.gravity_visible());
		});
		action->setCheckable (true);
		action->setChecked (_rigid_body_painter.gravity_visible());
	}

	// "Show aerodynamic forces"
	{
		auto* action = menu.addAction ("Show &aerodynamic forces", [&] {
			_rigid_body_painter.set_aerodynamic_forces_visible (!_rigid_body_painter.aerodynamic_forces_visible());
		});
		action->setCheckable (true);
		action->setChecked (_rigid_body_painter.aerodynamic_forces_visible());
	}

	// "Show external forces"
	{
		auto* action = menu.addAction ("Show &external forces", [&] {
			_rigid_body_painter.set_external_forces_visible (!_rigid_body_painter.external_forces_visible());
		});
		action->setCheckable (true);
		action->setChecked (_rigid_body_painter.external_forces_visible());
	}

	// "Show angular velocities"
	{
		auto* action = menu.addAction ("Show angula&r velocities", [&] {
			_rigid_body_painter.set_angular_velocities_visible (!_rigid_body_painter.angular_velocities_visible());
		});
		action->setCheckable (true);
		action->setChecked (_rigid_body_painter.angular_velocities_visible());
	}

	// "Show angular momenta"
	{
		auto* action = menu.addAction ("Show angular &momenta", [&] {
			_rigid_body_painter.set_angular_momenta_visible (!_rigid_body_painter.angular_momenta_visible());
		});
		action->setCheckable (true);
		action->setChecked (_rigid_body_painter.angular_momenta_visible());
	}

	return !!menu.exec (QCursor::pos());
}

} // namespace xf

