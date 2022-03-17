/* vim:ts=4
 *
 * Copyleft 2008…2018  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__N_BODY__MASS_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__N_BODY__MASS_H__INCLUDED

// Standard:
#include <cstddef>
#include <initializer_list>

// Lib:
#include <boost/range.hpp>

// Neutrino:
#include <neutrino/math/math.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>
#include <xefis/support/math/placement.h>
#include <xefis/support/geometry/triangle.h>


namespace xf {

template<class pSpace = void>
	class PointMass
	{
	  public:
		using Space = pSpace;

	  public:
		// Monopole moment:
		si::Mass								mass;
		// Dipole moment:
		SpaceVector<si::Length, Space>			position;
		// Quadrupole moment:
		SpaceMatrix<si::MomentOfInertia, Space>	moment_of_inertia;
	};


/**
 * Tag used in one of constructors of MassMoments.
 */
class FromPointMassRange
{ };


/**
 * Represents three moments of mass:
 *  • 0th = mass (monopole)
 *  • 1st = center of mass (dipole)
 *  • 2nd = moment of inertia at the center of mass (quadrupole)
 */
template<class pSpace = void>
	class MassMoments
	{
	  public:
		using Space		= pSpace;
		using PointMass	= xf::PointMass<Space>;

	  public:
		// Ctor
		MassMoments() = default;

		// TODO construct from direct mass, COM and MOI. Calculation from point masses should be in separate (static) function.

		// Ctor
		MassMoments (si::Mass, SpaceVector<si::Length, Space> const& center_of_mass_position, SpaceMatrix<si::MomentOfInertia, Space> const& moment_of_inertia);

		// Ctor
		MassMoments (std::initializer_list<PointMass> point_masses);

		// Ctor
		template<class PointMassIterator>
			explicit
			MassMoments (FromPointMassRange, PointMassIterator begin, PointMassIterator end);

		// Ctor method
		template<class PointMassIterator>
			static MassMoments
			from_point_masses (PointMassIterator begin, PointMassIterator end)
				{ return MassMoments (FromPointMassRange(), begin, end); }

		MassMoments&
		operator+= (MassMoments const& other);

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
		SpaceVector<si::Length, Space> const&
		center_of_mass_position() const noexcept
			{ return _center_of_mass_position; }

		/**
		 * Moment of inertia tensor about the center of mass.
		 */
		[[nodiscard]]
		SpaceMatrix<si::MomentOfInertia, Space> const&
		moment_of_inertia() const noexcept
		// TODO rozdziel na MOI@zero i na MOI@COM
			{ return _moment_of_inertia; }

		/**
		 * Inversed moment of inertia.
		 */
		[[nodiscard]]
		typename SpaceMatrix<si::MomentOfInertia, Space>::InversedMatrix const&
		inversed_moment_of_inertia() const noexcept
		// TODO rozdziel na MOI@zero i na MOI@COM
			{ return _inversed_moment_of_inertia; }

	  public:
		static MassMoments<Space>
		zero()
			{ return MassMoments<Space> (0_kg, math::zero, math::zero); }

	  private:
		si::Mass															_mass						{ 0_kg };
		SpaceVector<si::Length, Space>										_center_of_mass_position	{ 0_m, 0_m, 0_m };
		SpaceMatrix<si::MomentOfInertia, Space>								_moment_of_inertia			{ math::zero };
		typename SpaceMatrix<si::MomentOfInertia, Space>::InversedMatrix	_inversed_moment_of_inertia	{ math::zero };
	};


// Forward
template<class Space>
	SpaceMatrix<si::MomentOfInertia, Space>
	parallel_axis_theorem (SpaceMatrix<si::MomentOfInertia, Space> const& moi_at_com, si::Mass mass, SpaceVector<si::Length, Space> const& displacement);


