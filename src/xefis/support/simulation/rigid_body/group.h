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

#ifndef XEFIS__SUPPORT__SIMULATION__RIGID_BODY__GROUP_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__RIGID_BODY__GROUP_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/simulation/rigid_body/concepts.h>

// Neutrino:
#include <neutrino/noncopyable.h>

// Standard:
#include <cstddef>
#include <vector>


namespace xf::rigid_body {

class Body;
class System;

/**
 * A set of bodies that can be translated/rotated as a whole.
 * Group does not own bodies, rigid body System does.
 */
class Group
{
  public:
	// Ctor
	explicit
	Group (System& system);

	/**
	 * Add new body to the group and the system.
	 */
	template<BodyConcept SpecificBody, class ...Args>
		SpecificBody&
		add (Args&&...);

	/**
	 * Add new body to the group and the system.
	 */
	template<BodyConcept SpecificBody>
		SpecificBody&
		add (std::unique_ptr<SpecificBody>&&);

	/**
	 * Add new gravitating body to the group and the system.
	 */
	template<BodyConcept SpecificBody, class ...Args>
		SpecificBody&
		add_gravitating (Args&&...);

	/**
	 * Add new gravitating body to the group and the system.
	 */
	template<BodyConcept SpecificBody>
		SpecificBody&
		add_gravitating (std::unique_ptr<SpecificBody>&&);

	/**
	 * Rotate the body about world space origin by provided rotation matrix.
	 */
	void
	rotate_about_world_origin (RotationMatrix<WorldSpace> const&);

	/**
	 * Rotate the body about given point.
	 */
	void
	rotate_about (SpaceLength<WorldSpace> const& about_point, RotationMatrix<WorldSpace> const&);

	/**
	 * Translate the body by given vector.
	 */
	void
	translate (SpaceLength<WorldSpace> const&);

	/**
	 * Calculate total kinetic energy of the group in WorldSpace frame of reference.
	 */
	si::Energy
	kinetic_energy() const;

  private:
	System*				_system;
	std::vector<Body*>	_bodies;
};

} // namespace xf::rigid_body


#include <xefis/support/simulation/rigid_body/system.h>


namespace xf::rigid_body {

inline
Group::Group (System& system):
	_system (&system)
{ }


template<BodyConcept SpecificBody, class ...Args>
	inline SpecificBody&
	Group::add (Args&& ...args)
	{
		return add (std::make_unique<SpecificBody> (std::forward<Args> (args)...));
	}


template<BodyConcept SpecificBody>
	inline SpecificBody&
	Group::add (std::unique_ptr<SpecificBody>&& body)
	{
		auto& added_body = _system->add (std::move (body));
		_bodies.push_back (&added_body);
		return added_body;
	}


template<BodyConcept SpecificBody, class ...Args>
	inline SpecificBody&
	Group::add_gravitating (Args&& ...args)
	{
		return add_gravitating (std::make_unique<SpecificBody> (std::forward<Args> (args)...));
	}


template<BodyConcept SpecificBody>
	inline SpecificBody&
	Group::add_gravitating (std::unique_ptr<SpecificBody>&& body)
	{
		auto& added_body = _system->add (std::move (body));
		_bodies.push_back (&added_body);
		return added_body;
	}

} // namespace xf::rigid_body

#endif

