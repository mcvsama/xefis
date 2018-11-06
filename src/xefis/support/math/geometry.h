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

#ifndef XEFIS__SUPPORT__MATH__GEOMETRY_H__INCLUDED
#define XEFIS__SUPPORT__MATH__GEOMETRY_H__INCLUDED

// Standard:
#include <cstddef>
#include <cmath>

// Lib:
#include <lib/math/math.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/space.h>


namespace xf {

/**
 * Make a tensor W from vector v⃗, so that it acts as it was v⃗× operator:
 * v⃗ × Z = W * Z.
 */
template<class S, class TF = void, class SF = TF>
	SpaceMatrix<S, TF, SF>
	make_pseudotensor (SpaceVector<S, TF> const& v)
	{
		return {
			    0, -v[2], +v[1],
			+v[2],     0, -v[0],
			-v[1], +v[0],     0,
		};
	}


template<class S, class TF = void, class SF = TF>
	SpaceMatrix<S, TF, SF>
	make_diagonal_matrix (SpaceVector<S, TF> const& v)
	{
		return {
			v[0],    0,    0,
			   0, v[1],    0,
			   0,    0, v[2],
		};
	}


/**
 * Normalize vectors in matrix.
 * Use for orientation matrices.
 */
template<class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr void
	normalize_vectors (math::Matrix<S, C, R, TF, SF>& matrix)
	{
		for (std::size_t c = 0; c < C; ++c)
		{
			auto const v_norm = abs (matrix.column (c));

			for (std::size_t r = 0; r < R; ++r)
				matrix (c, r) /= si::quantity (v_norm);
		}
	}


/**
 * Normalize vectors in matrix.
 * Use for orientation matrices.
 */
template<class S, std::size_t C, std::size_t R, class TF, class SF>
	constexpr math::Matrix<S, C, R, TF, SF>
	vector_normalized (math::Matrix<S, C, R, TF, SF> matrix)
	{
		normalize_vectors (matrix);
		return matrix;
	}


/**
 * Return vector orthogonalized onto another vector.
 */
template<class S, class F>
	SpaceVector<S, F>
	orthogonalized (SpaceVector<S, F> const& vector, SpaceVector<S, F> const& onto)
	{
		return vector - (static_cast<S> (~vector * onto) * onto / square (abs (onto)));
	}


/**
 * Make matrix orthogonal so that X stays unchanged.
 */
template<class S, class TF, class SF>
	SpaceMatrix<S, TF, SF>
	orthogonalized (SpaceMatrix<S, TF, SF> const& m)
	{
		auto const new_y = orthogonalized (m.column (1), m.column (0));
		auto const new_z = cross_product (m.column (0), new_y);

		return { m.column (0), new_y, new_z };
	}


template<class T, class F>
	SpaceVector<T, F>
	length_limited (SpaceVector<T, F> vector, T const& max_length)
	{
		auto const length = abs (vector);

		if (length > max_length)
			vector = vector * max_length / length; // TODO *=

		return vector;
	}


template<class T, class F>
	auto
	normalized (SpaceVector<T, F> const& vector)
	{
		return vector / abs (vector);
	}


template<class T, class F>
	// TODO check types?
	SpaceVector<T, F>
	projection (SpaceVector<T, F> const& vector, SpaceVector<T, F> const& onto)
	{
		return (~vector * normalized (onto)) * onto;
	}

} // namespace xf

#endif

