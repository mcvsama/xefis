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

#ifndef XEFIS__SUPPORT__NATURE__MASS_MOMENTS_AT_ARM_H__INCLUDED
#define XEFIS__SUPPORT__NATURE__MASS_MOMENTS_AT_ARM_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/math/placement.h>
#include <xefis/support/math/triangle.h>

// Neutrino:
#include <neutrino/math/math.h>

// Lib:
#include <boost/range.hpp>

// Standard:
#include <cstddef>
#include <initializer_list>
#include <iterator>


namespace xf {

template<class Space>
	using InertiaTensor = SpaceMatrix<si::MomentOfInertia, Space>;


template<class Space>
	class MassMoments;


/**
 * Represents three moments of mass:
 *  • 0th = mass (monopole)
 *  • 1st = center of mass (dipole)
 *  • 2nd = moments of inertia tensor at the center of mass (quadrupole)
 */
template<class pSpace = void>
	class MassMomentsAtArm
	{
	  public:
		using Space = pSpace;

	  public:
		// Ctor
		MassMomentsAtArm() = default;

		// Ctor
		MassMomentsAtArm (si::Mass, InertiaTensor<Space> const& inertia_tensor_at_com);

		/**
		 * Create a mass moments with an inertia tensor viewed from different place than center of mass.
		 * To use a mass moments with (more useful) inertia tensor at COM, transform it with .
		 *
		 * \param	inertia_tensor_at_origin
		 *			Inertia tensor as viewed from the -center_of_mass_position (named origin here).
		 */
		MassMomentsAtArm (si::Mass, SpaceLength<Space> const& center_of_mass_position, InertiaTensor<Space> const& inertia_tensor_at_origin);

		/**
		 * Create MassMomentsAtArm from MassMoments with center of mass being at position [0, 0, 0].
		 */
		MassMomentsAtArm (MassMoments<Space> const&);

		// Ctor method
		static MassMomentsAtArm<Space>
		zero()
			{ return {}; }

		// Ctor method
		static MassMomentsAtArm
		from_point_masses (std::forward_iterator auto begin, std::forward_iterator auto end);

		// Ctor method
		static MassMomentsAtArm
		from_point_masses (std::initializer_list<MassMomentsAtArm<Space>> point_masses)
			{ return from_point_masses (point_masses.begin(), point_masses.end()); }

		/**
		 * Add another mass moments and thus create mass moments for the system of the two bodies
		 * like they were one. Assumes that the origin for both bodies is the same point in space
		 * (doesn't have to be center of mass or anything specific).
		 */
		MassMomentsAtArm&
		operator+= (MassMomentsAtArm const& other);

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
			{ return _center_of_mass_position; }

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

		/**
		 * Return the same mass moments but viewed from the center of mass.
		 * Result will have center of mass at [0, 0, 0] and updated inertia tensor.
		 */
		[[nodiscard]]
		MassMoments<Space>
		centered_at_center_of_mass() const
			{ return MassMoments<Space> (*this); }

