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

#ifndef XEFIS__SUPPORT__NATURE__VELOCITY_MOMENTS_H__INCLUDED
#define XEFIS__SUPPORT__NATURE__VELOCITY_MOMENTS_H__INCLUDED

// Standard:
#include <cstddef>

// Neutrino:
#include <neutrino/math/math.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>


namespace xf {

/**
 * VelocityMoments represents linear and angular velocity.
 * Angular velocity isn't normally called a moment, but it sounds consistent with Force- or Mass-moments.
 */
template<class Frame = void>
	struct VelocityMoments
	{
	  public:
		// Ctor
		constexpr
		VelocityMoments() = default;

		// Ctor
		constexpr
		VelocityMoments (SpaceVector<si::Velocity, Frame> const& velocity,
						 SpaceVector<si::AngularVelocity, Frame> const& angular_velocity);

		constexpr VelocityMoments&
		inline_add (VelocityMoments const& other, SpaceLength<Frame> const& arm);

		constexpr VelocityMoments&
		inline_subtract (VelocityMoments const& other, SpaceLength<Frame> const& arm);

		[[nodiscard]]
		constexpr SpaceVector<si::Velocity, Frame> const&
		velocity() const noexcept
			{ return _velocity; }

		constexpr void
		set_velocity (SpaceVector<si::Velocity, Frame> const& velocity)
			{ _velocity = velocity; }

		[[nodiscard]]
		constexpr SpaceVector<si::AngularVelocity, Frame> const&
		angular_velocity() const noexcept
			{ return _angular_velocity; }

		constexpr void
		set_angular_velocity (SpaceVector<si::AngularVelocity, Frame> const& angular_velocity)
			{ _angular_velocity = angular_velocity; }

	  public:
		static VelocityMoments<Frame>
		zero()
			{ return VelocityMoments<Frame> (math::zero, math::zero); }

	  private:
		SpaceVector<si::Velocity, Frame>		_velocity			{ 0_mps, 0_mps, 0_mps };
		SpaceVector<si::AngularVelocity, Frame>	_angular_velocity	{ 0_radps, 0_radps, 0_radps };
	};


template<class Frame>
	constexpr
	VelocityMoments<Frame>::VelocityMoments (SpaceVector<si::Velocity, Frame> const& velocity,
											 SpaceVector<si::AngularVelocity, Frame> const& angular_velocity):
		_velocity (velocity),
		_angular_velocity (angular_velocity)
	{ }


template<class Frame>
	constexpr VelocityMoments<Frame>&
	VelocityMoments<Frame>::inline_add (VelocityMoments const& other, SpaceLength<Frame> const& arm)
	{
		_velocity += other._velocity + tangential_velocity (_angular_velocity, arm);
		_angular_velocity += other._angular_velocity;
		return *this;
	}


template<class Frame>
	constexpr VelocityMoments<Frame>&
	VelocityMoments<Frame>::inline_subtract (VelocityMoments const& other, SpaceLength<Frame> const& arm)
	{
		_velocity -= other._velocity + tangential_velocity (_angular_velocity, arm);
		_angular_velocity -= other._angular_velocity;
		return *this;
	}


/*
 * Global functions
 */


template<class Frame>
	constexpr VelocityMoments<Frame>
	operator+ (VelocityMoments<Frame> a)
	{
		return a;
	}


template<class Frame>
	constexpr VelocityMoments<Frame>
	add (VelocityMoments<Frame> a, VelocityMoments<Frame> const& b, SpaceLength<Frame> const& arm)
	{
		return a.inline_add (b, arm);
	}


template<class Frame>
	constexpr VelocityMoments<Frame>
	operator- (VelocityMoments<Frame> a)
	{
		return {
			-a.velocity(),
			-a.angular_velocity(),
		};
	}


template<class Frame>
	constexpr VelocityMoments<Frame>
	subtract (VelocityMoments<Frame> a, VelocityMoments<Frame> const& b, SpaceLength<Frame> const& arm)
	{
		return a.inline_subtract (b, arm);
	}


template<class TargetFrame, class SourceFrame>
	constexpr VelocityMoments<TargetFrame>
	operator* (SpaceMatrix<double, TargetFrame, SourceFrame> const& transformation,
			   VelocityMoments<SourceFrame> const& velocity_moments)
	{
		return {
			transformation * velocity_moments.velocity(),
			transformation * velocity_moments.angular_velocity(),
		};
	}

} // namespace xf

#endif

