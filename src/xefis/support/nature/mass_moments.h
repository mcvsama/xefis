/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__NATURE__MASS_MOMENTS_H__INCLUDED
#define XEFIS__SUPPORT__NATURE__MASS_MOMENTS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/math/placement.h>
#include <xefis/support/math/triangle.h>
#include <xefis/support/nature/mass_moments_at_arm.h>

// Neutrino:
#include <neutrino/math/math.h>

// Lib:
#include <boost/range.hpp>

// Standard:
#include <cstddef>
#include <initializer_list>
#include <iterator>


namespace xf {

/**
 * Like MassMomentsAtArm, but assumes that the inertia tensor is viewed from center of mass.
 * And the center of mass always returns a zero vector.
 */
template<class pSpace = void>
	class MassMoments
	{
	  public:
		using Space = pSpace;

	  public:
		// Ctor
		MassMoments() = default;

		// Ctor
		MassMoments (si::Mass, InertiaTensor<Space> const& inertia_tensor_at_com);

		// Ctor method
		static MassMomentsAtArm<Space>
		zero()
			{ return {};}

		/**
		 * Convert to standard MassMomentsAtArm.
		 */
		explicit operator MassMomentsAtArm<Space>() const
			{ return { _mass, math::zero, _inertia_tensor }; }

		/**
		 * Rest mass.
		 */
		[[nodiscard]]
		si::Mass
		mass() const noexcept
			{ return _mass; }

		/**
		 * Position of center of mass.
		 */
		[[nodiscard]]
		SpaceLength<Space> const&
		center_of_mass_position() const noexcept
			{ return _zero_length; }

		/**
		 * Moments of inertia tensor viewed from the origin point,
		 * not from the center of mass.
		 */
		[[nodiscard]]
		InertiaTensor<Space> const&
		inertia_tensor() const noexcept
			{ return _inertia_tensor; }

		/**
		 * Inverse moments of inertia tensor.
		 */
		[[nodiscard]]
		typename InertiaTensor<Space>::InverseMatrix const&
		inverse_inertia_tensor() const noexcept
			{ return _inverse_inertia_tensor; }

	  private:
		si::Mass									_mass						{ 0_kg };
		SpaceLength<Space>							_zero_length				{ 0_m, 0_m, 0_m };
		InertiaTensor<Space>						_inertia_tensor				{ math::zero };
		// TODO consider making this optional (+std::mutex and mutable keyword):
		decltype (_inertia_tensor)::InverseMatrix	_inverse_inertia_tensor		{ math::zero };
	};


/*
 * Global functions
 */


template<class TargetSpace, class SourceSpace>
	inline MassMoments<TargetSpace>
	operator* (RotationQuaternion<TargetSpace, SourceSpace> const& rotation,
			   MassMoments<SourceSpace> const& mass_moments)
	{
		// Rotating a tensor with a quaterion:
		// either rotate each vector component of a tensor with the quaterion
		// separately or create a rotation matrix R and use formula: R * Tensor * ~R.
		auto const rotation_matrix = RotationMatrix<TargetSpace, SourceSpace> (rotation);

		return {
			mass_moments.mass(),
			rotation_matrix * mass_moments.inertia_tensor() * ~rotation_matrix,
		};
	}


/*
 * MassMomentsAtArm functions
 */


template<class S>
	inline
	MassMoments<S>::MassMoments (si::Mass mass, InertiaTensor<Space> const& inertia_tensor_at_com):
		_mass (mass),
		_inertia_tensor (inertia_tensor_at_com),
		_inverse_inertia_tensor (inv (inertia_tensor_at_com))
	{ }

} // namespace xf

#endif

