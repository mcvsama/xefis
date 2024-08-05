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

#ifndef XEFIS__SUPPORT__SIMULATION__RIGID_BODY__BODY_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__RIGID_BODY__BODY_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/earth/air/atmosphere.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/math/placement.h>
#include <xefis/support/nature/acceleration_moments.h>
#include <xefis/support/nature/force_moments.h>
#include <xefis/support/nature/mass_moments.h>
#include <xefis/support/nature/velocity_moments.h>
#include <xefis/support/nature/wrench.h>
#include <xefis/support/simulation/rigid_body/concepts.h>
#include <xefis/support/simulation/rigid_body/shape.h>

// Neutrino:
#include <neutrino/noncopyable.h>
#include <neutrino/numeric.h>
#include <neutrino/stdexcept.h>

// Standard:
#include <cstddef>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <string>


namespace xf::rigid_body {

/**
 * A set of calculations related to the body done by the simulator on each frame.
 */
class BodyFrameCache
{
  public:
	SpaceMatrix<si::Mass, WorldSpace>::InversedMatrix				inv_M;
	SpaceMatrix<si::MomentOfInertia, WorldSpace>::InversedMatrix	inv_I;
	ForceMoments<WorldSpace>										gravitational_force_moments;
	ForceMoments<WorldSpace>										external_force_moments; // Excluding gravitation.
	ForceMoments<WorldSpace>										constraint_force_moments;
	// Those are used temporarily when converging constraint_force_moments:
	AccelerationMoments<WorldSpace>									acceleration_moments;
	AccelerationMoments<WorldSpace>									acceleration_moments_except_gravity;
	VelocityMoments<WorldSpace>										velocity_moments;

  public:
	[[nodiscard]]
	ForceMoments<WorldSpace>
	all_force_moments() const noexcept
		{ return gravitational_force_moments + external_force_moments + constraint_force_moments; }

	[[nodiscard]]
	ForceMoments<WorldSpace>
	force_moments_except_gravity() const noexcept
		{ return external_force_moments + constraint_force_moments; }
};


/**
 * Rigid body.
 */
class Body: public Noncopyable
{
  public:
	enum ShapeType
	{
		ShapeIsConstant,
		ShapeChanges,
	};

  public:
	// Ctor
	template<class MassMomentsSpace>
		Body (MassMoments<MassMomentsSpace> const&, ShapeType = ShapeIsConstant);

	// Dtor
	virtual
	~Body() = default;

	[[nodiscard]]
	std::string const&
	label() const noexcept
		{ return _label; }

	void
	set_label (std::string const& label)
		{ _label = label; }

	/**
	 * Return mass moments at center-of-mass.
	 */
	template<CoordinateSystemConcept Space>
		[[nodiscard]]
		MassMoments<Space>
		mass_moments() const;

	/**
	 * Set new mass moments at center-of-mass.
	 */
	template<CoordinateSystemConcept Space>
		void
		set_mass_moments (MassMoments<Space> const&);

	/**
	 * Return placement of center-of-mass.
	 */
	[[nodiscard]]
	Placement<WorldSpace, BodyCOM> const&
	placement() const noexcept
		{ return _placement; }

	/**
	 * Set new placement of center-of-mass.
	 */
	void
	set_placement (Placement<WorldSpace, BodyCOM> const& placement) noexcept
		{ _placement = placement; }

	/**
	 * Return placement of origin point (relative to center-of-mass).
	 */
	[[nodiscard]]
	Placement<BodyCOM, BodyOrigin> const&
	origin_placement() const noexcept
		{ return _origin_placement; }

	/**
	 * Set new placement of origin (relative to center-of-mass).
	 */
	void
	set_origin_placement (Placement<BodyCOM, BodyOrigin> const& origin_placement) noexcept
		{ _origin_placement = origin_placement; }

	/**
	 * Return velocity moments of the center of mass in World coordinate system.
	 */
	template<CoordinateSystemConcept Space>
		[[nodiscard]]
		VelocityMoments<Space> const&
		velocity_moments() const;

