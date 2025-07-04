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
#include <xefis/support/atmosphere/atmospheric_scattering.h>
#include <xefis/support/color/blackbody.h>
#include <xefis/support/color/spaces.h>
#include <xefis/support/math/rotations.h>
#include <xefis/support/shapes/various_materials.h>
#include <xefis/support/shapes/various_shapes.h>
#include <xefis/support/simulation/rigid_body/system.h>
#include <xefis/support/ui/gl_space.h>
#include <xefis/support/ui/sky_dome.h>
#include <xefis/support/universe/sun_position.h>

// Neutrino:
#include <neutrino/concepts.h>
#include <neutrino/synchronized.h>
#include <neutrino/work_performer.h>
#include <neutrino/value_or_ptr.h>

// Qt:
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>
#include <QOpenGLTexture>
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
  private:
	static constexpr auto		kDefaultPositionScale		= 0.0025_mm / 1_m;
	static constexpr si::Length	kDefaultConstraintDiameter	= 1.5_cm;
	static constexpr si::Length	kDefaultHingeDiameter		= 3_cm;

	static constexpr auto		kAtmosphereRadius			= kEarthMeanRadius + 50_km;
	static constexpr auto		kSunRadius					= 696'340_km;
	static constexpr auto		kSunDistance				= 147'000'000_km;
	static constexpr auto		kSunFaceAngularRadius		= 1_rad * std::atan (kSunRadius / kSunDistance);
	static constexpr auto		kSunNoonMagnification		= 1.0f;
	static constexpr auto		kSunSunsetMagnification		= 1.5f; // In reality it's 3% magnification at sunset/sunrise, but looks better with 50%

	static constexpr uint32_t	kAtmosphericSunLight		= 0b0001;
	static constexpr uint32_t	kCosmicSunLight				= 0b0010;
	static constexpr uint32_t	kSkyLight					= 0b0100;
	static constexpr uint32_t	kFeatureLight				= 0b1000;

	// Rotates the aircraft so that it's visible from behind:
	static constexpr auto		kAircraftToBehindViewRotation	= z_rotation<WorldSpace> (-90_deg) * y_rotation<WorldSpace> (90_deg);

	// Rotates the world from screen coordinates (Earth visible from top of North (Z) with Null Island on the Right (X))
	// to Null Island being visible from above (Z) with North on top of the screen (Y) and 90°/0° on the right (X).
	//
	// 1. Start with assumption that screen coordinates are equal to ECEF coordinates:
	//    X = Null Island, Y = lon/lat 90°/0° and Z = North.
	// 2. Rotate -90° around X-axis to align the Y-axis with the equator
	//   (so that Z points towards the prime meridian instead of North):
	//        x_rotation (-90_deg)
	// 3. Rotate -90° around Z-axis to shift the X-axis from Null Island towards
	//    the prime meridian and align Y with the correct eastward direction:
	//        z_rotation (-90_deg)
	static constexpr auto		kScreenToNullIslandRotation		= x_rotation<WorldSpace> (-90_deg) * z_rotation<WorldSpace> (-90_deg);
	static inline const auto	kSunQColorInSpace				= qcolor_from_temperature (kSunSurfaceTemperature);
	static inline const auto	kSunColorInSpace				= to_gl_color (kSunQColorInSpace);

	struct SkyLight
	{
		// GL_LIGHT0, GL_LIGHT1, etc:
		GLenum				gl_number;
		// Position of the point on the sky where color sample will be taken;
		// longitude 0° corresponds to current azimuth of the Sun:
		si::LonLat			position;
	};

	struct PlanetTextureImages
	{
		QImage	earth;
	};

	struct UniverseTextureImages
	{
		QImage	universe_neg_x;
		QImage	universe_neg_y;
		QImage	universe_neg_z;
		QImage	universe_pos_x;
		QImage	universe_pos_y;
		QImage	universe_pos_z;
	};

	struct PlanetTextures
	{
		std::shared_ptr<QOpenGLTexture>	earth;
	};

	struct UniverseTextures
	{
		std::shared_ptr<QOpenGLTexture>	universe_neg_x;
		std::shared_ptr<QOpenGLTexture>	universe_neg_y;
		std::shared_ptr<QOpenGLTexture>	universe_neg_z;
		std::shared_ptr<QOpenGLTexture>	universe_pos_x;
		std::shared_ptr<QOpenGLTexture>	universe_pos_y;
		std::shared_ptr<QOpenGLTexture>	universe_pos_z;
	};

	struct FeaturesConfig
	{
		bool	constraints_visible:1			{ false };
		bool	gravity_visible:1				{ false };
		bool	external_forces_visible:1		{ false };
		bool	aerodynamic_forces_visible:1	{ false };
		bool	angular_velocities_visible:1	{ false };
		bool	angular_momenta_visible:1		{ false };
	};

	struct Planet
	{
		rigid_body::Body const*	body				{ nullptr };
		std::optional<Shape>	sky_dome_shape;
		std::optional<Shape>	ground_shape;
		// Angle of horizon (always below 0°) as viewed from the camera position:
		si::Angle				horizon_angle		{ 0_deg };
		float					camera_normalized_amsl_height			{ 0.0f };
		float					camera_clamped_normalized_amsl_height	{ 0.0f }; // Range: 0…1
		float					followed_body_normalized_amsl_height	{ 0.0f }; // Range: 0…1
		static std::array<SkyLight, 5> const
								sky_lights;
	};

	struct Sun
	{
		SunPosition					position;
		// Corrected position is for the case when sun sets or rises and the face is partially covered by earth;
		// In such case the corrected position is the center of the visible part of sun's face, not the center of the circle:
		HorizontalCoordinates		corrected_position_horizontal_coordinates;
		SpaceVector<double>			corrected_position_cartesian_horizontal_coordinates;
		// Color to use when setting up OpenGL light:
		GLColor						color_on_body;
		float						magnification			{ 1.0f };
		AtmosphericScattering const	atmospheric_scattering	{{
			.earth_radius = kEarthMeanRadius,
			.atmosphere_radius = kAtmosphereRadius,
			.enable_tonemapping = true,
		}};
		Shape						face_shape				{ make_solid_circle (kSunRadius, { 0_deg, 360_deg }, 19, kWhiteMatte) };
		Shape						shines_shape			{ make_sun_shines_shape() };
	};

	struct Universe
	{
		std::optional<Shape>			sky_box_shape;
		RotationQuaternion<WorldSpace>	ecef_to_celestial_rotation;
	};

  public:
	using CameraPositionCallback = std::function<void (SpaceLength<WorldSpace>)>;

	enum CameraMode
	{
		// If there's no followed body set, CockpitView behaves the same as ChaseView.
		CockpitView,
		ChaseView,
		RCPilotView,
		FixedView,
	};

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
	/**
	 * Ctor
	 *
	 * \param	WorkPerformer
	 *			If provided, must have at least 2 threads.
	 */
	explicit
	RigidBodyPainter (si::PixelDensity, nu::WorkPerformer* = nullptr);

	/**
	 * Return true when the painter is finally ready and loaded all
	 * textures and images needed. It could be used before, too, but
	 * it would miss eg. Earth textures.
	 */
	bool
	ready() const;

	/**
	 * Assign a thread pool to use, notably when computing sky colors.
	 * Pass nullptr to disable.
	 */
	void
	use_work_performer (nu::WorkPerformer* work_performer)
		{ _work_performer = work_performer; }

	/**
	 * Set simulation time. It makes Sun rendered in different positions.
	 */
	void
	set_time (si::Time const time);

	/**
	 * Set field of view.
	 */
	void
	set_fov (si::Angle const fov)
		{ _fov = fov; }

	/**
	 * Follow the selected object to keep it centered on the screen.
	 * Call set_followed_to_none() to disable camera following.
	 */
	template<class Object>
		requires nu::SameAsAnyOf<Object, rigid_body::Group, rigid_body::Body>
		void
		set_followed (Object const& object) noexcept
		{
			_followed = &object;
			compute_followed_position();
			compute_camera_transform();
		}

	/**
	 * Disable body/group following.
	 */
	void
	set_followed_to_none();

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
	 * Return position of the followed body or group.
	 */
	[[nodiscard]]
	si::LonLatRadius<>
	followed_position() const noexcept
		{ return _followed_polar_position; }

	/**
	 * Set camera mode.
	 */
	void
	set_camera_mode (CameraMode);

	/**
	 * Set camera position.
	 * Used in camera modes RCPilotView and FixedView.
	 */
	void
	set_camera_position (si::LonLatRadius<>);

	/**
	 * Set camera position offset from followed object position.
	 */
	void
	set_user_camera_translation (SpaceLength<WorldSpace> const& translation);

	/**
	 * Set camera orientation about the followed object.
	 */
	void
	set_user_camera_rotation (SpaceVector<si::Angle> const& rotation);

	/**
	 * Get camera distance from the followed object.
	 */
	[[nodiscard]]
	si::Length
	camera_distance_to_followed() const;

	/**
	 * Set a callback to be called when camera position changes.
	 * Can be nullptr to unset.
	 */
	void
	set_camera_position_callback (RigidBodyPainter::CameraPositionCallback const callback)
		{ _camera_position_callback = callback; }

	/**
	 * Draw given object as focused (painted differently).
	 */
	template<class Object>
		requires nu::SameAsAnyOf<Object, rigid_body::Group, rigid_body::Body, rigid_body::Constraint>
		void
		set_focused (Object const& object) noexcept
			{ _focused = &object; }

	/**
	 * Unfocus any focused object.
	 */
	void
	set_focused_to_none() noexcept
		{ _focused = std::monostate(); }

	/**
	 * Return a focused group, if set, or nullptr.
	 */
	[[nodiscard]]
	rigid_body::Group const*
	focused_group() const noexcept
		{ return focused<rigid_body::Group>(); }

	/**
	 * Return a focused body, if set, or nullptr.
	 */
	[[nodiscard]]
	rigid_body::Body const*
	focused_body() const noexcept
		{ return focused<rigid_body::Body>(); }

	/**
	 * Return a focused constraint, if set, or nullptr.
	 */
	[[nodiscard]]
	rigid_body::Constraint const*
	focused_constraint() const noexcept
		{ return focused<rigid_body::Constraint>(); }

	/**
	 * Return a focused group, body or constraint, if set, or nullptr.
	 */
	template<class Object>
		requires nu::SameAsAnyOf<Object, rigid_body::Group, rigid_body::Body, rigid_body::Constraint>
		[[nodiscard]]
		Object const*
		focused() const noexcept
		{
			if (auto* object = std::get_if<Object const*> (&_focused))
				return *object;
			else
				return nullptr;
		}

	/**
	 * Set the hovered body. It's painted with different color.
	 * Call set_hovered_to_none() set none as hovered.
	 */
	template<class Object>
		requires nu::SameAsAnyOf<Object, rigid_body::Body, rigid_body::Constraint>
		void
		set_hovered (Object const& object) noexcept
			{ _hovered = &object; }

	void
	set_hovered_to_none() noexcept
		{ _hovered = std::monostate(); }

	/**
	 * Return a hovered group, body or constraint, if set, or nullptr.
	 */
	template<class Object>
		requires nu::SameAsAnyOf<Object, rigid_body::Body, rigid_body::Constraint>
		[[nodiscard]]
		Object const*
		hovered() const noexcept
		{
			if (auto* object = std::get_if<Object const*> (&_hovered))
				return *object;
			else
				return nullptr;
		}

	/**
	 * Return the planet body or nullptr.
	 */
	[[nodiscard]]
	rigid_body::Body const*
	planet() const noexcept
		{ return _planet ? _planet->body : nullptr; }

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
	set_planet (rigid_body::Body const* planet_body);

	/**
	 * Return true if rendering of sun is enabled.
	 */
	bool
	sun_enabled() const
		{ return _sun.has_value(); }

	/**
	 * Enable/disable rendering of sun.
	 */
	void
	set_sun_enabled (bool);

	/**
	 * Return true if rendering of universe is enabled.
	 */
	bool
	universe_enabled() const
		{ return _universe.has_value(); }

	/**
	 * Enable/disable rendering of the universe (stars, Milky Way, etc).
	 */
	void
	set_universe_enabled (bool);

	/**
	 * Return FeaturesConfig structure. It can be modified.
	 */
	[[nodiscard]]
	FeaturesConfig&
	features_config()
		{ return _features_config; }

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

	/**
	 * Deletes textures allocated on first creation of RigidBodyPainter
	 * and reused by other instances. Saves memory.
	 */
	static void
	drop_resources();

  private:
	void
	precompute();

	void
	setup_camera (QOpenGLPaintDevice&);

	void
	setup_camera_projection (QSize const size);

	void
	setup_modelview();

	void
	setup_lights();

	void
	setup_sun_light();

	void
	set_atmospheric_sun_light_enabled (bool);

	void
	set_cosmic_sun_light_enabled (bool);

	void
	setup_sky_light();

	void
	set_sky_light_enabled (bool);

	void
	setup_feature_light();

	void
	set_feature_light_enabled (bool);

	void
	enable_only_lights (uint32_t light_flags);

	/**
	 * Enable sun light/sky light, cosmic light or feature lights
	 * depending of presence of sun and planet.
	 */
	void
	enable_appropriate_lights();

	/**
	 * Rotates OpenGL world so that Z direction is towards the Sun.
	 * Preconditions:
	 *  • _sun_local_hour_angle and _sun_declination are computed first.
	 *  • current OpenGL rotation is identity.
	 */
	void
	make_z_towards_the_sun (SunPosition const&);

	/**
	 * Rotates OpenGL world so that Z direction is towards the center of Earth and X is towards south (azimuth 180°).
	 * Precondition: current OpenGL rotation is identity.
	 */
	void
	make_z_sky_top_x_south();

	/**
	 * Like make_z_towards_the_sun(), but make the Z touch the horizon.
	 */
	void
	make_z_sky_top_x_sun_azimuth (SunPosition const&);

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
	paint_universe_and_sun();

	void
	paint_planet();

	void
	paint_air_particles();

	void
	paint (rigid_body::System const&);

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

	void
	paint (rigid_body::Constraint const&);

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
	paint_forces (rigid_body::Body const&);

	void
	paint_angular_velocity (rigid_body::Body const&);

	void
	paint_angular_momentum (rigid_body::Body const&);

	void
	draw_arrow (SpaceLength<WorldSpace> const& origin, SpaceLength<WorldSpace> const& vector, ShapeMaterial const& material = {});

	/**
	 * Correct the sky colors to be less green at dusk/dawn.
	 */
	[[nodiscard]]
	SpaceVector<float, RGBSpace>
	sky_correction (SpaceVector<float, RGBSpace> rgb, SunPosition const&) const;

	void
	compute_followed_position();

	[[nodiscard]]
	SpaceLength<WorldSpace>
	planet_position() const;

	[[nodiscard]]
	SpaceLength<WorldSpace>
	get_center_of_mass (rigid_body::Group const&);

	/**
	 * Check if texture images need loading.
	 */
	void
	check_texture_images();

	void
	check_planet_texture_images();

	void
	check_universe_texture_images();

	/**
	 * Check if loading of textures is needed. First the check_texture_images()
	 * should be called to load images that will be needed by the textures.
	 */
	void
	check_textures();

	void
	check_planet_textures();

	void
	check_universe_textures();

	/**
	 * Check if computation of new SkyDome was requested.
	 * Also if it has been finished, start using it.
	 */
	void
	check_sky_dome_and_ground_shape();

	/**
	 * \threadsafe	As long as atmospheric_scattering is unmodified.
	 */
	[[nodiscard]]
	Shape
	compute_sky_dome_shape();

	/**
	 * Check whether sky box can be computed, and if so, compute it.
	 */
	void
	check_sky_box();

	/**
	 * Return sky box alpha (visibility) to use when painting sky box.
	 * Depends on time of day and camera altitude.
	 */
	float
	compute_sky_box_visibility (si::Angle const sun_altitude_above_horizon) const;

	void
	compute_camera_transform();

	/**
	 * Make sure camera never goes under the surface of the Earth.
	 */
	void
	fix_camera_position();

	[[nodiscard]]
	RotationQuaternion<WorldSpace>
	gravity_down_rotation (si::LonLat const position)
		{ return x_rotation<WorldSpace> (position.lat() - 90_deg) * y_rotation<WorldSpace> (-position.lon()); }

	[[nodiscard]]
	constexpr float
	compute_sun_visible_surface_factor (si::Angle const sun_altitude_above_horizon);

	/**
	 * When sun face starts to become obscured by the horizon, pretend that the center of the sun
	 * is not where it was, but at the center of visible (unobscured) shape.
	 */
	[[nodiscard]]
	HorizontalCoordinates
	corrected_sun_position_near_horizon (HorizontalCoordinates) const;

	[[nodiscard]]
	static std::shared_ptr<QOpenGLTexture>
	make_texture (QImage const& image);

	[[nodiscard]]
	static Shape
	make_sun_shines_shape();

  private:
	si::PixelDensity				_pixel_density;
	si::Angle						_fov						{ 40_deg };
	nu::ValueOrPtr<nu::WorkPerformer, std::size_t, nu::Logger const&>
									_work_performer;
	si::Time						_time;
	si::Time						_prev_saved_time;
	CameraMode						_camera_mode				{ CockpitView };
	// Requested camera position:
	si::LonLatRadius<>				_requested_camera_polar_position;
	// User offset (it's actually in camera space):
	SpaceLength<WorldSpace>			_user_camera_translation;
	// Final computed camera position (from requested camera position and user camera translation):
	si::LonLatRadius<>				_camera_polar_position;
	CameraPositionCallback			_camera_position_callback;
	// Requested camera rotation in screen coordinates:
	SpaceVector<si::Angle>			_user_camera_angles;
	RotationQuaternion<WorldSpace>
									_user_camera_rotation		{ math::identity };
	// Final computed camera rotation:
	Placement<WorldSpace, WorldSpace>
									_camera;
	SpaceLength<WorldSpace>			_camera_position_for_sky_dome;
	// Position of the followed body:
	SpaceLength<WorldSpace>			_followed_position;
	si::LonLatRadius<>				_followed_polar_position;

	GLSpace							_gl							{ 1.0 / 1_m };
	std::variant<std::monostate, rigid_body::Group const*, rigid_body::Body const*>
									_followed;
	std::variant<std::monostate, rigid_body::Group const*, rigid_body::Body const*, rigid_body::Constraint const*>
									_focused;
	std::variant<std::monostate, rigid_body::Body const*, rigid_body::Constraint const*>
									_hovered;
	FeaturesConfig					_features_config;
	std::map<rigid_body::Group const*, GroupRenderingConfig>
									_group_rendering_config;
	std::map<rigid_body::Body const*, BodyRenderingConfig>
									_body_rendering_config;
	std::minstd_rand0				_air_particles_prng;
	std::map<rigid_body::Group const*, SpaceLength<WorldSpace>>
									_group_centers_of_mass_cache;

	std::optional<Planet>			_planet;
	std::optional<Sun>				_sun;
	std::optional<Universe>			_universe;

	static nu::Synchronized<std::shared_future<PlanetTextureImages>>
									_planet_texture_images;
	static nu::Synchronized<std::shared_future<UniverseTextureImages>>
									_universe_texture_images;
	std::optional<PlanetTextures>	_planet_textures;
	std::optional<UniverseTextures>	_universe_textures;
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


constexpr float
RigidBodyPainter::compute_sun_visible_surface_factor (si::Angle const sun_altitude_above_horizon)
{
	auto const v = nu::renormalize (sun_altitude_above_horizon,
									nu::Range { -kSunFaceAngularRadius, +kSunFaceAngularRadius },
									nu::Range { 0.0f, 1.0f });
	return std::clamp<float> (v, 0.0f, 1.0f);
}

} // namespace xf

#endif

