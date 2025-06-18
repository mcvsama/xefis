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
#include <neutrino/work_performer.h>

// Qt:
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QWheelEvent>
#include <QWidget>

// Standard:
#include <cstddef>
#include <functional>
#include <optional>


namespace xf {

class Machine;

/**
 * Window showing rigid_body::System state as animation (but the system must be evolved elsewhere).
 * Allows rotation/translation with mouse.
 */
class RigidBodyViewer: public GLAnimationWidget
{
  public:
	// Evolution function called before each display frame:
	using BeforePaintCallback = std::function<void (std::optional<si::Time> frame_duration)>;
	using FPSMode = GLAnimationWidget::FPSMode;
	using GroupRenderingConfig = RigidBodyPainter::GroupRenderingConfig;
	using BodyRenderingConfig = RigidBodyPainter::BodyRenderingConfig;

	enum class Playback
	{
		Paused,
		// Use '.' (period) key to step the simulation:
		Stepping,
		Running,
	};

  public:
	static constexpr auto		kRotationButton				{ Qt::RightButton };
	static constexpr auto		kTranslationButton			{ Qt::LeftButton };
	static constexpr auto		kResetViewButton			{ Qt::BackButton };

	static constexpr si::Angle	kDefaultXAngle				{ 0_deg };
	static constexpr si::Angle	kDefaultYAngle				{ 0_deg };
	static constexpr SpaceLength<WorldSpace>
								kDefaultCameraTranslation	{ 0_m, 0_m, 10_m };
	static constexpr SpaceVector<si::Angle>
								kDefaultCameraRotation		{ 0_deg, 0_deg, 0_deg };

	static constexpr auto		kRotationScale				{ 2_deg / 1_mm };
	static constexpr auto		kTranslationScale			{ 2.5_cm / 1_mm };
	static constexpr float		kHighPrecisionFactor		{ 0.05f };

  public:
	// Ctor
	explicit
	RigidBodyViewer (QWidget* parent, RefreshRate, neutrino::WorkPerformer* work_performer = nullptr);

	/**
	 * Assign a thread pool for RigidBodyPainter.
	 */
	void
	use_work_performer (neutrino::WorkPerformer* work_performer)
		{ _rigid_body_painter.use_work_performer (work_performer); }

	/**
	 * Return used rigid body system. Might be nullptr.
	 */
	rigid_body::System const*
	rigid_body_system() const noexcept
		{ return _rigid_body_system; }

	/**
	 * Set time for RigidBodyPainter.
	 */
	void
	set_time (si::Time const time)
	{
		_rigid_body_painter.set_time (time);
		update();
	}

	/**
	 * Set field of view.
	 */
	void
	set_fov (si::Angle const fov)
	{
		_rigid_body_painter.set_fov (fov);
		update();
	}

	/**
	 * Assign a rigid body system. Pass nullptr to unassign.
	 */
	void
	set_rigid_body_system (rigid_body::System const* system)
	{
		_rigid_body_system = system;
		update();
	}

	/**
	 * Set the callback to be called on each UI frame.
	 * Use it to evolve the rigid body system.
	 * Pass nullptr to unset.
	 */
	void
	set_before_paint_callback (BeforePaintCallback const callback = {})
		{ _before_paint_callback = callback; }

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
	set_followed (auto const& object) noexcept
	{
		_rigid_body_painter.set_followed (object);
		update();
	}

	/**
	 * Calls set_followed_to_none() on internal RigidBodyPainter.
	 */
	void
	set_followed_to_none() noexcept
	{
		_rigid_body_painter.set_followed_to_none();
		update();
	}

	/**
	 * Return followed_group() from internal RigidBodyPainter.
	 */
	[[nodiscard]]
	rigid_body::Group const*
	followed_group() const noexcept
		{ return _rigid_body_painter.followed_group(); }

	/**
	 * Return followed_body() from internal RigidBodyPainter.
	 */
	[[nodiscard]]
	rigid_body::Body const*
	followed_body() const noexcept
		{ return _rigid_body_painter.followed_body(); }

	/**
	 * Return position of the followed body or group.
	 */
	[[nodiscard]]
	si::LonLatRadius<>
	followed_position() const noexcept
		{ return _rigid_body_painter.followed_position(); }

	/**
	 * Calls set_focused() on internal RigidBodyPainter.
	 */
	void
	set_focused (auto const& object) noexcept
	{
		_rigid_body_painter.set_focused (object);
		update();
	}

	/**
	 * Calls set_focused_to_none() on internal RigidBodyPainter.
	 */
	void
	set_focused_to_none() noexcept
	{
		_rigid_body_painter.set_focused_to_none();
		update();
	}

	/**
	 * Calls set_hovered() on internal RigidBodyPainter.
	 */
	void
	set_hovered (auto const& object) noexcept
	{
		_rigid_body_painter.set_hovered (object);
		update();
	}

	/**
	 * Calls set_hovered_to_none() on internal RigidBodyPainter.
	 */
	void
	set_hovered_to_none() noexcept
	{
		_rigid_body_painter.set_hovered_to_none();
		update();
	}

	/**
	 * Return the planet body.
	 */
	[[nodiscard]]
	rigid_body::Body const*
	planet() const noexcept
		{ return _rigid_body_painter.planet(); }

