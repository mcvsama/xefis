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

#ifndef XEFIS__SUPPORT__SIMULATION__N_BODY__BODY_SHAPE_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__N_BODY__BODY_SHAPE_H__INCLUDED

// Standard:
#include <cstddef>

// Lib:
#include <lib/math/math.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/space.h>
#include <xefis/support/nature/physics.h>
#include <xefis/support/simulation/n_body/body_part.h>


namespace xf::sim {

class BodyShape
{
  public:
	/**
	 * Add a part to the shape.
	 */
	template<class Part>
		requires (std::is_base_of_v<BasicBodyPart, Part>)
		Part&
		add (std::unique_ptr<Part>&&);

	/**
	 * Vector of parts.
	 */
	std::vector<std::unique_ptr<BasicBodyPart>> const&
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
	 * Moment of inertia tensor about the center of mass.
	 */
	[[nodiscard]]
	SpaceMatrix<si::MomentOfInertia, AirframeFrame> const&
	moment_of_inertia() const noexcept
		{ return _total_moment_of_inertia; }

	/**
	 * Inversed moment of inertia.
	 */
	[[nodiscard]]
	SpaceMatrix<si::MomentOfInertia, AirframeFrame>::InversedMatrix const&
	inversed_moment_of_inertia() const noexcept
		{ return _inversed_total_moment_of_inertia; }

	// TODO need a method to recompute total MOI, COM, angular velocity, etc. when part's position changes

  private:
	/**
	 * Recompute total mass and total moment of inertia.
	 */
	void
	recompute();

  private:
	std::vector<std::unique_ptr<BasicBodyPart>>						_parts;
	// Stuff calculated from parts:
	si::Mass														_total_mass;
	SpaceMatrix<si::MomentOfInertia, AirframeFrame>					_total_moment_of_inertia;
	SpaceMatrix<si::MomentOfInertia, AirframeFrame>::InversedMatrix	_inversed_total_moment_of_inertia;
};


template<class Part>
	requires (std::is_base_of_v<BasicBodyPart, Part>)
	inline Part&
	BodyShape::add (std::unique_ptr<Part>&& part)
	{
		_parts.push_back (std::move (part));
		recompute();
		return static_cast<Part&> (*_parts.back());
	}

} // namespace xf::sim

#endif

