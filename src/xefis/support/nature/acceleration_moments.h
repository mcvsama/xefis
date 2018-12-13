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

#ifndef XEFIS__SUPPORT__NATURE__ACCELERATION_MOMENTS_H__INCLUDED
#define XEFIS__SUPPORT__NATURE__ACCELERATION_MOMENTS_H__INCLUDED

// Standard:
#include <cstddef>

// Neutrino:
#include <neutrino/math/math.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>


namespace xf {

/**
 * AccelerationMoments represents basic moments of acceleration:
 *   • 0th moment = linear acceleration,
 *   • 1st moment = angular acceleration.
 */
template<class Frame = void>
	struct AccelerationMoments
	{
	  public:
		// Ctor
		constexpr
		AccelerationMoments() = default;

		// Ctor
		constexpr
		AccelerationMoments (SpaceVector<si::Acceleration, Frame> const& acceleration,
							 SpaceVector<si::AngularAcceleration, Frame> const& angular_acceleration);

		constexpr AccelerationMoments&
		operator+= (AccelerationMoments const& other);

		constexpr AccelerationMoments&
		operator-= (AccelerationMoments const& other);

		[[nodiscard]]
		constexpr SpaceVector<si::Acceleration, Frame> const&
		acceleration() const noexcept
			{ return _acceleration; }

		constexpr void
		set_acceleration (SpaceVector<si::Acceleration, Frame> const& acceleration)
			{ _acceleration = acceleration; }

		[[nodiscard]]
		constexpr SpaceVector<si::AngularAcceleration, Frame> const&
		angular_acceleration() const noexcept
			{ return _angular_acceleration; }

		constexpr void
		set_angular_acceleration (SpaceVector<si::AngularAcceleration, Frame> const& angular_acceleration)
			{ _angular_acceleration = angular_acceleration; }

	  private:
		SpaceVector<si::Acceleration, Frame>		_acceleration			{ 0_mps2, 0_mps2, 0_mps2 };
		SpaceVector<si::AngularAcceleration, Frame>	_angular_acceleration	{ 0_radps2, 0_radps2, 0_radps2 };
	};


template<class Frame>
	constexpr
	AccelerationMoments<Frame>::AccelerationMoments (SpaceVector<si::Acceleration, Frame> const& acceleration, SpaceVector<si::AngularAcceleration, Frame> const& angular_acceleration):
		_acceleration (acceleration),
		_angular_acceleration (angular_acceleration)
	{ }


template<class Frame>
	constexpr AccelerationMoments<Frame>&
	AccelerationMoments<Frame>::operator+= (AccelerationMoments const& other)
	{
		_acceleration += other._acceleration;
		_angular_acceleration += other._angular_acceleration;
		return *this;
	}


template<class Frame>
	constexpr AccelerationMoments<Frame>&
	AccelerationMoments<Frame>::operator-= (AccelerationMoments const& other)
	{
		_acceleration -= other._acceleration;
		_angular_acceleration -= other._angular_acceleration;
		return *this;
	}


/*
 * Global functions
 */


template<class Frame>
	constexpr AccelerationMoments<Frame>
	operator+ (AccelerationMoments<Frame> a)
	{
		return a;
	}


template<class Frame>
	constexpr AccelerationMoments<Frame>
	operator+ (AccelerationMoments<Frame> a, AccelerationMoments<Frame> const& b)
	{
		return a += b;
	}


template<class Frame>
	constexpr AccelerationMoments<Frame>
	operator- (AccelerationMoments<Frame> a)
	{
		return {
			-a.acceleration(),
			-a.angular_acceleration(),
		};
	}


template<class Frame>
	constexpr AccelerationMoments<Frame>
	operator- (AccelerationMoments<Frame> a, AccelerationMoments<Frame> const& b)
	{
		return a -= b;
	}


template<class TargetFrame, class SourceFrame>
	constexpr AccelerationMoments<TargetFrame>
	operator* (SpaceMatrix<double, TargetFrame, SourceFrame> const& transformation,
			   AccelerationMoments<SourceFrame> const& acceleration_moments)
	{
		return {
			transformation * acceleration_moments.acceleration(),
			transformation * acceleration_moments.angular_acceleration(),
		};
	}


template<class Multiplier, class Frame>
	constexpr AccelerationMoments<Frame>
	operator* (Multiplier const& multiplier,
			   AccelerationMoments<Frame> const& acceleration_moments)
	{
		return {
			multiplier * acceleration_moments.acceleration(),
			multiplier * acceleration_moments.angular_acceleration(),
		};
	}

} // namespace xf

#endif

