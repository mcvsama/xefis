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

#ifndef XEFIS__SUPPORT__SIMULATION__RIGID_BODY__SYSTEM_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__RIGID_BODY__SYSTEM_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/simulation/rigid_body/body.h>
#include <xefis/support/simulation/rigid_body/concepts.h>
#include <xefis/support/simulation/rigid_body/constraint.h>
#include <xefis/support/simulation/rigid_body/frame_precomputation.h>

// Neutrino:
#include <neutrino/noncopyable.h>

// Standard:
#include <cstddef>
#include <memory>
#include <set>
#include <string_view>
#include <type_traits>
#include <vector>


namespace xf::rigid_body {

class Group;

/**
 * A system of rigid bodies connected with constraints.
 */
class System: private nu::Noncopyable
{
  public:
	using FramePrecomputations	= std::vector<std::unique_ptr<BasicFramePrecomputation>>;
	using Groups				= std::set<std::unique_ptr<Group>>;
	using Bodies				= std::vector<std::unique_ptr<Body>>;
	using Constraints			= std::vector<std::unique_ptr<Constraint>>;
	using BodyPointers			= std::vector<Body*>;

  public:
	// Ctor
	System() = default;

	// Ctor
	System (Atmosphere const&);

	/**
	 * Add new body to the system.
	 */
	template<BodyConcept SpecificBody, class ...Args>
		SpecificBody&
		add (Args&&...);

	/**
	 * Add new body to the system.
	 */
	template<BodyConcept SpecificBody>
		SpecificBody&
		add (std::unique_ptr<SpecificBody>&&);

	/**
	 * Add new gravitating body to the system.
	 */
	template<BodyConcept SpecificBody, class ...Args>
		SpecificBody&
		add_gravitating (Args&&...);

	/**
	 * Add new gravitating body to the system.
	 */
	template<BodyConcept SpecificBody>
		SpecificBody&
		add_gravitating (std::unique_ptr<SpecificBody>&&);

	/**
	 * Add new constraint to the system.
	 */
	template<ConstraintConcept SpecificConstraint, class ...Args>
		SpecificConstraint&
		add (Args&&...);

	/**
	 * Add new constraint to the system.
	 */
	template<ConstraintConcept SpecificConstraint>
		SpecificConstraint&
		add (std::unique_ptr<SpecificConstraint>&&);

	/**
	 * Add new BasicFramePrecomputation to the system.
	 */
	template<BasicFramePrecomputationConcept SpecificFramePrecomputation, class ...Args>
		SpecificFramePrecomputation&
		add (Args&&...);

	/**
	 * Add new BasicFramePrecomputation to the system.
	 */
	template<BasicFramePrecomputationConcept SpecificFramePrecomputation>
		SpecificFramePrecomputation&
		add (std::unique_ptr<SpecificFramePrecomputation>&&);

	/**
	 * Return atmosphere model to use by bodies or nullptr if it wasn't set.
	 */
	[[nodiscard]]
	Atmosphere const*
	atmosphere() const noexcept
		{ return _atmosphere; }

	/**
	 * Set atmosphere model to use by bodies or set it to nullptr.
	 */
	void
	set_atmosphere (Atmosphere const* atmosphere) noexcept
		{ _atmosphere = atmosphere; }

	/**
	 * Make a group belonging to this system.
	 */
	[[nodiscard]]
	Group&
	make_group (std::string_view label = {});

	/**
	 * Return sequence of body groups.
	 */
	[[nodiscard]]
	Groups const&
	groups() const
		{ return _groups; }

	/**
	 * Return sequence of simulated bodies.
	 */
	[[nodiscard]]
	Bodies const&
	bodies() const noexcept
		{ return _bodies; }

	/**
	 * Return sequence of simulated gravitating bodies.
	 */
	[[nodiscard]]
	BodyPointers const&
	gravitating_bodies() const noexcept
		{ return _gravitating_bodies; }

	/**
	 * Return sequence of simulated non-gravitating bodies.
	 */
	[[nodiscard]]
	BodyPointers const&
	non_gravitating_bodies() const noexcept
		{ return _non_gravitating_bodies; }

	/**
	 * Return sequence of body constraints.
	 */
	[[nodiscard]]
	Constraints const&
	constraints() const noexcept
		{ return _constraints; }