	/**
	 * Set new velocity moments of center-of-mass.
	 */
	template<CoordinateSystemConcept Space>
		void
		set_velocity_moments (VelocityMoments<Space> const&);

	/**
	 * Return acceleration moments of center-of-mass.
	 */
	template<CoordinateSystemConcept Space>
		[[nodiscard]]
		AccelerationMoments<Space> const&
		acceleration_moments() const;

	/**
	 * Set new acceleration moments of center-of-mass.
	 */
	template<CoordinateSystemConcept Space>
		void
		set_acceleration_moments (AccelerationMoments<Space> const&);

	/**
	 * Return acceleration moments excluding gravitational acceleration moments.
	 * This is what an onboard accelerometer would return.
	 */
	template<CoordinateSystemConcept Space>
		[[nodiscard]]
		AccelerationMoments<Space>
		acceleration_moments_except_gravity() const;

	/**
	 * Return total external force moments generated by this object at the center-of-mass.
	 */
	template<CoordinateSystemConcept Space>
		[[nodiscard]]
		ForceMoments<Space> const&
		external_force_moments() const;

	/**
	 * Apply force at center-of-mass for the duration of the following simulation frame.
	 * It will be treated as external force-moments for this body.
	 * Multiple calls add new forces instead of overwriting last one.
	 */
	template<CoordinateSystemConcept Space>
		void
		apply_impulse (ForceMoments<Space> const& force_moments);

	/**
	 * Apply force at given position relative to the center-of-mass for the duration of the following simulation frame.
	 * See remarks for apply_impulse (ForceMoments<Space>).
	 */
	template<CoordinateSystemConcept ForceSpace, CoordinateSystemConcept PositionSpace>
		void
		apply_impulse (ForceMoments<ForceSpace> const& force_moments, SpaceLength<PositionSpace> const& position);

	/**
	 * Apply force for the duration of the following simulation frame.
	 * Wrench should be relative to body's center-of-mass.
	 * See remarks for apply_impulse (ForceMoments<Space>).
	 */
	template<CoordinateSystemConcept Space>
		void
		apply_impulse (Wrench<Space> const& wrench);

	/**
	 * Resets all applied impulses.
	 */
	void
	reset_applied_impulses() noexcept;

	/**
	 * Return body's shape.
	 * It should be assumet it can change from frame to frame unless shape_is_constant()
	 * returns true.
	 */
	[[nodiscard]]
	std::optional<Shape> const&
	shape() const noexcept
		{ return _shape; }

	/**
	 * Set body shape. Shape's origin will be placed at body's center of mass.
	 */
	void
	set_shape (std::optional<Shape> const&);

	/**
	 * Set body shape.
	 */
	void
	set_shape (std::optional<Shape>&& shape)
		{ _shape = std::move (shape); }

	/**
	 * Return true if shape() never changes and can be cached by the caller.
	 */
	[[nodiscard]]
	bool
	shape_is_constant() const noexcept
		{ return _shape_type == ShapeIsConstant; }

	/**
	 * Position of origin in chosen Space coordinate system.
	 */
	template<CoordinateSystemConcept Space>
		[[nodiscard]]
		SpaceLength<Space>
		origin() const;

	/**
	 * Rotate the body about it's center of mass by provided rotation matrix.
	 */
	void
	rotate_about_center_of_mass (RotationMatrix<WorldSpace> const&);

	/**
	 * Rotate the body about world space origin by provided rotation matrix.
	 */
	void
	rotate_about_world_origin (RotationMatrix<WorldSpace> const&);

	/**
	 * Rotate the body about the body origin.
	 */
	void
	rotate_about_body_origin (RotationMatrix<WorldSpace> const&);

	/**
	 * Rotate the body about given point.
	 */
	void
	rotate_about (SpaceLength<WorldSpace> const& about_point, RotationMatrix<WorldSpace> const& rotation)
		{ _placement.rotate_base_frame_about (about_point, rotation); }

