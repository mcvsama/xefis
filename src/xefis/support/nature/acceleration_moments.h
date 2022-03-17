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
template<class Space = void>
	struct AccelerationMoments
	{
	  public:
		// Ctor
		constexpr
		AccelerationMoments() = default;

		// Ctor
		constexpr
		AccelerationMoments (SpaceVector<si::Acceleration, Space> const& acceleration,
							 SpaceVector<si::AngularAcceleration, Space> const& angular_acceleration);

		constexpr AccelerationMoments&
		operator+= (AccelerationMoments const& other);

		constexpr AccelerationMoments&
		operator-= (AccelerationMoments const& other);

		[[nodiscard]]
		constexpr SpaceVector<si::Acceleration, Space> const&
		acceleration() const noexcept
			{ return _acceleration; }

		constexpr void
		set_acceleration (SpaceVector<si::Acceleration, Space> const& acceleration)
			{ _acceleration = acceleration; }

		[[nodiscard]]
		constexpr SpaceVector<si::AngularAcceleration, Space> const&
		angular_acceleration() const noexcept
			{ return _angular_acceleration; }

		constexpr void
		set_angular_acceleration (SpaceVector<si::AngularAcceleration, Space> const& angular_acceleration)
			{ _angular_acceleration = angular_acceleration; }

	  private:
		SpaceVector<si::Acceleration, Space>		_acceleration			{ 0_mps2, 0_mps2, 0_mps2 };
		SpaceVector<si::AngularAcceleration, Space>	_angular_acceleration	{ 0_radps2, 0_radps2, 0_radps2 };
	};


template<class Space>
	constexpr
	AccelerationMoments<Space>::AccelerationMoments (SpaceVector<si::Acceleration, Space> const& acceleration, SpaceVector<si::AngularAcceleration, Space> const& angular_acceleration):
		_acceleration (acceleration),
		_angular_acceleration (angular_acceleration)
	{ }


template<class Space>
	constexpr AccelerationMoments<Space>&
	AccelerationMoments<Space>::operator+= (AccelerationMoments const& other)
	{
		_acceleration += other._acceleration;
		_angular_acceleration += other._angular_acceleration;
		return *this;
	}


template<class Space>
	constexpr AccelerationMoments<Space>&
	AccelerationMoments<Space>::operator-= (AccelerationMoments const& other)
	{
		_acceleration -= other._acceleration;
		_angular_acceleration -= other._angular_acceleration;
		return *this;
	}


/*
 * Global functions
 */


template<class Space>
	constexpr AccelerationMoments<Space>
	operator+ (AccelerationMoments<Space> a)
	{
		return a;
	}


template<class Space>
	constexpr AccelerationMoments<Space>
	operator+ (AccelerationMoments<Space> a, AccelerationMoments<Space> const& b)
	{
		return a += b;
	}


template<class Space>
	constexpr AccelerationMoments<Space>
	operator- (AccelerationMoments<Space> a)
	{
		return {
			-a.acceleration(),
			-a.angular_acceleration(),
		};
	}


template<class Space>
	constexpr AccelerationMoments<Space>
	operator- (AccelerationMoments<Space> a, AccelerationMoments<Space> const& b)
	{
		return a -= b;
	}


template<class TargetSpace, class SourceSpace>
	constexpr AccelerationMoments<TargetSpace>
	operator* (SpaceMatrix<double, TargetSpace, SourceSpace> const& transformation,
			   AccelerationMoments<SourceSpace> const& acceleration_moments)
	{
		return {
			transformation * acceleration_moments.acceleration(),
			transformation * acceleration_moments.angular_acceleration(),
		};
	}


template<class Multiplier, class Space>
	constexpr AccelerationMoments<Space>
	operator* (Multiplier const& multiplier,
			   AccelerationMoments<Space> const& acceleration_moments)
	{
		return {
			multiplier * acceleration_moments.acceleration(),
			multiplier * acceleration_moments.angular_acceleration(),
		};
	}

} // namespace xf

#endif