	/**
	 * Calls set_planet() on internal RigidBodyPainter.
	 */
	void
	set_planet (rigid_body::Body const* planet_body) noexcept
	{
		_rigid_body_painter.set_planet (planet_body);
		update();
	}

	/**
	 * Return true if rendering of sun is enabled.
	 */
	bool
	sun_enabled() const
		{ return _rigid_body_painter.sun_enabled(); }

	/**
	 * Enable/disable rendering of sun.
	 */
	void
	set_sun_enabled (bool enabled)
	{
		_rigid_body_painter.set_sun_enabled (enabled);
		update();
	}

	/**
	 * Return true if rendering of universe is enabled.
	 */
	bool
	universe_enabled() const
		{ return _rigid_body_painter.universe_enabled(); }

	/**
	 * Enable/disable rendering of the universe (stars, Milky Way, etc).
	 */
	void
	set_universe_enabled (bool enabled)
	{
		_rigid_body_painter.set_universe_enabled (enabled);
		update();
	}

	/**
	 * Forward the mode to RigidBodyPainter.
	 */
	void
	set_camera_mode (RigidBodyPainter::CameraMode const mode)
	{
		_rigid_body_painter.set_camera_mode (mode);
		update();
	}

	/**
	 * Reset camera position to default.
	 */
	void
	reset_camera_position();

	/**
	 * Return current camera position.
	 */
	[[nodiscard]]
	SpaceLength<WorldSpace> const&
	camera_translation() const noexcept
		{ return _camera_translation; }

	/**
	 * Return current camera angles.
	 */
	[[nodiscard]]
	SpaceVector<si::Angle> const&
	camera_rotation() const noexcept
		{ return _camera_rotation; }

	/**
	 * Set a callback to be called when camera position changes.
	 * Can be nullptr to unset.
	 *
	 * This isn't the same as translation; translation is relative
	 * to the body, position is absolute and computed by the RigidBodyPainter.
	 */
	void
	set_camera_position_callback (RigidBodyPainter::CameraPositionCallback const callback)
		{ _rigid_body_painter.set_camera_position_callback (callback); }

	/**
	 * Return playback mode.
	 */
	[[nodiscard]]
	Playback
	playback() const noexcept
		{ return _playback; }

	/**
	 * Toggle pause.
	 */
	void
	toggle_pause();

	/**
	 * Go into stepping mode of the simulation and make a single
	 * step forward.
	 */
	void
	step();

	[[nodiscard]]
	GroupRenderingConfig&
	get_rendering_config (rigid_body::Group const& group)
		{ return _rigid_body_painter.get_rendering_config (group); }

	[[nodiscard]]
	BodyRenderingConfig&
	get_rendering_config (rigid_body::Body const& body)
		{ return _rigid_body_painter.get_rendering_config (body); }

	/**
	 * Force redrawing of the stuff, after eg. rendering config was modified.
	 */
	void
	update();

  protected:
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

	// QWidget API
	void
	resizeEvent (QResizeEvent*) override;

  private:
	/**
	 * Make sure that when not-previously loaded resources (textures) are needed now
	 * (eg. when user enabled the Universe), we will know about it and run the update()
	 * when they're ready.
	 */
	void
	start_waiting_for_resources();

	void
	mark_dirty() noexcept
		{ _dirty = true; }

	void
	paint (QOpenGLPaintDevice&);

	/**
	 * Return 1.0 normally or kHighPrecisionFactor value when Shift is pressed on the keyboard.
	 */
	[[nodiscard]]
	float
	precision()
		{ return (QGuiApplication::queryKeyboardModifiers() & Qt::ShiftModifier) ? kHighPrecisionFactor : 1.0; }

	/**
	 * Display popup menu. Return true if user selected any action from the menu.
	 */
	bool
	display_menu();

	/**
	 * Forward current camera position to the RigidBodyPainter.
	 */
	void
	forward_camera_translation()
	{
		_rigid_body_painter.set_user_camera_translation (_camera_translation);
		update();
	}

	/**
	 * Forward current camera angles to the RigidBodyPainter.
	 */
	void
	forward_camera_rotation()
	{
		_rigid_body_painter.set_user_camera_rotation (_camera_rotation);
		update();
	}

  private:
	Machine*					_machine						{ nullptr };
	rigid_body::System const*	_rigid_body_system				{ nullptr };
	RigidBodyPainter			_rigid_body_painter;
	BeforePaintCallback			_before_paint_callback;
	bool						_dirty							{ true };
	QPoint						_last_pos;
	bool						_changing_rotation: 1			{ false };
	bool						_changing_translation: 1		{ false };
	// Right-click and move causes rotation of the view, right-click without moving opens a popup menu:
	bool						_mouse_moved_since_press: 1		{ true };
	// Prevents menu reappearing immediately when trying to close it with a right click:
	bool						_prevent_menu_reappear: 1		{ false };
	Playback					_playback						{ Playback::Paused };
	std::size_t					_steps_to_do					{ 0 };
	// Camera position relative to the followed body:
	SpaceLength<WorldSpace>		_camera_translation				{ kDefaultCameraTranslation };
	SpaceVector<si::Angle>		_camera_rotation				{ kDefaultCameraRotation };
	// Self-deletes after a while and becomes a dangling pointer:
	QTimer*						_painter_ready_check_timer		{ new QTimer (this) };
};

} // namespace xf

#endif

