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

#ifndef XEFIS__SUPPORT__UI__RIGID_BODY_VIEWER_H__INCLUDED
#define XEFIS__SUPPORT__UI__RIGID_BODY_VIEWER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/system.h>
#include <xefis/support/ui/gl_animation_widget.h>
#include <xefis/support/ui/rigid_body_painter.h>

// Neutrino:
#include <neutrino/qt/qutils.h>

// Qt:
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QWidget>

// Standard:
#include <cstddef>


namespace xf {

class Machine;

/**
 * Window showing rigid_body::System state as animation (but the system must be evolved elsewhere).
 * Allows rotation/translation with mouse.
 */
class RigidBodyViewer: public GLAnimationWidget
{
	Q_OBJECT

  public:
	// Evolution function called before each display frame:
	using Evolve = std::function<void (si::Time dt)>;
	using FPSMode = GLAnimationWidget::FPSMode;

	enum class Playback
	{
		Paused,
		// Use '.' (period) key to step the simulation:
		Stepping,
		Running,
	};

  public:
	static constexpr auto		kRotationButton		{ Qt::RightButton };
	static constexpr auto		kTranslationButton	{ Qt::LeftButton };
	static constexpr auto		kResetViewButton	{ Qt::BackButton };

	static constexpr si::Angle	kDefaultXAngle		{ 0_deg };
	static constexpr si::Angle	kDefaultYAngle		{ 0_deg };
	static constexpr SpaceLength<rigid_body::WorldSpace>
								kDefaultPosition	{ 0_m, 0_m, 10_m };

	static constexpr auto		kRotationScale		{ 2_deg / 1_mm };
	static constexpr auto		kTranslationScale	{ 2.5_cm / 1_mm };
	static constexpr float		kHighPrecision		{ 0.05f };

  public:
	// Ctor
	explicit
	RigidBodyViewer (QWidget* parent, RefreshRate);

	/**
	 * Assign a rigid body system. Pass nullptr to unassign.
	 *
	 * \param	evolve
	 *			Evolution function called before each redraw.
	 *			May be nullptr.
	 */
	void
	set_rigid_body_system (rigid_body::System*, Evolve evolve_function = {});

	/**
	 * Set related machine. Used to show configurator widget when pressing Esc.
	 * Pass nullptr to unset.
	 */
	void
	set_machine (xf::Machine* machine)
		{ _machine = machine; }

	/**
	 * Calls set_followed() on internal RigidBodyPainter.
	 */
	void
	set_followed (rigid_body::Body* followed_body) noexcept
		{ _rigid_body_painter.set_followed (followed_body); }

	/**
	 * Calls set_planet() on internal RigidBodyPainter.
	 */
	void
	set_planet (rigid_body::Body* planet_body) noexcept
		{ _rigid_body_painter.set_planet (planet_body); }

	/**
	 * Return current camera position.
	 */
	SpaceLength<rigid_body::WorldSpace> const&
	position() const noexcept
		{ return _position; }

	/**
	 * Return current camera rotation about the X axis.
	 */
	si::Angle
	x_angle() const noexcept
		{ return _x_angle; }

	/**
	 * Return current camera rotation about the Y axis.
	 */
	si::Angle
	y_angle() const noexcept
		{ return _y_angle; }

	/**
	 * Return true if viewer is in "paused" state
	 * (by the spacebar key).
	 */
	bool
	paused() const noexcept
		{ return _playback == Playback::Paused; }

	// QWidget API
	void
	mousePressEvent (QMouseEvent*) override;

	// QWidget API
	void
	mouseReleaseEvent (QMouseEvent*) override;

	// QWidget API
	void
	mouseMoveEvent (QMouseEvent*) override;

	// QWidget API
	void
	wheelEvent (QWheelEvent*) override;

	// QWidget API
	void
	keyPressEvent (QKeyEvent*) override;

  private:
	void
	draw (QOpenGLPaintDevice&);

	/**
	 * Return 1.0 normally or kHighPrecision value when Shift is pressed on the keyboard.
	 */
	float
	precision()
		{ return (QGuiApplication::queryKeyboardModifiers() & Qt::ShiftModifier) ? kHighPrecision : 1.0; }

	/**
	 * Display popup menu. Return true if user selected any action from the menu.
	 */
	bool
	display_menu();

  private slots:
	void
	show_configurator();

  private:
	Machine*							_machine						{ nullptr };
	rigid_body::System*					_rigid_body_system				{ nullptr };
	RigidBodyPainter					_rigid_body_painter;
	Evolve								_evolve;
	QPoint								_last_pos;
	bool								_changing_rotation: 1			{ false };
	bool								_changing_translation: 1		{ false };
	// Right-click and move causes rotation of the view, right-click without moving opens a popup menu:
	bool								_mouse_moved_since_press: 1		{ true };
	// Prevents menu reappearing immediately when trying to close it with a right click:
	bool								_prevent_menu_reappear: 1		{ false };
	Playback							_playback						{ Playback::Paused };
	SpaceLength<rigid_body::WorldSpace>	_position						{ kDefaultPosition };
	si::Angle							_x_angle						{ kDefaultXAngle };
	si::Angle							_y_angle						{ kDefaultYAngle };
};


inline void
RigidBodyViewer::set_rigid_body_system (rigid_body::System* system, Evolve evolve)
{
	_rigid_body_system = system;
	_evolve = evolve;
}

} // namespace xf

#endif

