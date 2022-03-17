/* vim:ts=4
 *
 * Copyleft 2012…2018  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__NATURE__WRENCH_H__INCLUDED
#define XEFIS__SUPPORT__NATURE__WRENCH_H__INCLUDED

// Standard:
#include <cstddef>

// Lib:
#include <boost/range.hpp>

// Neutrino:
#include <neutrino/math/math.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/nature/force_moments.h>


namespace xf {

template<class Space = void>
	class Wrench: public ForceMoments<Space>
	{
	  public:
		// Ctor
		constexpr
		Wrench() = default;

		// Ctor
		explicit constexpr
		Wrench (ForceMoments<Space> const&);

		// Ctor
		constexpr
		Wrench (ForceMoments<Space> const&, SpaceVector<si::Length, Space> const& position);

		// Ctor
		constexpr
		Wrench (SpaceVector<si::Force, Space> const& force, SpaceVector<si::Torque, Space> const& torque, SpaceVector<si::Length, Space> const& position);

		/**
		 * Force's root.
		 */
		[[nodiscard]]
		SpaceVector<si::Length, Space> const&
		position() const noexcept
			{ return _position; }

		/**
		 * Set force's root.
		 */
		void
		set_position (SpaceVector<si::Length, Space> const& position)
			{ _position = position; }

	  private:
		SpaceVector<si::Length, Space>	_position	{ math::zero };
	};


template<class Space>
	constexpr
	Wrench<Space>::Wrench (ForceMoments<Space> const& force_torque):
		ForceMoments<Space> (force_torque)
	{ }


template<class Space>
	constexpr
	Wrench<Space>::Wrench (ForceMoments<Space> const& force_torque, SpaceVector<si::Length, Space> const& position):
		ForceMoments<Space> (force_torque),
		_position (position)
	{ }


template<class Space>
	constexpr
	Wrench<Space>::Wrench (SpaceVector<si::Force, Space> const& force, SpaceVector<si::Torque, Space> const& torque, SpaceVector<si::Length, Space> const& position):
		ForceMoments<Space> (force, torque),
		_position (position)
	{ }


/*
 * Global functions
 */


/**
 * This changes the application position of the force/torque. Doesn't recalculate anything.
 */
template<class Space>
	inline Wrench<Space>
	operator+ (Wrench<Space> wrench, SpaceLength<Space> const& offset)
	{
		wrench.set_position (wrench.position() + offset);
		return wrench;
	}


/**
 * This changes the application position of the force/torque. Doesn't recalculate anything.
 */
template<class Space>
	inline Wrench<Space>
	operator- (Wrench<Space> wrench, SpaceLength<Space> const& offset)
	{
		wrench.set_position (wrench.position() - offset);
		return wrench;
	}


template<class TargetSpace = void, class SourceSpace = TargetSpace>
	inline Wrench<TargetSpace>
	operator* (RotationMatrix<TargetSpace, SourceSpace> const& rotation, Wrench<SourceSpace> const& wrench)
	{
		return {
			rotation * static_cast<ForceMoments<SourceSpace>> (wrench),
			rotation * wrench.position(),
		};
	}


/**
 * Calculate equivalent force and torque about the origin (not necessarily a center of mass).
 *
 * Warning: if you have non-BodySpace Wrench, transform it first to BodySpace
 * before using resultant_force(), because space origin is assumed to be center-of-mass.
 */
template<class Space>
	inline ForceMoments<Space>
	resultant_force (Wrench<Space> const& wrench)
	{
		ForceMoments<Space> result (wrench);
		result.set_torque (result.torque() + cross_product (wrench.position(), wrench.force()));
		return result;
	}


/**
 * Calculate total equivalent force and torque about the origin
 * from a set of forces and torques at various points in space.
 */
template<class Space, class WrenchIterator>
	inline ForceMoments<Space>
	resultant_force (WrenchIterator begin, WrenchIterator end)
	{
		ForceMoments<Space> total;

		for (auto j = begin; j != end; ++j)
		{
			auto const eq = resultant_force (*j);
			total.torque += eq.torque;
			total.force += eq.force;
		}

		return total;
	}

} // namespace xf

#endif

