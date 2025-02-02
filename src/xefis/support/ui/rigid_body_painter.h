/* vim:ts=4
 *
 * Copyleft 2022  Michał Gawron
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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/rotations.h>
#include <xefis/support/simulation/rigid_body/system.h>
#include <xefis/support/ui/gl_space.h>

// Qt:
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>
#include <QRect>
#include <QPoint>

// Standard:
#include <cstddef>
#include <map>
#include <random>
#include <utility>
#include <variant>


namespace xf {

/**
 * Paints state of rigid_body::System in OpenGL.
 */
class RigidBodyPainter: protected QOpenGLFunctions
{
	static constexpr auto		kDefaultPositionScale		= 0.0025_mm / 1_m;
	static constexpr si::Length	kDefaultConstraintDiameter	= 1.5_cm;
	static constexpr si::Length	kDefaultHingeDiameter		= 3_cm;

	static constexpr auto		kSkyHeight					= 500_km;
	static constexpr auto		kHorizonRadius				= 500_km;
	static constexpr auto		kSunDistance				= 10_km;
	static constexpr auto		kSunRadius					= 200_km;

  public:
	struct GroupRenderingConfig
	{
		bool	center_of_mass_visible { false };
		// TODO moments_of_inertia_visible
	};

	struct BodyRenderingConfig
	{
		bool	body_visible { true };
		bool	origin_visible { false };
		bool	center_of_mass_visible { false };
		bool	moments_of_inertia_visible { false };
		// TODO forces_visible, angular_velocities visible, etc
	};

  public:
	// Ctor
	explicit
	RigidBodyPainter (si::PixelDensity);

	/**
	 * Follow the selected group to keep it centered on the screen.
	 * Pass nullptr to disable follow.
	 */
	void
	set_followed (rigid_body::Group const& followed_group) noexcept
		{ _followed = &followed_group; }

	/**
	 * Follow the selected body to keep it centered on the screen.
	 * Pass nullptr to disable follow.
	 */
	void
	set_followed (rigid_body::Body const& followed_body) noexcept
		{ _followed = &followed_body; }

	/**
	 * Disable body/group following.
	 */
	void
	set_followed_to_none() noexcept
		{ _followed = std::monostate(); }

	/**
	 * Return a followed group, if set, or nullptr.
	 */
	[[nodiscard]]
	rigid_body::Group const*
	followed_group() const noexcept;

	/**
	 * Return a followed body, if set, or nullptr.
	 */
	[[nodiscard]]
	rigid_body::Body const*
	followed_body() const noexcept;

	/**
	 * Enable camera orientation following the main body.
	 * Enabled by default.
	 */
	void
	set_following_body_orientation (bool enabled) noexcept
		{ _following_orientation = enabled; }

	[[nodiscard]]
	bool
	following_body_orientation() const noexcept
		{ return _following_orientation; }

	/**
	 * Set the focused group.
	 * Pass nullptr to set none as focused.
	 */
	void
	set_focused (rigid_body::Group const& focused_group) noexcept
		{ _focused = &focused_group; }

	/**
	 * Set the focused body. It's painted with different color.
	 * Pass nullptr to set none as focused.
	 */
	void
	set_focused (rigid_body::Body const& focused_body) noexcept
		{ _focused = &focused_body; }

	/**
	 * Unfocus any group/body.
	 */
	void
	set_focused_to_none() noexcept
		{ _focused = std::monostate(); }

	/**
	 * Return a focused group, if set, or nullptr.
	 */
	[[nodiscard]]
	rigid_body::Group const*
	focused_group() const noexcept;

	/**
	 * Return a focused body, if set, or nullptr.
	 */
	[[nodiscard]]
	rigid_body::Body const*
	focused_body() const noexcept;

	/**
	 * Return the planet body or nullptr.
	 */
	[[nodiscard]]
	rigid_body::Body const*
	planet() const noexcept
		{ return _planet_body; }

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
	set_planet (rigid_body::Body const* planet_body) noexcept
		{ _planet_body = planet_body; }

	/**
	 * Set the hovered body. It's painted with different color.
	 * Pass nullptr to set none as focused.
	 */
	void
	set_hovered (rigid_body::Body const* hovered_body) noexcept
		{ _hovered_body = hovered_body; }

	/**
	 * Set camera focus point.
	 */
	void
	set_camera_position (SpaceLength<WorldSpace> const& position)
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
	 * Return true if gravity forces are set to be visible.
	 */
	[[nodiscard]]
	bool
	gravity_visible() const noexcept
		{ return _gravity_visible; }

	/**
	 * Show/hide gravity forces.
	 */
	void
	set_gravity_visible (bool visible) noexcept
		{ _gravity_visible = visible; }

	/**
	 * Return true if aerodynamic forces are set to be visible.
	 */
	[[nodiscard]]
	bool
	aerodynamic_forces_visible() const noexcept
		{ return _aerodynamic_forces_visible; }

	/**
	 * Show/hide aerodynamic forces.
	 */
	void
	set_aerodynamic_forces_visible (bool visible) noexcept
		{ _aerodynamic_forces_visible = visible; }

	/**
	 * Return true if external forces are set to be visible.
	 */
	[[nodiscard]]
	bool
	external_forces_visible() const noexcept
		{ return _external_forces_visible; }

	/**
	 * Show/hide external forces.
	 */
	void
	set_external_forces_visible (bool visible) noexcept
		{ _external_forces_visible = visible; }

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