	/**
	 * Return sequence of frame precomputation objects.
	 */
	[[nodiscard]]
	FramePrecomputations const&
	frame_precomputations() const noexcept
		{ return _frame_precomputations; }

	/**
	 * Calculate total translational energy of all bodies in the system.
	 */
	[[nodiscard]]
	si::Energy
	translational_kinetic_energy() const;

	/**
	 * Calculate total rotational energy of all bodies in the system.
	 */
	[[nodiscard]]
	si::Energy
	rotational_kinetic_energy() const;

	/**
	 * Rotate whole system about space origin by provided rotation quaternion.
	 */
	void
	rotate_about_world_origin (RotationQuaternion<WorldSpace> const&);

	/**
	 * Translate whole system by given vector.
	 */
	void
	translate (SpaceLength<WorldSpace> const&);

	/**
	 * Apply given Baumgarte stabilization factor to all constraints.
	 */
	void
	set_baumgarte_factor (double factor) noexcept;

	/**
	 * Apply given Constraint Force Mixing factor to all constraints.
	 */
	void
	set_constraint_force_mixing_factor (double factor) noexcept;

	/**
	 * Apply velocity damping factors to all constraints.
	 */
	void
	set_friction_factor (double factor) noexcept;

  private:
	FramePrecomputations	_frame_precomputations;
	Groups					_groups;
	Bodies					_bodies;
	Constraints				_constraints;
	// Bodies acting on all bodies gravitationally (contains pointers to elements in _bodies):
	BodyPointers			_gravitating_bodies;
	BodyPointers			_non_gravitating_bodies;
	Atmosphere const*		_atmosphere { nullptr };
};

} // namespace xf::rigid_body


#include <xefis/support/simulation/rigid_body/group.h>


namespace xf::rigid_body {

inline
System::System (Atmosphere const& atmosphere):
	_atmosphere (&atmosphere)
{ }


template<BodyConcept SpecificBody, class ...Args>
	inline SpecificBody&
	System::add (Args&& ...args)
	{
		return add (std::make_unique<SpecificBody> (std::forward<Args> (args)...));
	}


template<BodyConcept SpecificBody>
	inline SpecificBody&
	System::add (std::unique_ptr<SpecificBody>&& body)
	{
		_bodies.push_back (std::move (body));
		_non_gravitating_bodies.push_back (_bodies.back().get());
		return static_cast<SpecificBody&> (*_bodies.back());
	}


template<BodyConcept SpecificBody, class ...Args>
	inline SpecificBody&
	System::add_gravitating (Args&& ...args)
	{
		return add_gravitating (std::make_unique<SpecificBody> (std::forward<Args> (args)...));
	}


template<BodyConcept SpecificBody>
	inline SpecificBody&
	System::add_gravitating (std::unique_ptr<SpecificBody>&& body)
	{
		_bodies.push_back (std::move (body));
		_gravitating_bodies.push_back (_bodies.back().get());
		return static_cast<SpecificBody&> (*_bodies.back());
	}


template<ConstraintConcept SpecificConstraint, class ...Args>
	inline SpecificConstraint&
	System::add (Args&& ...args)
	{
		return add (std::make_unique<SpecificConstraint> (std::forward<Args> (args)...));
	}


template<ConstraintConcept SpecificConstraint>
	inline SpecificConstraint&
	System::add (std::unique_ptr<SpecificConstraint>&& constraint)
	{
		_constraints.push_back (std::move (constraint));
		return static_cast<SpecificConstraint&> (*_constraints.back());
	}


template<BasicFramePrecomputationConcept SpecificFramePrecomputation, class ...Args>
	inline SpecificFramePrecomputation&
	System::add (Args&& ...args)
	{
		return add (std::make_unique<SpecificFramePrecomputation> (std::forward<Args> (args)...));
	}


template<BasicFramePrecomputationConcept SpecificFramePrecomputation>
	inline SpecificFramePrecomputation&
	System::add (std::unique_ptr<SpecificFramePrecomputation>&& frame_cache)
	{
		_frame_precomputations.push_back (std::move (frame_cache));
		return static_cast<SpecificFramePrecomputation&> (*_frame_precomputations.back());
	}

} // namespace xf::rigid_body

#endif

