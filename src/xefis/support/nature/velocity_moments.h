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
template<class Space = void>
	struct VelocityMoments
	{
	  public:
		// Ctor
		constexpr
		VelocityMoments() = default;

		// Ctor
		constexpr
		VelocityMoments (SpaceVector<si::Velocity, Space> const& velocity,
						 SpaceVector<si::AngularVelocity, Space> const& angular_velocity);

		constexpr VelocityMoments&
		inline_add (VelocityMoments const& other, SpaceLength<Space> const& arm);

		constexpr VelocityMoments&
		inline_subtract (VelocityMoments const& other, SpaceLength<Space> const& arm);

		[[nodiscard]]
		constexpr SpaceVector<si::Velocity, Space> const&
		velocity() const noexcept
			{ return _velocity; }

		constexpr void
		set_velocity (SpaceVector<si::Velocity, Space> const& velocity)
			{ _velocity = velocity; }

		[[nodiscard]]
		constexpr SpaceVector<si::AngularVelocity, Space> const&
		angular_velocity() const noexcept
			{ return _angular_velocity; }

		constexpr void
		set_angular_velocity (SpaceVector<si::AngularVelocity, Space> const& angular_velocity)
			{ _angular_velocity = angular_velocity; }

	  public:
		static VelocityMoments<Space>
		zero()
			{ return VelocityMoments<Space> (math::zero, math::zero); }

	  private:
		SpaceVector<si::Velocity, Space>		_velocity			{ 0_mps, 0_mps, 0_mps };
		SpaceVector<si::AngularVelocity, Space>	_angular_velocity	{ 0_radps, 0_radps, 0_radps };
	};


template<class Space>
	constexpr
	VelocityMoments<Space>::VelocityMoments (SpaceVector<si::Velocity, Space> const& velocity,
											 SpaceVector<si::AngularVelocity, Space> const& angular_velocity):
		_velocity (velocity),
		_angular_velocity (angular_velocity)
	{ }


template<class Space>
	constexpr VelocityMoments<Space>&
	VelocityMoments<Space>::inline_add (VelocityMoments const& other, SpaceLength<Space> const& arm)
	{
		_velocity += other._velocity + tangential_velocity (_angular_velocity, arm);
		_angular_velocity += other._angular_velocity;
		return *this;
	}


template<class Space>
	constexpr VelocityMoments<Space>&
	VelocityMoments<Space>::inline_subtract (VelocityMoments const& other, SpaceLength<Space> const& arm)
	{
		_velocity -= other._velocity + tangential_velocity (_angular_velocity, arm);
		_angular_velocity -= other._angular_velocity;
		return *this;
	}


/*
 * Global functions
 */


template<class Space>
	constexpr VelocityMoments<Space>
	operator+ (VelocityMoments<Space> a)
	{
		return a;
	}


template<class Space>
	constexpr VelocityMoments<Space>
	add (VelocityMoments<Space> a, VelocityMoments<Space> const& b, SpaceLength<Space> const& arm)
	{
		return a.inline_add (b, arm);
	}


template<class Space>
	constexpr VelocityMoments<Space>
	operator- (VelocityMoments<Space> a)
	{
		return {
			-a.velocity(),
			-a.angular_velocity(),
		};
	}


template<class Space>
	constexpr VelocityMoments<Space>
	subtract (VelocityMoments<Space> a, VelocityMoments<Space> const& b, SpaceLength<Space> const& arm)
	{
		return a.inline_subtract (b, arm);
	}


template<class TargetSpace, class SourceSpace>
	constexpr VelocityMoments<TargetSpace>
	operator* (SpaceMatrix<double, TargetSpace, SourceSpace> const& transformation,
			   VelocityMoments<SourceSpace> const& velocity_moments)
	{
		return {
			transformation * velocity_moments.velocity(),
			transformation * velocity_moments.angular_velocity(),
		};
	}

} // namespace xf

#endif