	/**
	 * Return config object for given group.
	 */
	[[nodiscard]]
	GroupRenderingConfig&
	get_rendering_config (rigid_body::Group const& group)
		{ return _group_rendering_config[&group]; }

	/**
	 * Return config object for given body.
	 */
	[[nodiscard]]
	BodyRenderingConfig&
	get_rendering_config (rigid_body::Body const& body)
		{ return _body_rendering_config[&body]; }

	/**
	 * Paint the system.
	 */
	void
	paint (rigid_body::System const& system, QOpenGLPaintDevice& canvas);

  private:
	void
	setup (QOpenGLPaintDevice&);

	void
	setup_feature_light();

	void
	setup_natural_light();

	void
	setup_camera();

	void
	apply_camera_rotations();

	void
	setup_light();

	void
	paint_world (rigid_body::System const&);

	/**
	 * Red - X, Green - Y, Blue - Z
	 * (RGB - XYZ)
	 */
	void
	paint_ecef_basis (QOpenGLPaintDevice&);

	void
	paint_basis (si::Length arrow_length);

	void
	paint_planet();

	void
	paint_air_particles();

	void
	paint (rigid_body::System const&);

	template<math::CoordinateSystem BaseSpace, math::CoordinateSystem Space>
		void
		transform_gl_to (Placement<BaseSpace, Space> const&);

	/**
	 * Translate and rotate OpenGL space to the center of mass of the group.
	 */
	void
	transform_gl_to_center_of_mass (rigid_body::Group const&);

	/**
	 * Translate and rotate OpenGL space to the center of mass of the body.
	 */
	void
	transform_gl_to_center_of_mass (rigid_body::Body const&);

	/**
	 * Transform OpenGL space to the body origin, assuming that the space
	 * is already transformed to body's center of mass.
	 */
	void
	transform_gl_from_body_center_of_mass_to_origin (rigid_body::Body const&);

	void
	paint (rigid_body::Body const&, BodyRenderingConfig const&);

	/**
	 * Paint additional features like XYZ basis, center of mass and origin.
	 */
	void
	paint_helpers (rigid_body::Group const&, GroupRenderingConfig const&, bool focused);

	/**
	 * Paint additional features like XYZ basis, center of mass and origin.
	 */
	void
	paint_helpers (rigid_body::Body const&, BodyRenderingConfig const&, bool focused);

	void
	paint_center_of_mass();

	void
	paint_origin();

	void
	paint_moments_of_inertia_cuboid (MassMoments<BodyCOM> const&);

	void
	paint_constraint (rigid_body::Constraint const&);

	void
	paint_forces (rigid_body::Body const&);

	void
	paint_angular_velocity (rigid_body::Body const&);

	void
	paint_angular_momentum (rigid_body::Body const&);

	void
	draw_arrow (SpaceLength<WorldSpace> const& origin, SpaceLength<WorldSpace> const& vector, rigid_body::ShapeMaterial const& material = {});

	[[nodiscard]]
	SpaceLength<WorldSpace>
	followed_position();

	[[nodiscard]]
	SpaceLength<WorldSpace>
	get_center_of_mass (rigid_body::Group const&);

  private:
	si::PixelDensity		_pixel_density;
	// Camera position is relative to the followed body:
	SpaceLength<WorldSpace>	_camera_position;
	SpaceVector<si::Angle>	_camera_angles;
	LonLatRadius			_position_on_earth			{ 0_deg, 0_deg, 0_m };
	GLSpace					_gl;
	std::variant<std::monostate, rigid_body::Group const*, rigid_body::Body const*>
							_followed;
	bool					_following_orientation		{ true };
	std::variant<std::monostate, rigid_body::Group const*, rigid_body::Body const*>
							_focused;
	rigid_body::Body const*	_planet_body				{ nullptr };
	rigid_body::Body const*	_hovered_body				{ nullptr };
	bool					_constraints_visible		{ false };
	bool					_gravity_visible			{ false };
	bool					_external_forces_visible	{ false };
	bool					_aerodynamic_forces_visible	{ false };
	bool					_angular_velocities_visible	{ false };
	bool					_angular_momenta_visible	{ false };
	std::map<rigid_body::Group const*, GroupRenderingConfig>
							_group_rendering_config;
	std::map<rigid_body::Body const*, BodyRenderingConfig>
							_body_rendering_config;
	std::minstd_rand0		_air_particles_prng;
	std::map<rigid_body::Group const*, SpaceLength<WorldSpace>>
							_group_centers_of_mass_cache;
};


inline rigid_body::Group const*
RigidBodyPainter::followed_group() const noexcept
{
	if (auto* group = std::get_if<rigid_body::Group const*> (&_followed))
		return *group;
	else
		return nullptr;
}


inline rigid_body::Body const*
RigidBodyPainter::followed_body() const noexcept
{
	if (auto* body = std::get_if<rigid_body::Body const*> (&_followed))
		return *body;
	else
		return nullptr;
}


inline rigid_body::Group const*
RigidBodyPainter::focused_group() const noexcept
{
	if (auto* group = std::get_if<rigid_body::Group const*> (&_focused))
		return *group;
	else
		return nullptr;
}


inline rigid_body::Body const*
RigidBodyPainter::focused_body() const noexcept
{
	if (auto* body = std::get_if<rigid_body::Body const*> (&_focused))
		return *body;
	else
		return nullptr;
}

} // namespace xf

#endif