	/**
	 * Translate the body by given vector.
	 */
	template<CoordinateSystemConcept Space = WorldSpace>
		void
		translate (SpaceLength<Space> const& translation);

	/**
	 * Translate the body so that its center of mass is at newly given point.
	 */
	void
	move_to (SpaceLength<WorldSpace> const& new_position)
		{ _placement.set_position (new_position); }

	/**
	 * Translate the body so that its origin is at newly given point.
	 */
	void
	move_origin_to (SpaceLength<WorldSpace> const&);

	/**
	 * Calculate translational kinetic energy of the body in WorldSpace frame of reference.
	 */
	[[nodiscard]]
	si::Energy
	translational_kinetic_energy() const;

	/**
	 * Calculate rotational kinetic energy of the body in WorldSpace frame of reference.
	 */
	[[nodiscard]]
	si::Energy
	rotational_kinetic_energy() const;

	/**
	 * Return frame cache of the body.
	 * To be used by the simulator.
	 */
	[[nodiscard]]
	BodyFrameCache&
	frame_cache() noexcept
		{ return _frame_cache; }

	/**
	 * Return frame cache of the body.
	 * To be used by the simulator.
	 */
	[[nodiscard]]
	BodyFrameCache const&
	frame_cache() const noexcept
		{ return _frame_cache; }

	/**
	 * Return true if the body is broken and must not be used in the system
	 * evolution (eg. it contains NaN values anywhere).
	 */
	[[nodiscard]]
	bool
	broken() const
		{ return _broken; }

	/**
	 * Mark body as broken to remove it from calculations (but not from the
	 * system, it should simply be skipped when evolving the system, etc).
	 */
	void
	set_broken() noexcept
		{ _broken = true; }

	/**
	 * Evolve the body (eg. change the shape if it's changeable).
	 */
	virtual void
	evolve ([[maybe_unused]] si::Time dt)
	{ }

	/**
	 * Ask body to update external force moments by calling apply_impulse() methods.
	 */
	virtual void
	update_external_forces (Atmosphere const*)
	{ }