template<class F>
	inline
	MassMoments<F>::MassMoments (si::Mass mass, SpaceVector<si::Length, Space> const& center_of_mass_position, SpaceMatrix<si::MomentOfInertia, Space> const& moment_of_inertia):
		_mass (mass),
		_center_of_mass_position (center_of_mass_position),
		_moment_of_inertia (moment_of_inertia),
		_inversed_moment_of_inertia (inv (moment_of_inertia))
	{ }


template<class F>
	inline
	MassMoments<F>::MassMoments (std::initializer_list<PointMass> point_masses):
		MassMoments (FromPointMassRange(), point_masses.begin(), point_masses.end())
	{ }


template<class F>
	template<class PointMassIterator>
		inline
		MassMoments<F>::MassMoments (FromPointMassRange, PointMassIterator begin, PointMassIterator end)
		{
			size_t count = 0;

			// TODO what really is needed: skipping pointmasses with mass == 0_kg.
			SpaceMatrix<double, Space> const unit (math::unit);

			si::Mass computed_mass = 0_kg;
			SpaceVector<decltype (si::Length{} * si::Mass{}), Space> computed_center (math::zero);
			SpaceMatrix<si::MomentOfInertia, Space> computed_moment_of_inertia (math::zero);

			// Center of mass formula: R = 1/M * Σ m[i] * r[i]; where M is total mass.

			for (auto const& point_mass: boost::make_iterator_range (begin, end))
			{
				auto const m = point_mass.mass;
				auto const r = point_mass.position;
				auto const i = point_mass.moment_of_inertia;

				if (m != 0_kg)
				{
					++count;
					computed_mass += m;
					computed_center += m * r;
					computed_moment_of_inertia += parallel_axis_theorem (i, m, r);
				}
			}

			// Override values if we actually only used one point mass, or none at all:
			if (count == 0)
			{
				_mass = 0_kg;
				_center_of_mass_position = math::zero;
				_moment_of_inertia = math::zero;
				_inversed_moment_of_inertia = math::zero;
			}
			else if (count == 1)
			{
				_mass = begin->mass;
				_center_of_mass_position = begin->position;
				_moment_of_inertia = begin->moment_of_inertia;
				_inversed_moment_of_inertia = inv (_moment_of_inertia);
			}
			else
			{
				_mass = computed_mass;
				_moment_of_inertia = computed_moment_of_inertia;
				_inversed_moment_of_inertia = inv (computed_moment_of_inertia);
				_center_of_mass_position = computed_center / computed_mass;
			}
		}


template<class F>
	inline MassMoments<F>&
	MassMoments<F>::operator+= (MassMoments const& other)
	{
		auto const& mm1 = *this;
		auto const& mm2 = other;

		MassMoments<F> result;
		result._mass = mm1._mass + mm2._mass;

		if (result._mass != 0_kg)
		{
			// New center of mass:
			result._center_of_mass_position = 1.0 / result._mass * (mm1._mass * mm1._center_of_mass_position + mm2._mass * mm2._center_of_mass_position);
			// Tensor parallel axis theorem: <https://en.wikipedia.org/wiki/Parallel_axis_theorem#Tensor_generalization>
			// FIXME the original mass_moments.moment_of_inertia() must go through COM; that is done. But make sure that result._moment_of_inertia also goes through
			// COM of the result.
			result._moment_of_inertia = parallel_axis_theorem (mm1._moment_of_inertia, mm1._mass, mm1._center_of_mass_position - result._center_of_mass_position)
									  + parallel_axis_theorem (mm2._moment_of_inertia, mm2._mass, mm2._center_of_mass_position - result._center_of_mass_position);
			result._inversed_moment_of_inertia = inv (result._moment_of_inertia);

			*this = result;
		}

		return *this;
	}


/*
 * Global functions
 */


template<class Space>
	inline MassMoments<Space>
	operator+ (MassMoments<Space> a, MassMoments<Space> const& b)
	{
		return a += b;
	}


