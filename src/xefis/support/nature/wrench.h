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

template<class Frame = void>
	class Wrench: public ForceMoments<Frame>
	{
	  public:
		// Ctor
		constexpr
		Wrench() = default;

		// Ctor
		explicit constexpr
		Wrench (ForceMoments<Frame> const&);

		// Ctor
		constexpr
		Wrench (ForceMoments<Frame> const&, SpaceVector<si::Length, Frame> const& position);

		// Ctor
		constexpr
		Wrench (SpaceVector<si::Force, Frame> const& force, SpaceVector<si::Torque, Frame> const& torque, SpaceVector<si::Length, Frame> const& position);

		/**
		 * Force's root.
		 */
		[[nodiscard]]
		SpaceVector<si::Length, Frame> const&
		position() const noexcept
			{ return _position; }

		/**
		 * Set force's root.
		 */
		void
		set_position (SpaceVector<si::Length, Frame> const& position)
			{ _position = position; }

	  private:
		SpaceVector<si::Length, Frame>	_position	{ math::zero };
	};


template<class Frame>
	constexpr
	Wrench<Frame>::Wrench (ForceMoments<Frame> const& force_torque):
		ForceMoments<Frame> (force_torque)
	{ }


template<class Frame>
	constexpr
	Wrench<Frame>::Wrench (ForceMoments<Frame> const& force_torque, SpaceVector<si::Length, Frame> const& position):
		ForceMoments<Frame> (force_torque),
		_position (position)
	{ }


template<class Frame>
	constexpr
	Wrench<Frame>::Wrench (SpaceVector<si::Force, Frame> const& force, SpaceVector<si::Torque, Frame> const& torque, SpaceVector<si::Length, Frame> const& position):
		ForceMoments<Frame> (force, torque),
		_position (position)
	{ }


/*
 * Global functions
 */


/**
 * This changes the application position of the force/torque. Doesn't recalculate anything.
 */
template<class Frame>
	inline Wrench<Frame>
	operator+ (Wrench<Frame> wrench, SpaceLength<Frame> const& offset)
	{
		wrench.set_position (wrench.position() + offset);
		return wrench;
	}


/**
 * This changes the application position of the force/torque. Doesn't recalculate anything.
 */
template<class Frame>
	inline Wrench<Frame>
	operator- (Wrench<Frame> wrench, SpaceLength<Frame> const& offset)
	{
		wrench.set_position (wrench.position() - offset);
		return wrench;
	}


template<class TargetFrame = void, class SourceFrame = TargetFrame>
	inline Wrench<TargetFrame>
	operator* (RotationMatrix<TargetFrame, SourceFrame> const& rotation, Wrench<SourceFrame> const& wrench)
	{
		return {
			rotation * static_cast<ForceMoments<SourceFrame>> (wrench),
			rotation * wrench.position(),
		};
	}


/**
 * Calculate equivalent force and torque about the origin (not necessarily a center of mass).
 *
 * Warning: if you have non-BodySpace Wrench, transform it first to BodySpace
 * before using resultant_force(), because space origin is assumed to be center-of-mass.
 */
template<class Frame>
	inline ForceMoments<Frame>
	resultant_force (Wrench<Frame> const& wrench)
	{
		ForceMoments<Frame> result (wrench);
		result.set_torque (result.torque() + cross_product (wrench.position(), wrench.force()));
		return result;
	}


/**
 * Calculate total equivalent force and torque about the origin
 * from a set of forces and torques at various points in space.
 */
template<class Frame, class WrenchIterator>
	inline ForceMoments<Frame>
	resultant_force (WrenchIterator begin, WrenchIterator end)
	{
		ForceMoments<Frame> total;

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