  private:
	std::string											_label;
	MassMoments<BodyCOM>								_mass_moments;
	// Location of center-of-mass:
	Placement<WorldSpace, BodyCOM>						_placement;
	// Location of origin:
	Placement<BodyCOM, BodyOrigin>						_origin_placement;
	// Velocity of center-of-mass:
	VelocityMoments<WorldSpace>							_velocity_moments;
	mutable std::optional<VelocityMoments<BodyCOM>>		_body_space_velocity_moments;
	// Acceleration moments of center-of-mass:
	AccelerationMoments<WorldSpace>						_acceleration_moments;
	mutable std::optional<AccelerationMoments<BodyCOM>>	_body_space_acceleration_moments;
	// Impulses applied for the duration of the simulation frame:
	mutable std::optional<ForceMoments<WorldSpace>>		_world_space_applied_impulses;
	ForceMoments<BodyCOM>								_applied_impulses;
	// Stuff calculated when simulation is run:
	BodyFrameCache										_frame_cache;
	// Body shape:
	std::optional<Shape>								_shape;
	ShapeType											_shape_type;
	// Mutex for all those _world_space_* and _body_space_* optionals:
	mutable std::mutex									_optionals_mutex;
	// The body is not valid for computation anymore (eg. has NaNs in physical quantities):
	bool												_broken { false };
};


template<class MassMomentsSpace>
	inline
	Body::Body (MassMoments<MassMomentsSpace> const& mass_moments, ShapeType const shape_type):
		_shape_type (shape_type)
	{
		set_mass_moments (mass_moments);
	}


template<CoordinateSystemConcept Space>
	inline MassMoments<Space>
	Body::mass_moments() const
	{
		if constexpr (std::is_same_v<Space, BodyCOM>)
			return _mass_moments;
		else
			static_assert (false, "Unsupported coordinate system");
	}


template<CoordinateSystemConcept Space>
	inline void
	Body::set_mass_moments (MassMoments<Space> const& mass_moments)
	{
		auto const com_position = mass_moments.center_of_mass_position();

		// We want mass moments to be viewed from the center of mass, so translate if necessary
		// (this should transform the inertia tensor accordingly):
		if constexpr (std::is_same_v<Space, BodyCOM>)
			_mass_moments = mass_moments.centered_at_center_of_mass();
		else
			static_assert (false, "Unsupported coordinate system");

		// Move the body so that the placement().position() points to the current center of mass:
		translate<BodyCOM> (com_position);

		// Because the origin is defined as relative to center of mass, and we just moved center of mass
		// while not wanting to move origin, move the origin back:
		_origin_placement.translate_frame (-com_position);
	}


template<CoordinateSystemConcept Space>
	inline VelocityMoments<Space> const&
	Body::velocity_moments() const
	{
		if constexpr (std::is_same_v<Space, WorldSpace>)
			return _velocity_moments;
		else if constexpr (std::is_same_v<Space, BodyCOM>)
		{
			std::lock_guard lock (_optionals_mutex);

			if (!_body_space_velocity_moments)
				_body_space_velocity_moments = _placement.unbound_transform_to_body (_velocity_moments);

			return *_body_space_velocity_moments;
		}
		else
			static_assert (false, "Unsupported coordinate system");
	}


template<CoordinateSystemConcept Space>
	inline void
	Body::set_velocity_moments (VelocityMoments<Space> const& velocity_moments)
	{
		_body_space_velocity_moments.reset();

		if constexpr (std::is_same_v<Space, WorldSpace>)
			_velocity_moments = velocity_moments;
		else if constexpr (std::is_same_v<Space, BodyCOM>)
			_velocity_moments = _placement.unbound_transform_to_base (velocity_moments);
		else
			static_assert (false, "Unsupported coordinate system");
	}


template<CoordinateSystemConcept Space>
	inline AccelerationMoments<Space> const&
	Body::acceleration_moments() const
	{
		if constexpr (std::is_same_v<Space, WorldSpace>)
			return _acceleration_moments;
		else if constexpr (std::is_same_v<Space, BodyCOM>)
		{
			std::lock_guard lock (_optionals_mutex);

			if (!_body_space_acceleration_moments)
				_body_space_acceleration_moments = _placement.unbound_transform_to_body (_acceleration_moments);

			return *_body_space_acceleration_moments;
		}
		else
			static_assert (false, "Unsupported coordinate system");
	}


template<CoordinateSystemConcept Space>
	inline void
	Body::set_acceleration_moments (AccelerationMoments<Space> const& acceleration_moments)
	{
		_body_space_acceleration_moments.reset();

		if constexpr (std::is_same_v<Space, WorldSpace>)
			_acceleration_moments = acceleration_moments;
		else if constexpr (std::is_same_v<Space, BodyCOM>)
			_acceleration_moments = _placement.unbound_transform_to_base (acceleration_moments);
		else
			static_assert (false, "Unsupported coordinate system");
	}


template<CoordinateSystemConcept Space>
	inline AccelerationMoments<Space>
	Body::acceleration_moments_except_gravity() const
	{
		if constexpr (std::is_same_v<Space, WorldSpace>)
			return frame_cache().acceleration_moments_except_gravity;
		else if constexpr (std::is_same_v<Space, BodyCOM>)
			return _placement.unbound_transform_to_body (frame_cache().acceleration_moments_except_gravity);
		else
			static_assert (false, "Unsupported coordinate system");
	}


template<CoordinateSystemConcept Space>
	inline ForceMoments<Space> const&
	Body::external_force_moments() const
	{
		if constexpr (std::is_same_v<Space, WorldSpace>)
		{
			std::lock_guard lock (_optionals_mutex);

			if (!_world_space_applied_impulses)
				_world_space_applied_impulses = _placement.unbound_transform_to_base (_applied_impulses);

			return *_world_space_applied_impulses;
		}
		else if constexpr (std::is_same_v<Space, BodyCOM>)
			return _applied_impulses;
		else
			static_assert (false, "Unsupported coordinate system");
	}


template<CoordinateSystemConcept Space>
	inline void
	Body::apply_impulse (ForceMoments<Space> const& force_moments)
	{
		_world_space_applied_impulses.reset();

		if constexpr (std::is_same_v<Space, WorldSpace>)
			_applied_impulses += _placement.unbound_transform_to_body (force_moments);
		else if constexpr (std::is_same_v<Space, BodyCOM>)
			_applied_impulses += force_moments;
		else
			static_assert (false, "Unsupported coordinate system");
	}


template<CoordinateSystemConcept ForceSpace, CoordinateSystemConcept PositionSpace>
	inline void
	Body::apply_impulse (ForceMoments<ForceSpace> const& force_moments, SpaceLength<PositionSpace> const& position)
	{
		_world_space_applied_impulses.reset();

		ForceMoments<BodyCOM> body_space_force_moments;
		SpaceLength<BodyCOM> body_space_position;

		if constexpr (std::is_same_v<ForceSpace, WorldSpace>)
			body_space_force_moments = _placement.unbound_transform_to_body (force_moments);
		else if constexpr (std::is_same_v<ForceSpace, BodyCOM>)
			body_space_force_moments = force_moments;
		else
			static_assert (false, "Unsupported coordinate system");

		if constexpr (std::is_same_v<PositionSpace, WorldSpace>)
			body_space_position = _placement.bound_transform_to_body (position);
		else if constexpr (std::is_same_v<ForceSpace, BodyCOM>)
			body_space_position = position;
		else
			static_assert (false, "Unsupported coordinate system");

		apply_impulse (Wrench (body_space_force_moments, body_space_position));
	}


template<CoordinateSystemConcept Space>
	inline void
	Body::apply_impulse (Wrench<Space> const& wrench)
	{
		_world_space_applied_impulses.reset();

		if constexpr (std::is_same_v<Space, WorldSpace>)
		{
			// The resultant_force() assumes origin as the center of mass, so for Wrenches in WorldSpace coordinates
			// it gives incorrect results, since C.O.M. is rarely at the WorldSpace origin.
			// So first transform Wrench with its position to BodyCOM, only then calculate resultant force.
			_applied_impulses += resultant_force (_placement.bound_transform_to_body (wrench));
		}
		else if constexpr (std::is_same_v<Space, BodyCOM>)
			_applied_impulses += resultant_force (wrench);
		else
			static_assert (false, "Unsupported coordinate system");
	}


inline void
Body::reset_applied_impulses() noexcept
{
	_applied_impulses = ForceMoments<BodyCOM>();
	_world_space_applied_impulses.reset();
}


inline void
Body::set_shape (std::optional<Shape> const& shape)
{
	auto copy = shape;
	set_shape (std::move (shape));
}


template<CoordinateSystemConcept Space>
	inline SpaceLength<Space>
	Body::origin() const
	{
		if constexpr (std::is_same_v<Space, WorldSpace>)
			return _placement.bound_transform_to_base (_origin_placement.position());
		else if constexpr (std::is_same_v<Space, BodyCOM>)
			return _origin_placement.position();
		else if constexpr (std::is_same_v<Space, BodyOrigin>)
			return math::zero;
		else
			static_assert (false, "Unsupported coordinate system");
	}


template<CoordinateSystemConcept Space>
	inline void
	Body::translate (SpaceLength<Space> const& translation)
	{
		if constexpr (std::is_same_v<Space, WorldSpace>)
			_placement.translate_frame (translation);
		else if constexpr (std::is_same_v<Space, BodyCOM>)
			_placement.translate_frame (placement().body_to_base_rotation() * translation);
		else
			static_assert (false, "Unsupported coordinate system");
	}

} // namespace xf::rigid_body

#endif

