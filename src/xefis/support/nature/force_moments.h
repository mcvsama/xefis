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

#ifndef XEFIS__SUPPORT__NATURE__FORCE_MOMENTS_H__INCLUDED
#define XEFIS__SUPPORT__NATURE__FORCE_MOMENTS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>

// Neutrino:
#include <neutrino/math/math.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * ForceMoments represents basic moments of force:
 *   • 0th moment = force
 *   • 1st moment = torque (except for moments that don't change angular momentum).
 */
template<math::CoordinateSystem Space = void>
	class ForceMoments
	{
	  public:
		// Ctor
		constexpr
		ForceMoments() = default;

		// Ctor
		constexpr
		ForceMoments (SpaceForce<Space> const& force, SpaceTorque<Space> const& torque);

		constexpr ForceMoments&
		operator+= (ForceMoments const&);

		constexpr ForceMoments&
		operator-= (ForceMoments const&);

		constexpr ForceMoments&
		operator+= (SpaceForce<Space> const&);

		constexpr ForceMoments&
		operator-= (SpaceForce<Space> const&);

		constexpr ForceMoments&
		operator+= (SpaceTorque<Space> const&);

		constexpr ForceMoments&
		operator-= (SpaceTorque<Space> const&);

		[[nodiscard]]
		constexpr SpaceForce<Space> const&
		force() const noexcept
			{ return _force; }

		constexpr void
		set_force (SpaceForce<Space> const& force)
			{ _force = force; }

		[[nodiscard]]
		constexpr SpaceTorque<Space> const&
		torque() const noexcept
			{ return _torque; }

		constexpr void
		set_torque (SpaceTorque<Space> const& torque)
			{ _torque = torque; }

		/**
		 * Return this ForceMoments at given point (torque will be different).
		 * That is return resultant force as if this was a wrench with force/torque application point at -point.
		 */
		constexpr ForceMoments<Space>
		at (SpaceLength<Space> const& point) const;

	  public:
		static ForceMoments<Space>
		zero()
			{ return ForceMoments<Space> (math::zero, math::zero); }

	  private:
		SpaceForce<Space>	_force	{ 0_N, 0_N, 0_N };
		SpaceTorque<Space>	_torque	{ 0_Nm, 0_Nm, 0_Nm };
	};


template<math::CoordinateSystem Space>
	constexpr
	ForceMoments<Space>::ForceMoments (SpaceForce<Space> const& force, SpaceTorque<Space> const& torque):
		_force (force),
		_torque (torque)
	{ }


template<math::CoordinateSystem Space>
	constexpr ForceMoments<Space>&
	ForceMoments<Space>::operator+= (ForceMoments const& other)
	{
		_force += other._force;
		_torque += other._torque;
		return *this;
	}


template<math::CoordinateSystem Space>
	constexpr ForceMoments<Space>&
	ForceMoments<Space>::operator-= (ForceMoments const& other)
	{
		_force -= other._force;
		_torque -= other._torque;
		return *this;
	}


template<math::CoordinateSystem Space>
	constexpr ForceMoments<Space>&
	ForceMoments<Space>::operator+= (SpaceForce<Space> const& other)
	{
		_force += other;
		return *this;
	}


template<math::CoordinateSystem Space>
	constexpr ForceMoments<Space>&
	ForceMoments<Space>::operator-= (SpaceForce<Space> const& other)
	{
		_force -= other;
		return *this;
	}


template<math::CoordinateSystem Space>
	constexpr ForceMoments<Space>&
	ForceMoments<Space>::operator+= (SpaceTorque<Space> const& other)
	{
		_torque += other;
		return *this;
	}


template<math::CoordinateSystem Space>
	constexpr ForceMoments<Space>&
	ForceMoments<Space>::operator-= (SpaceTorque<Space> const& other)
	{
		_torque -= other;
		return *this;
	}


template<math::CoordinateSystem Space>
	constexpr ForceMoments<Space>
	ForceMoments<Space>::at (SpaceLength<Space> const& point) const
	{
		auto const additional_torque = cross_product (-point, _force);

		return {
			_force,
			_torque + additional_torque,
		};
	}


/*
 * Global functions
 */


template<math::CoordinateSystem Space>
	constexpr ForceMoments<Space>
	operator+ (ForceMoments<Space> a)
	{
		return a;
	}


template<math::CoordinateSystem Space>
	constexpr ForceMoments<Space>
	operator+ (ForceMoments<Space> a, ForceMoments<Space> const& b)
	{
		return a += b;
	}


template<math::CoordinateSystem Space>
	constexpr ForceMoments<Space>
	operator+ (ForceMoments<Space> a, SpaceForce<Space> const& b)
	{
		return a += b;
	}


template<math::CoordinateSystem Space>
	constexpr ForceMoments<Space>
	operator+ (ForceMoments<Space> a, SpaceTorque<Space> const& b)
	{
		return a += b;
	}


template<math::CoordinateSystem Space>
	constexpr ForceMoments<Space>
	operator- (ForceMoments<Space> a)
	{
		return {
			-a.force(),
			-a.torque(),
		};
	}


template<math::CoordinateSystem Space>
	constexpr ForceMoments<Space>
	operator- (ForceMoments<Space> a, ForceMoments<Space> const& b)
	{
		return a -= b;
	}


template<math::CoordinateSystem Space>
	constexpr ForceMoments<Space>
	operator- (ForceMoments<Space> a, SpaceForce<Space> const& b)
	{
		return a -= b;
	}


template<math::CoordinateSystem Space>
	constexpr ForceMoments<Space>
	operator- (ForceMoments<Space> a, SpaceTorque<Space> const& b)
	{
		return a -= b;
	}


template<math::CoordinateSystem TargetSpace, math::CoordinateSystem SourceSpace>
	constexpr ForceMoments<TargetSpace>
	operator* (RotationQuaternion<TargetSpace, SourceSpace> const& rotation,
			   ForceMoments<SourceSpace> const& force_moments)
	{
		return {
			rotation * force_moments.force(),
			rotation * force_moments.torque(),
		};
	}

} // namespace xf


#include <xefis/support/nature/acceleration_moments.h>
#include <xefis/support/nature/mass_moments.h>


namespace xf {

template<math::CoordinateSystem Space = void>
	[[nodiscard]]
	constexpr ForceMoments<Space>
	compute_force_moments (MassMoments<Space> const& mm, AccelerationMoments<Space> const& am)
	{
		return compute_force_moments (mm, am);
	}


template<math::CoordinateSystem Space = void>
	[[nodiscard]]
	constexpr ForceMoments<Space>
	operator* (MassMoments<Space> const& mm, AccelerationMoments<Space> const& am)
	{
		return compute_force_moments (mm, am);
	}


template<math::CoordinateSystem Space = void>
	[[nodiscard]]
	constexpr ForceMoments<Space>
	operator* (AccelerationMoments<Space> const& am, MassMoments<Space> const& mm)
	{
		return compute_force_moments (mm, am);
	}

} // namespace xf

#endif

