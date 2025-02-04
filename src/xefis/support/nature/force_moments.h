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
template<class Space = void>
	class ForceMoments
	{
	  public:
		// Ctor
		constexpr
		ForceMoments() = default;

		// Ctor
		constexpr
		ForceMoments (SpaceVector<si::Force, Space> const& force, SpaceVector<si::Torque, Space> const& torque);

		constexpr ForceMoments&
		operator+= (ForceMoments const&);

		constexpr ForceMoments&
		operator-= (ForceMoments const&);

		constexpr ForceMoments&
		operator+= (SpaceVector<si::Force, Space> const&);

		constexpr ForceMoments&
		operator-= (SpaceVector<si::Force, Space> const&);

		constexpr ForceMoments&
		operator+= (SpaceVector<si::Torque, Space> const&);

		constexpr ForceMoments&
		operator-= (SpaceVector<si::Torque, Space> const&);

		[[nodiscard]]
		constexpr SpaceVector<si::Force, Space> const&
		force() const noexcept
			{ return _force; }

		constexpr void
		set_force (SpaceVector<si::Force, Space> const& force)
			{ _force = force; }

		[[nodiscard]]
		constexpr SpaceVector<si::Torque, Space> const&
		torque() const noexcept
			{ return _torque; }

		constexpr void
		set_torque (SpaceVector<si::Torque, Space> const& torque)
			{ _torque = torque; }

		/**
		 * Return this ForceMoments at given point (torque will be different).
		 * That is return resultant force as if this was a wrench with force/torque application point at -point.
		 */
		constexpr ForceMoments<Space>
		at (SpaceVector<si::Length, Space> const& point) const;

	  public:
		static ForceMoments<Space>
		zero()
			{ return ForceMoments<Space> (math::zero, math::zero); }

	  private:
		SpaceVector<si::Force, Space>	_force	{ 0_N, 0_N, 0_N };
		SpaceVector<si::Torque, Space>	_torque	{ 0_Nm, 0_Nm, 0_Nm };
	};


template<class Space>
	constexpr
	ForceMoments<Space>::ForceMoments (SpaceVector<si::Force, Space> const& force, SpaceVector<si::Torque, Space> const& torque):
		_force (force),
		_torque (torque)
	{ }


template<class Space>
	constexpr ForceMoments<Space>&
	ForceMoments<Space>::operator+= (ForceMoments const& other)
	{
		_force += other._force;
		_torque += other._torque;
		return *this;
	}


template<class Space>
	constexpr ForceMoments<Space>&
	ForceMoments<Space>::operator-= (ForceMoments const& other)
	{
		_force -= other._force;
		_torque -= other._torque;
		return *this;
	}


template<class Space>
	constexpr ForceMoments<Space>&
	ForceMoments<Space>::operator+= (SpaceVector<si::Force, Space> const& other)
	{
		_force += other;
		return *this;
	}


template<class Space>
	constexpr ForceMoments<Space>&
	ForceMoments<Space>::operator-= (SpaceVector<si::Force, Space> const& other)
	{
		_force -= other;
		return *this;
	}


template<class Space>
	constexpr ForceMoments<Space>&
	ForceMoments<Space>::operator+= (SpaceVector<si::Torque, Space> const& other)
	{
		_torque += other;
		return *this;
	}


template<class Space>
	constexpr ForceMoments<Space>&
	ForceMoments<Space>::operator-= (SpaceVector<si::Torque, Space> const& other)
	{
		_torque -= other;
		return *this;
	}


template<class Space>
	constexpr ForceMoments<Space>
	ForceMoments<Space>::at (SpaceVector<si::Length, Space> const& point) const
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


template<class Space>
	constexpr ForceMoments<Space>
	operator+ (ForceMoments<Space> a)
	{
		return a;
	}


template<class Space>
	constexpr ForceMoments<Space>
	operator+ (ForceMoments<Space> a, ForceMoments<Space> const& b)
	{
		return a += b;
	}


template<class Space>
	constexpr ForceMoments<Space>
	operator+ (ForceMoments<Space> a, SpaceVector<si::Force, Space> const& b)
	{
		return a += b;
	}


template<class Space>
	constexpr ForceMoments<Space>
	operator+ (ForceMoments<Space> a, SpaceVector<si::Torque, Space> const& b)
	{
		return a += b;
	}


template<class Space>
	constexpr ForceMoments<Space>
	operator- (ForceMoments<Space> a)
	{
		return {
			-a.force(),
			-a.torque(),
		};
	}


template<class Space>
	constexpr ForceMoments<Space>
	operator- (ForceMoments<Space> a, ForceMoments<Space> const& b)
	{
		return a -= b;
	}


template<class Space>
	constexpr ForceMoments<Space>
	operator- (ForceMoments<Space> a, SpaceVector<si::Force, Space> const& b)
	{
		return a -= b;
	}


template<class Space>
	constexpr ForceMoments<Space>
	operator- (ForceMoments<Space> a, SpaceVector<si::Torque, Space> const& b)
	{
		return a -= b;
	}


template<class TargetSpace, class SourceSpace>
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

#endif