template<class TargetSpace, class SourceSpace>
	inline MassMoments<TargetSpace>
	operator* (SpaceMatrix<double, TargetSpace, SourceSpace> const& transformation,
			   MassMoments<SourceSpace> const& mass_moments)
	{
		return {
			mass_moments.mass(),
			transformation * mass_moments.center_of_mass_position(),
			transformation * mass_moments.moment_of_inertia() * ~transformation, // TODO is this okay? <https://hepweb.ucsd.edu/ph110b/110b_notes/node24.html>
		};
	}


template<class Space>
	constexpr MassMoments<Space>
	operator+ (MassMoments<Space> a)
	{
		return a;
	}


// TODO perhaps just like velocity-moments, can't use +, but need add (other, arm) to correctly add moment_of_inertia?
// TODO and what would arm mean? Arm to center-of-mass, right?
template<class Space>
	inline MassMoments<Space>
	operator+ (SpaceVector<si::Length, Space> const& offset,
			   MassMoments<Space> const& mass_moments)
	{
		return {
			mass_moments.mass(),
			mass_moments.center_of_mass_position() + offset,
			parallel_axis_theorem (mass_moments.moment_of_inertia(), mass_moments.mass(), offset),
		};
	}


template<class Space>
	inline MassMoments<Space>
	operator+ (MassMoments<Space> const& mass_moments,
			   SpaceVector<si::Length, Space> const& offset)
	{
		return offset + mass_moments;
	}


template<class Space>
	inline MassMoments<Space>
	operator- (SpaceVector<si::Length, Space> const& offset,
			   MassMoments<Space> const& mass_moments)
	{
        return -offset + mass_moments;
    }


template<class Space>
	inline MassMoments<Space>
	operator- (MassMoments<Space> const& mass_moments,
			   SpaceVector<si::Length, Space> const& offset)
	{
        return -offset + mass_moments;
    }


/*
 * Global functions
 */


template<class Space>
	SpaceMatrix<si::MomentOfInertia, Space>
	parallel_axis_theorem_arm (si::Mass mass, SpaceVector<si::Length, Space> const& displacement)
	{
		SpaceMatrix<double, Space> const unit (math::unit);
		return mass * (unit * (~displacement * displacement).scalar() - displacement * ~displacement);
	}


template<class Space>
	SpaceMatrix<si::MomentOfInertia, Space>
	parallel_axis_theorem (SpaceMatrix<si::MomentOfInertia, Space> const& moi_at_com, si::Mass mass, SpaceVector<si::Length, Space> const& displacement)
	{
		return moi_at_com + parallel_axis_theorem_arm (mass, displacement);
	}


template<class Scalar, class Space>
	inline MassMoments<Space>
	calculate_mass_moments (std::vector<PlaneTriangle<Scalar, Space>> const& polygon_triangulation, si::Length const chord_length, si::Length const wing_length, si::Density const material_density)
	{
		// Have 2D triangulation points, make two sets of them, split the virtual wing into two identical-length parts,
		// make the points at the center of each wing part. This way we'll get correct MOI for all 3D axes.

		std::vector<PointMass<Space>> point_masses;
		point_masses.reserve (2 * polygon_triangulation.size());

		for (auto const& triangle: polygon_triangulation)
		{
			auto const s = chord_length;
			auto const centroid = s * triangle_centroid (triangle);
			auto const mass = 0.5 * wing_length * (s * s) * area_2d (triangle) * material_density;
			auto const position_1 = SpaceLength<Space> (centroid[0], centroid[1], 0.25 * wing_length);
			auto const position_2 = SpaceLength<Space> (centroid[0], centroid[1], 0.75 * wing_length);

			point_masses.push_back (PointMass<Space> { mass, position_1, math::zero });
			point_masses.push_back (PointMass<Space> { mass, position_2, math::zero });
		}

		return MassMoments<Space> (FromPointMassRange(), begin (point_masses), end (point_masses));
	}

} // namespace xf

#endif

