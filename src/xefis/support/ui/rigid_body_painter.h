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

#ifndef XEFIS__SUPPORT__UI__RIGID_BODY_PAINTER_H__INCLUDED
#define XEFIS__SUPPORT__UI__RIGID_BODY_PAINTER_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>
#include <QRect>
#include <QPoint>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/euler_angles.h>
#include <xefis/support/simulation/rigid_body/system.h>
#include <xefis/support/ui/gl_space.h>


namespace xf {

/**
 * Paints state of rigid_body::System in OpenGL.
 */
class RigidBodyPainter: protected QOpenGLFunctions
{
	static constexpr auto		kDefaultPositionScale		= 0.0025_mm / 1_m;
	static constexpr auto		kDefaultMassScale			= 20_cm / 1_kg;
	static constexpr si::Length	kDefaultConstraintDiameter	= 2_cm;
	static constexpr si::Length	kDefaultHingeDiameter		= 6_cm;

	static constexpr auto		kSkyHeight					= 500_km;
	static constexpr auto		kHorizonRadius				= 500_km;
	static constexpr auto		kSunDistance				= 10_km;
	static constexpr auto		kSunRadius					= 200_km;

  public:
	// Ctor
	explicit
	RigidBodyPainter (si::PixelDensity);

	/**
	 * Follow the selected body to keep it centered on the screen.
	 * Pass nullptr to disable follow.
	 */
	void
	set_followed (rigid_body::Body* followed_body) noexcept
		{ _followed_body = followed_body; }

	/**
	 * Set planet body.
	 *
	 * It's used to correctly display ground and sky location
	 * and to have the camera oriented so that the ground is always down.
	 * It is assumed that the planet is Earth, and has Earth's size.
	 *
	 * Can be nullptr to disable planet rendering.
	 */
	void
	set_planet (rigid_body::Body* planet_body) noexcept
		{ _planet_body = planet_body; }

	/**
	 * Paint the system.
	 */
	void
	paint (rigid_body::System const& system, QOpenGLPaintDevice& canvas);

	/**
	 * Set camera focus point.
	 */
	void
	set_camera_position (SpaceLength<rigid_body::WorldSpace> const& position)
		{ _camera_position = position; }

	/**
	 * Set camera position about the focus point.
	 */
	void
	set_camera_angles (si::Angle x, si::Angle y, si::Angle z);

	/**
	 * Return true if constraints are set to be visible.
	 */
	[[nodiscard]]
	bool
	constraints_visible() const noexcept
		{ return _constraints_visible; }

	/**
	 * Show/hide constraints.
	 */
	void
	set_constraints_visible (bool visible) noexcept
		{ _constraints_visible = visible; }

	/**
	 * Return true if forces are set to be visible.
	 */
	[[nodiscard]]
	bool
	forces_visible() const noexcept
		{ return _forces_visible; }

	/**
	 * Show/hide forces.
	 */
	void
	set_forces_visible (bool visible) noexcept
		{ _forces_visible = visible; }

	/**
	 * Return true if angular velocity are set to be visible.
	 */
	[[nodiscard]]
	bool
	angular_velocities_visible() const noexcept
		{ return _angular_velocities_visible; }

	/**
	 * Show/hide angular velocity.
	 */
	void
	set_angular_velocities_visible (bool visible) noexcept
		{ _angular_velocities_visible = visible; }

	/**
	 * Return true if angular momentum are set to be visible.
	 */
	[[nodiscard]]
	bool
	angular_momenta_visible() const noexcept
		{ return _angular_momenta_visible; }

	/**
	 * Show/hide angular momentum.
	 */
	void
	set_angular_momenta_visible (bool visible) noexcept
		{ _angular_momenta_visible = visible; }

  private:
	void
	setup (QOpenGLPaintDevice&);

	void
	setup_camera();

	void
	setup_light();

	void
	paint_world (rigid_body::System const&, QOpenGLPaintDevice&);

	void
	paint_ecef_basis (QOpenGLPaintDevice&);

	void
	paint_planet();

	void
	paint_system (rigid_body::System const&, QOpenGLPaintDevice&);

	void
	paint_body (rigid_body::Body const&);

	void
	paint_constraint (rigid_body::Constraint const&);

	void
	paint_forces (rigid_body::Body const&);

	void
	paint_angular_velocity (rigid_body::Body const&);

	void
	paint_angular_momentum (rigid_body::Body const&);

	void
	draw_arrow (SpaceLength<rigid_body::WorldSpace> const& origin, SpaceLength<rigid_body::WorldSpace> const& vector, rigid_body::ShapeMaterial const& material = {});

	[[nodiscard]]
	SpaceLength<rigid_body::WorldSpace>
	followed_body_position() const;

  private:
	si::PixelDensity					_pixel_density;
	decltype (1_m / 1_kg)				_mass_scale					{ kDefaultMassScale };
	SpaceLength<rigid_body::WorldSpace>	_camera_position;
	EulerAngles							_camera_angles;
	LonLatRadius						_position_on_earth			{ 0_deg, 0_deg, 0_m };
	GLSpace								_gl;
	rigid_body::Body*					_followed_body				{ nullptr };
	rigid_body::Body*					_planet_body				{ nullptr };
	bool								_constraints_visible		{ false };
	bool								_forces_visible				{ false };
	bool								_angular_velocities_visible	{ false };
	bool								_angular_momenta_visible	{ false };
};


inline void
RigidBodyPainter::set_camera_angles (si::Angle const x, si::Angle const y, si::Angle const z)
{
	_camera_angles[0] = x;
	_camera_angles[1] = y;
	_camera_angles[2] = z;
}

} // namespace xf

#endif

