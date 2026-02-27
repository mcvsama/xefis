/* vim:ts=4
 *
 * Copyleft 2018  Michał Gawron
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
#include <QInputDialog>
#include <QMenu>
#include <QScreen>
#include <QShortcut>

// Standard:
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cmath>
#include <functional>


namespace xf {

RigidBodyViewer::RigidBodyViewer (QWidget* parent, RefreshRate const refresh_rate, nu::WorkPerformer* work_performer):
	GLAnimationWidget (parent, refresh_rate, std::bind (&RigidBodyViewer::paint, this, std::placeholders::_1)),
	_rigid_body_painter (si::PixelDensity (screen()->physicalDotsPerInch()), work_performer)
{
	setWindowTitle("Xefis rigid body viewer");
	setMouseTracking (true);

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

	_painter_ready_check_timer->callOnTimeout ([this] {
		if (_rigid_body_painter.ready())
		{
			update();
			_painter_ready_check_timer->stop();
		}
	});

	start_waiting_for_resources();
}


void
RigidBodyViewer::reset_camera_position()
{
	_camera_translation = default_camera_translation();
	forward_camera_translation();
	_camera_rotation = kDefaultCameraRotation;
	forward_camera_rotation();
}


SpaceLength<WorldSpace>
RigidBodyViewer::default_camera_translation() const noexcept
{
	if (auto const* body = followed_body())
		return auto_zoom_camera_translation (body->bounding_sphere_radius());

	if (auto const* group = followed_group())
		return auto_zoom_camera_translation (group->bounding_sphere_radius());

	return kDefaultCameraTranslation;
}


SpaceLength<WorldSpace>
RigidBodyViewer::auto_zoom_camera_translation (si::Length const followed_object_radius) noexcept
{
	if (followed_object_radius <= 0_m)
		return kDefaultCameraTranslation;

	auto const required_distance = kAutoZoomMarginFactor * followed_object_radius / sin (0.5 * kAutoZoomFOV);
	auto const camera_distance = std::max<si::Length> (required_distance, kZoomRange.min());
	return { 0_m, 0_m, camera_distance };
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
RigidBodyViewer::update()
{
	mark_dirty();
	GLAnimationWidget::update();
}


void
RigidBodyViewer::populate_rendering_menu (QMenu& menu)
{
	// "Show constraints"
	if (_rigid_body_system && !_rigid_body_system->constraints().empty())
	{
		auto* action = menu.addAction ("Show &constraints", [this] {
			auto& conf = _rigid_body_painter.features_config();
			conf.constraints_visible = !conf.constraints_visible;
			update();
		});
		action->setCheckable (true);
		action->setChecked (_rigid_body_painter.features_config().constraints_visible);
	}

	// "Show gravity"
	{
		auto* action = menu.addAction ("Show &gravity", [this] {
			auto& conf = _rigid_body_painter.features_config();
			conf.gravity_visible = !conf.gravity_visible;
			update();
		});
		action->setCheckable (true);
		action->setChecked (_rigid_body_painter.features_config().gravity_visible);
	}

	// "Show aerodynamic forces"
	{
		auto* action = menu.addAction ("Show &aerodynamic forces", [this] {
			auto& conf = _rigid_body_painter.features_config();
			conf.aerodynamic_forces_visible = !conf.aerodynamic_forces_visible;
			update();
		});
		action->setCheckable (true);
		action->setChecked (_rigid_body_painter.features_config().aerodynamic_forces_visible);
	}

	// "Show external forces"
	{
		auto* action = menu.addAction ("Show &external forces", [this] {
			auto& conf = _rigid_body_painter.features_config();
			conf.external_forces_visible = !conf.external_forces_visible;
			update();
		});
		action->setCheckable (true);
		action->setChecked (_rigid_body_painter.features_config().external_forces_visible);
	}

	// "Show angular velocities"
	{
		auto* action = menu.addAction ("Show angula&r velocities", [this] {
			auto& conf = _rigid_body_painter.features_config();
			conf.angular_velocities_visible = !conf.angular_velocities_visible;
			update();
		});
		action->setCheckable (true);
		action->setChecked (_rigid_body_painter.features_config().angular_velocities_visible);
	}

	// "Show angular momenta"
	{
		auto* action = menu.addAction ("Show angular &momenta", [this] {
			auto& conf = _rigid_body_painter.features_config();
			conf.angular_momenta_visible = !conf.angular_momenta_visible;
			update();
		});
		action->setCheckable (true);
		action->setChecked (_rigid_body_painter.features_config().angular_momenta_visible);
	}

	menu.addSeparator();

	{
		auto* action = menu.addAction ("Sun enabled", [this] {
			set_sun_enabled (!sun_enabled());
			start_waiting_for_resources();
		});
		action->setCheckable (true);
		action->setChecked (_rigid_body_painter.sun_enabled());
	}

	{
		auto* action = menu.addAction ("Universe enabled", [this] {
			set_universe_enabled (!universe_enabled());
			start_waiting_for_resources();
		});
		action->setCheckable (true);
		action->setChecked (_rigid_body_painter.universe_enabled());
	}
}


void
RigidBodyViewer::populate_context_menu_for (rigid_body::Body& body, QMenu& menu, QTreeWidgetItem* body_item)
{
	auto& rendering = get_rendering_config (body);

	{
		auto* action = menu.addAction ("&Follow this body", [this, &body] {
			set_followed (body);
			notify_system_changed();
		});
		action->setIcon (_followed_body_icon);
	}

	menu.addAction ("&Edit name", [this, &body, body_item] {
		setup_edit_action_for (body, body_item);
	});

	{
		auto* action = menu.addAction ("Break this body", [this, &body] {
			body.set_broken();
			update();
			notify_system_changed();
		});

		if (body.broken())
			action->setEnabled (false);
	}

	menu.addSeparator();

	{
		auto* action = menu.addAction ("Body visible", [this, &body, &rendering] {
			rendering.body_visible = !rendering.body_visible;
			update();
		});
		action->setCheckable (true);
		action->setChecked (rendering.body_visible);
	}

	{
		auto* action = menu.addAction ("Origin always visible", [this, &body, &rendering] {
			rendering.origin_visible = !rendering.origin_visible;
			update();
		});
		action->setCheckable (true);
		action->setChecked (rendering.origin_visible);
	}

	{
		auto* action = menu.addAction ("Center of mass always visible", [this, &body, &rendering] {
			rendering.center_of_mass_visible = !rendering.center_of_mass_visible;
			update();
		});
		action->setCheckable (true);
		action->setChecked (rendering.center_of_mass_visible);
	}

	{
		auto* action = menu.addAction ("Moments of inertia cuboid visible", [this, &body, &rendering] {
			rendering.moments_of_inertia_visible = !rendering.moments_of_inertia_visible;
			update();
		});
		action->setCheckable (true);
		action->setChecked (rendering.moments_of_inertia_visible);
	}

	if (_rigid_body_system)
	{
		if (auto const* group = _rigid_body_system->find_group_for (body))
		{
			menu.addSeparator();
			auto* action = menu.addAction ("&Follow this group", [this, group] {
				set_followed (*group);
				notify_system_changed();
			});
			action->setIcon (_followed_group_icon);
		}
	}
}


void
RigidBodyViewer::populate_context_menu_for (rigid_body::Group& group, QMenu& menu, QTreeWidgetItem* group_item)
{
	auto& rendering = get_rendering_config (group);

	{
		auto* action = menu.addAction ("&Follow this group", [this, &group] {
			set_followed (group);
			notify_system_changed();
		});
		action->setIcon (_followed_group_icon);
	}

	menu.addAction ("&Edit name", [this, &group, group_item] {
		setup_edit_action_for (group, group_item);
	});

	{
		auto* action = menu.addAction ("Center of mass always visible", [this, &group, &rendering] {
			rendering.center_of_mass_visible = !rendering.center_of_mass_visible;
			update();
		});
		action->setCheckable (true);
		action->setChecked (rendering.center_of_mass_visible);
	}
}


void
RigidBodyViewer::populate_context_menu_for (rigid_body::Constraint& constraint, QMenu& menu, QTreeWidgetItem* constraint_item)
{
	menu.addAction ("&Edit name", [this, &constraint, constraint_item] {
		setup_edit_action_for (constraint, constraint_item);
	});

	menu.addAction ("Break this constraint", [this, &constraint] {
		constraint.set_broken();
		update();
		notify_system_changed();
	});
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
			reset_camera_position();
			break;

		default:
			break;
	}

	if (event->button() == kRotationButton || event->button() == kTranslationButton)
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

	if (event->button() == kTranslationButton && !_mouse_moved_since_press)
		focus_body_from_cursor (event->pos());

	if (event->button() == Qt::RightButton)
		if (!_prevent_menu_reappear)
			if (!_mouse_moved_since_press)
				if (!display_menu (event->position().toPoint()))
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
		_camera_rotation.x() = nu::clamp<si::Angle> (_camera_rotation.x(), { -90_deg, +90_deg });
		_camera_rotation.y() = -nu::floored_mod (-_camera_rotation.y() - scale * delta.x(), 360_deg);
		forward_camera_rotation();
	}

	if (_changing_translation)
	{
		auto const scale = precision() * kTranslationScale / pixel_density;
		_camera_translation[0] += scale * -delta.x();
		_camera_translation[1] += scale * +delta.y();
		forward_camera_translation();
	}

	update_hovered_body_from_cursor (event->pos());
}


void
RigidBodyViewer::leaveEvent (QEvent* event)
{
	update_hovered_body_from_cursor (std::nullopt);
	GLAnimationWidget::leaveEvent (event);
}


void
RigidBodyViewer::wheelEvent (QWheelEvent* event)
{
	auto const distance_from_followed_object = _rigid_body_painter.camera_distance_to_followed();
	auto const unlimited_scale = precision() * 1e-3 * distance_from_followed_object / 0.2_deg;
	auto const scale = std::max (unlimited_scale, 0.01_m / 1_deg);
	auto const unlimited_z = _camera_translation.z() + scale * 1_deg * (-event->angleDelta().y() / 8.0);
	auto const z = std::clamp (unlimited_z, kZoomRange.min(), kZoomRange.max());
	_camera_translation.z() = z;

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
RigidBodyViewer::resizeEvent (QResizeEvent* event)
{
	mark_dirty();
	GLAnimationWidget::resizeEvent (event);
}


void
RigidBodyViewer::timer_update()
{
	if (_playback == Playback::Running)
	{
		_before_paint_callback (1 / refresh_rate());
		mark_dirty();
	}

	GLAnimationWidget::timer_update();
}


void
RigidBodyViewer::start_waiting_for_resources()
{
	using namespace std::literals::chrono_literals;
	_painter_ready_check_timer->start (20ms);
}


void
RigidBodyViewer::paint (QOpenGLPaintDevice& canvas)
{
	if (_before_paint_callback)
	{
		switch (_playback)
		{
			case Playback::Paused:
				break;

			case Playback::Stepping:
				if (_steps_to_do > 0)
				{
					_steps_to_do -= 1;
					_before_paint_callback (std::nullopt);
					mark_dirty();
				}
				break;

			case Playback::Running:
				// Handled in timer_update().
				break;
		}
	}

	if (_rigid_body_system && _dirty)
	{
		_rigid_body_painter.paint (*_rigid_body_system, canvas);
		_dirty = false;
	}
}


void
RigidBodyViewer::update_hovered_body_from_cursor (std::optional<QPoint> const cursor_position)
{
	rigid_body::Body const* hovered_body = nullptr;

	if (_rigid_body_system && cursor_position)
	{
		auto const& pos = *cursor_position;

		if (rect().contains (pos))
			hovered_body = _rigid_body_painter.body_under_cursor (*_rigid_body_system, pos, size());
	}

	if (hovered_body)
		_rigid_body_painter.set_hovered (*hovered_body);
	else
		_rigid_body_painter.set_hovered_to_none();

	if (_hovered_body_from_cursor != hovered_body)
	{
		_hovered_body_from_cursor = hovered_body;

		if (_hovered_body_callback)
			_hovered_body_callback (_hovered_body_from_cursor);
	}

	update();
}


void
RigidBodyViewer::focus_body_from_cursor (QPoint const& cursor_position)
{
	if (_rigid_body_system)
	{
		if (rect().contains (cursor_position))
		{
			if (auto const* body = _rigid_body_painter.body_under_cursor (*_rigid_body_system, cursor_position, size()))
			{
				_rigid_body_painter.set_focused (*body);

				if (_clicked_body_callback)
					_clicked_body_callback (body);

				update();
			}
		}
	}
}


bool
RigidBodyViewer::display_menu (QPoint const& cursor_position)
{
	QMenu menu;

	// Menu specific to clicked object:
	if (_rigid_body_system)
	{
		if (auto* body = _rigid_body_painter.body_under_cursor (*_rigid_body_system, cursor_position, size()))
		{
			populate_context_menu_for (*body, menu);
			menu.addSeparator();
			auto* rendering_menu = menu.addMenu ("Rendering");
			populate_rendering_menu (*rendering_menu);
		}
		else
			populate_rendering_menu (menu);
	}
	else
		populate_rendering_menu (menu);

	return !!menu.exec (mapToGlobal (cursor_position));
}







void
RigidBodyViewer::setup_edit_action_for (HasConfigurableLabel& object, QTreeWidgetItem* item)
{
	if (item)
		item->treeWidget()->editItem (item, 0);
	else
	{
		auto clicked_ok = false;
		auto const new_name = QInputDialog::getText (this, "Edit name", "Name:", QLineEdit::Normal, QString::fromStdString (object.label()), &clicked_ok);

		if (clicked_ok)
		{
			object.set_label (new_name.toStdString());
			notify_system_changed();
		}
	}
}

} // namespace xf
