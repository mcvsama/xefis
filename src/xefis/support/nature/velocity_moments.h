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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>

// Neutrino:
#include <neutrino/math/math.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * VelocityMoments represents linear and angular velocity.
 * Angular velocity isn't normally called a moment, but it sounds consistent with Force- or Mass-moments.
 */
template<math::CoordinateSystem Space = void>
	class VelocityMoments
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
		inplace_add (VelocityMoments const& other, SpaceLength<Space> const& arm);

		constexpr VelocityMoments&
		inplace_subtract (VelocityMoments const& other, SpaceLength<Space> const& arm);

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


template<math::CoordinateSystem Space>
	constexpr
	VelocityMoments<Space>::VelocityMoments (SpaceVector<si::Velocity, Space> const& velocity,
											 SpaceVector<si::AngularVelocity, Space> const& angular_velocity):
		_velocity (velocity),
		_angular_velocity (angular_velocity)
	{ }


template<math::CoordinateSystem Space>
	constexpr VelocityMoments<Space>&
	VelocityMoments<Space>::inplace_add (VelocityMoments const& other, SpaceLength<Space> const& arm)
	{
		_velocity += other._velocity + tangential_velocity (_angular_velocity, arm);
		_angular_velocity += other._angular_velocity;
		return *this;
	}


template<math::CoordinateSystem Space>
	constexpr VelocityMoments<Space>&
	VelocityMoments<Space>::inplace_subtract (VelocityMoments const& other, SpaceLength<Space> const& arm)
	{
		_velocity -= other._velocity + tangential_velocity (_angular_velocity, arm);
		_angular_velocity -= other._angular_velocity;
		return *this;
	}


/*
 * Global functions
 */


template<math::CoordinateSystem Space>
	constexpr VelocityMoments<Space>
	operator+ (VelocityMoments<Space> a)
	{
		return a;
	}


template<math::CoordinateSystem Space>
	constexpr VelocityMoments<Space>
	operator+ (VelocityMoments<Space> a, VelocityMoments<Space> b)
	{
		return {
			a.velocity() + b.velocity(),
			a.angular_velocity() + b.angular_velocity(),
		};
	}


template<math::CoordinateSystem Space>
	constexpr VelocityMoments<Space>
	add (VelocityMoments<Space> a, VelocityMoments<Space> const& b, SpaceLength<Space> const& arm)
	{
		return a.inplace_add (b, arm);
	}


template<math::CoordinateSystem Space>
	constexpr VelocityMoments<Space>
	operator- (VelocityMoments<Space> a)
	{
		return {
			-a.velocity(),
			-a.angular_velocity(),
		};
	}


template<math::CoordinateSystem Space>
	constexpr VelocityMoments<Space>
	operator- (VelocityMoments<Space> a, VelocityMoments<Space> b)
	{
		return {
			a.velocity() - b.velocity(),
			a.angular_velocity() - b.angular_velocity(),
		};
	}


template<math::CoordinateSystem Space>
	constexpr VelocityMoments<Space>
	subtract (VelocityMoments<Space> a, VelocityMoments<Space> const& b, SpaceLength<Space> const& arm)
	{
		return a.inplace_subtract (b, arm);
	}


template<math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	constexpr VelocityMoments<TargetSpace>
	operator* (RotationQuaternion<TargetSpace, SourceSpace> const& rotation,
			   VelocityMoments<SourceSpace> const& velocity_moments)
	{
		return {
			rotation * velocity_moments.velocity(),
			rotation * velocity_moments.angular_velocity(),
		};
	}

} // namespace xf

#endif