	  private:
		si::Mass									_mass						{ 0_kg };
		SpaceLength<Space>							_center_of_mass_position	{ 0_m, 0_m, 0_m };
		InertiaTensor<Space>						_inertia_tensor				{ math::zero };
		// TODO consider making this optional (+std::mutex and mutable keyword):
		decltype (_inertia_tensor)::InverseMatrix	_inverse_inertia_tensor		{ math::zero };
	};


/*
 * Global functions
 */


template<class Space>
	inline MassMomentsAtArm<Space>
	operator+ (MassMomentsAtArm<Space> a, MassMomentsAtArm<Space> const& b)
	{
		return a += b;
	}


template<class TargetSpace, class SourceSpace>
	inline MassMomentsAtArm<TargetSpace>
	operator* (RotationQuaternion<TargetSpace, SourceSpace> const& rotation,
			   MassMomentsAtArm<SourceSpace> const& mass_moments)
	{
		auto const rotation_matrix = RotationMatrix<TargetSpace, SourceSpace> (rotation);

		return {
			mass_moments.mass(),
			rotation_matrix * mass_moments.center_of_mass_position(),
			rotation_matrix * mass_moments.inertia_tensor() * ~rotation_matrix,
		};
	}


/**
 * Return inertia tensor resulting from a spatial displacement.
 * Doesn't include the own (center of mass) inertia tensor part.
 * The displacement vector R can be negated without changing the result.
 */
template<class Space>
	InertiaTensor<Space>
	displacement_inertia_tensor (si::Mass const mass, SpaceLength<Space> const& R)
	{
		SpaceMatrix<double, Space> const E (math::unit);
		// m * [(R ⋅ R) E3 - R ⊗ R]
		return mass * (dot_product (R, R) * E - outer_product (R, R));
	}


/**
 * Converts inertia tensor as seen from the center of mass to seen from given point.
 * The displacement is relative to center of mass position.
 */
template<class Space>
	InertiaTensor<Space>
	inertia_tensor_com_to_point (si::Mass const mass,
								 InertiaTensor<Space> const& inertia_tensor_at_center_of_mass,
								 SpaceLength<Space> const& displacement_from_com)
	{
		return inertia_tensor_at_center_of_mass + displacement_inertia_tensor (mass, displacement_from_com);
	}


/**
 * Converts inertia tensor as seen from any point to seen from center of mass.
 * The displacement is relative to center of mass position.
 */
template<class Space>
	InertiaTensor<Space>
	inertia_tensor_point_to_com (si::Mass const mass,
								 InertiaTensor<Space> const& inertia_tensor_at_point,
								 SpaceLength<Space> const& displacement_from_com)
	{
		return inertia_tensor_at_point - displacement_inertia_tensor (mass, displacement_from_com);
	}


/**
 * Converts inertia tensor as seen from one point to seen from another point.
 * The displacements are relative to center of mass position.
 */
template<class Space>
	InertiaTensor<Space>
	inertia_tensor_point_to_point (si::Mass const mass,
								   InertiaTensor<Space> const& old_inertia_tensor_at_point,
								   SpaceLength<Space> const& old_displacement_from_com,
								   SpaceLength<Space> const& new_displacement_from_com)
	{
        return old_inertia_tensor_at_point - displacement_inertia_tensor (mass, old_displacement_from_com) + displacement_inertia_tensor (mass, new_displacement_from_com);
	}


/**
 * Calculate mass moments of a wing viewed from origin.
 * Assuming the wing is extruded along +Z axis and chord length scales X and Y axes.
 */
template<class Scalar, class Space>
	inline MassMomentsAtArm<Space>
	calculate_mass_moments (std::vector<PlaneTriangle<Scalar, Space>> const& polygon_triangulation, si::Length const chord_length, si::Length const wing_length, si::Density const material_density)
	{
		// Have 2D triangulation points, make two sets of them, split the virtual wing into two identical-length parts,
		// make the points at the center of each wing part. This way we'll get correct MOI for all 3D axes.

		std::vector<MassMomentsAtArm<Space>> point_masses;
		point_masses.reserve (2 * polygon_triangulation.size());

		for (auto const& triangle: polygon_triangulation)
		{
			auto const scaler = chord_length;
			auto const centroid = scaler * triangle_centroid (triangle);
			auto const area = scaler * scaler * area_2d (triangle);
			auto const volume = area * 0.5 * wing_length;
			auto const point_mass = volume * material_density;
			auto const position_1 = SpaceLength<Space> (centroid[0], centroid[1], 0.25 * wing_length);
			auto const position_2 = SpaceLength<Space> (centroid[0], centroid[1], 0.75 * wing_length);
			auto const inertia_tensor_1 = displacement_inertia_tensor (point_mass, position_1);
			auto const inertia_tensor_2 = displacement_inertia_tensor (point_mass, position_2);

			point_masses.push_back (MassMomentsAtArm<Space> { point_mass, position_1, inertia_tensor_1 });
			point_masses.push_back (MassMomentsAtArm<Space> { point_mass, position_2, inertia_tensor_2 });
		}

		return MassMomentsAtArm<Space>::from_point_masses (begin (point_masses), end (point_masses));
	}


/*
 * MassMomentsAtArm functions
 */


template<class S>
	inline
	MassMomentsAtArm<S>::MassMomentsAtArm (si::Mass mass, InertiaTensor<Space> const& inertia_tensor_at_com):
		MassMomentsAtArm (mass, math::zero, inertia_tensor_at_com) // Origin is the same as center-of-mass here.
	{ }


template<class S>
	inline
	MassMomentsAtArm<S>::MassMomentsAtArm (si::Mass mass, SpaceLength<Space> const& center_of_mass_position, InertiaTensor<Space> const& inertia_tensor_at_origin):
		_mass (mass),
		_center_of_mass_position (center_of_mass_position),
		_inertia_tensor (inertia_tensor_at_origin),
		_inverse_inertia_tensor (inv (inertia_tensor_at_origin))
	{ }


template<class S>
	inline
	MassMomentsAtArm<S>::MassMomentsAtArm (MassMoments<Space> const& mass_moments):
		_mass (mass_moments.mass()),
		_inertia_tensor (mass_moments.inertia_tensor()),
		_inverse_inertia_tensor (mass_moments.inverse_inertia_tensor())
	{ }


template<class S>
	inline MassMomentsAtArm<S>
	MassMomentsAtArm<S>::from_point_masses (std::forward_iterator auto begin, std::forward_iterator auto end)
	{
		MassMomentsAtArm<S> result;

		for (auto const& point_mass: boost::make_iterator_range (begin, end))
			result += point_mass;

		return result;
	}


template<class S>
	inline MassMomentsAtArm<S>&
	MassMomentsAtArm<S>::operator+= (MassMomentsAtArm const& other)
	{
		auto const m1 = _mass;
		auto const m2 = other._mass;
		auto const& r1 = _center_of_mass_position;
		auto const& r2 = other._center_of_mass_position;
		auto const& I1 = _inertia_tensor;
		auto const& I2 = other._inertia_tensor;

		_mass = m1 + m2;
		_center_of_mass_position = (m1 * r1 + m2 * r2) / (m1 + m2);
		_inertia_tensor = I1 + I2;
		_inverse_inertia_tensor = inv (_inertia_tensor);

		return *this;
	}

} // namespace xf

#endif

