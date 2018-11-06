/* vim:ts=4
 *
 * Copyleft 2008…2018  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__BODY_SHAPE_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__BODY_SHAPE_H__INCLUDED

// Standard:
#include <cstddef>

// Lib:
#include <lib/math/math.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/space.h>
#include <xefis/support/nature/physics.h>
#include <xefis/support/simulation/body_part.h>


namespace xf::sim {

class BodyShape
{
  public:
	/**
	 * Add a part to the shape.
	 */
	template<class Part>
		requires (std::is_base_of_v<BodyPart, Part>)
		Part&
		add (std::unique_ptr<Part>&&);

	std::vector<std::unique_ptr<BodyPart>> const&
	parts() const
		{ return _parts; }

	/**
	 * Rest mass.
	 */
	[[nodiscard]]
	si::Mass
	mass() const noexcept
		{ return _total_mass; }

	/**
	 * Set new rest mass.
	 * TODO deprecate, calculate mass from parts
	 */
	void
	set_mass (si::Mass mass)
		{ _total_mass = mass; }

	/**
	 * Moment of inertia tensor about the center of mass.
	 */
	[[nodiscard]]
	SpaceMatrix<si::MomentOfInertia, BodyFrame> const&
	moment_of_inertia() const noexcept
		{ return _total_moment_of_inertia; }

	/**
	 * Inversed moment of inertia.
	 */
	[[nodiscard]]
	SpaceMatrix<si::MomentOfInertia, BodyFrame>::InversedMatrix const&
	inversed_moment_of_inertia() const noexcept
		{ return _inversed_total_moment_of_inertia; }

	/**
	 * Set new moment of inertia tensor.
	 * TODO deprecate, calculate mass from parts
	 */
	void
	set_moment_of_inertia (SpaceMatrix<si::MomentOfInertia, BodyFrame> const&);

  private:
	std::vector<std::unique_ptr<BodyPart>>						_parts;
	// Stuff calculated from parts:
	si::Mass													_total_mass;
	SpaceMatrix<si::MomentOfInertia, BodyFrame>					_total_moment_of_inertia;
	SpaceMatrix<si::MomentOfInertia, BodyFrame>::InversedMatrix	_inversed_total_moment_of_inertia;
};


template<class Part>
	requires (std::is_base_of_v<BodyPart, Part>)
	Part&
	BodyShape::add (std::unique_ptr<Part>&& part)
	{
		_parts.push_back (std::move (part));
		return static_cast<Part&> (*_parts.back());
	}

} // namespace xf::sim

#endif

