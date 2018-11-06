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

#ifndef XEFIS__SUPPORT__MATH__PHYSICS_H__INCLUDED
#define XEFIS__SUPPORT__MATH__PHYSICS_H__INCLUDED

// Standard:
#include <cstddef>

// Lib:
#include <boost/range.hpp>
#include <lib/math/math.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/space.h>


namespace xf {

/**
 * ForceTorque is a Wrench with force application point at the center of mass.
 */
// TODO move to sep. file
template<class Frame>
	struct ForceTorque
	{
	  public:
		// Ctor
		constexpr
		ForceTorque() = default;

		// Ctor
		constexpr
		ForceTorque (SpaceVector<si::Force, Frame> const& force, SpaceVector<si::Torque, Frame> const& torque);

		constexpr ForceTorque&
		operator+= (ForceTorque const& other);

		constexpr ForceTorque&
		operator-= (ForceTorque const& other);

		[[nodiscard]]
		constexpr SpaceVector<si::Force, Frame> const&
		force() const noexcept
			{ return _force; }

		constexpr void
		set_force (SpaceVector<si::Force, Frame> const& force)
			{ _force = force; }

		[[nodiscard]]
		constexpr SpaceVector<si::Torque, Frame> const&
		torque() const noexcept
			{ return _torque; }

		constexpr void
		set_torque (SpaceVector<si::Torque, Frame> const& torque)
			{ _torque = torque; }

	  private:
		SpaceVector<si::Force, Frame>	_force	{ 0_N, 0_N, 0_N };
		SpaceVector<si::Torque, Frame>	_torque	{ 0_Nm, 0_Nm, 0_Nm };
	};


template<class Frame>
	constexpr
	ForceTorque<Frame>::ForceTorque (SpaceVector<si::Force, Frame> const& force, SpaceVector<si::Torque, Frame> const& torque):
		_force (force),
		_torque (torque)
	{ }


template<class Frame>
	constexpr ForceTorque<Frame>&
	ForceTorque<Frame>::operator+= (ForceTorque const& other)
	{
		_force += other._force;
		_torque += other._torque;
		return *this;
	}


template<class Frame>
	constexpr ForceTorque<Frame>&
	ForceTorque<Frame>::operator-= (ForceTorque const& other)
	{
		_force -= other._force;
		_torque -= other._torque;
		return *this;
	}


template<class TargetFrame, class SourceFrame>
	constexpr ForceTorque<TargetFrame>
	operator* (SpaceMatrix<double, TargetFrame, SourceFrame> const& transformation,
			   ForceTorque<SourceFrame> const& force_torque)
	{
		return {
			transformation * force_torque.force(),
			transformation * force_torque.torque(),
		};
	}


/*
 * Global functions
 */


template<class Frame>
	constexpr ForceTorque<Frame>
	operator+ (ForceTorque<Frame> const& a, ForceTorque<Frame> const& b)
	{
		ForceTorque<Frame> sum;
		sum.set_force (a.force() + b.force());
		sum.set_torque (a.torque() + b.torque());
		return sum;
	}


template<class Frame>
	constexpr ForceTorque<Frame>
	operator- (ForceTorque<Frame> const& a, ForceTorque<Frame> const& b)
	{
		ForceTorque<Frame> sum;
		sum.set_force (a.force() - b.force());
		sum.set_torque (a.torque() - b.torque());
		return sum;
	}


// TODO move to sep. file
template<class Frame>
	class Wrench: public ForceTorque<Frame>
	{
	  public:
		// Ctor
		constexpr
		Wrench() = default;

		// Ctor
		explicit constexpr
		Wrench (ForceTorque<Frame> const&);

		// Ctor
		explicit constexpr
		Wrench (ForceTorque<Frame> const&, SpaceVector<si::Length, Frame> const& position);

		// Ctor
		explicit constexpr
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
	Wrench<Frame>::Wrench (ForceTorque<Frame> const& force_torque):
		ForceTorque<Frame> (force_torque)
	{ }


template<class Frame>
	constexpr
	Wrench<Frame>::Wrench (ForceTorque<Frame> const& force_torque, SpaceVector<si::Length, Frame> const& position):
		ForceTorque<Frame> (force_torque),
		_position (position)
	{ }


template<class Frame>
	constexpr
	Wrench<Frame>::Wrench (SpaceVector<si::Force, Frame> const& force, SpaceVector<si::Torque, Frame> const& torque, SpaceVector<si::Length, Frame> const& position):
		ForceTorque<Frame> (force, torque),
		_position (position)
	{ }


/*
 * Global functions
 */


template<class Frame, class PointMassIterator>
	inline SpaceVector<si::Length, Frame>
	center_of_gravity (PointMassIterator masses_begin, PointMassIterator masses_end)
	{
		SpaceVector<decltype (si::Length{} * si::Mass{}), Frame> center (math::zero);
		si::Mass total_mass = 0_kg;

		for (auto const& mx: boost::make_iterator_range (masses_begin, masses_end))
		{
			auto const r = std::get<SpaceVector<si::Length, Frame>> (mx);
			auto const m = std::get<si::Mass> (mx);

			center += m * r;
			total_mass += m;
		}

		return 1.0 / total_mass * center;
	}


template<class Frame, class PointMassIterator>
	inline SpaceMatrix<si::MomentOfInertia, Frame>
	moment_of_inertia (PointMassIterator masses_begin, PointMassIterator masses_end)
	{
		SpaceMatrix<si::MomentOfInertia, Frame> sum (math::zero);

		for (auto const& mx: boost::make_iterator_range (masses_begin, masses_end))
		{
			auto const r = std::get<SpaceVector<si::Length, Frame>> (mx);
			auto const m = std::get<si::Mass> (mx);
			auto const u = SpaceMatrix<double, Frame> (math::unit);

			sum += m * (u * (~r * r) - r * ~r);
		}

		return sum;
	}


/**
 * Move mass distributions so that position [0, 0, 0] is in center of gravity.
 * Return vector by which all masses were moved.
 */
template<class Frame, class PointMassIterator>
	inline SpaceVector<si::Length, Frame>
	move_to_center_of_gravity (PointMassIterator begin, PointMassIterator end)
	{
		auto const cog_correction = -center_of_gravity<Frame> (begin, end);

		for (auto& particle: boost::make_iterator_range (begin, end))
			std::get<SpaceVector<si::Length, Frame>> (particle) += cog_correction;

		return cog_correction;
	}


/**
 * Return sum of point masses.
 */
template<class PointMassIterator>
	inline si::Mass
	total_mass (PointMassIterator begin, PointMassIterator end)
	{
		auto sum = 0_kg;

		for (auto& particle: boost::make_iterator_range (begin, end))
			sum += std::get<si::Mass> (particle);

		return sum;
	}


/**
 * Calculate equivalent force and torque about the center of mass.
 */
template<class Frame>
	inline ForceTorque<Frame>
	resultant_force (Wrench<Frame> const& wrench)
	{
		ForceTorque<Frame> result (wrench);
		result.set_torque (result.torque() + cross_product (wrench.position(), wrench.force()));
		// TODO orientation of position is not necessarily same as orientation of force??????????????????//////
		return result;
	}


/**
 * Calculate total equivalent force and torque about the center of mass
 * from a set of forces and torques at various points in space.
 */
template<class Frame, class WrenchIterator>
	inline ForceTorque<Frame>
	resultant_force (WrenchIterator begin, WrenchIterator end)
	{
		ForceTorque<Frame> total;

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

