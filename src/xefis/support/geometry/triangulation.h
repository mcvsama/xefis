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

#ifndef XEFIS__SUPPORT__GEOMETRY__TRIANGULATION_H__INCLUDED
#define XEFIS__SUPPORT__GEOMETRY__TRIANGULATION_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/math/geometry.h>

// Standard:
#include <algorithm>
#include <array>
#include <complex>
#include <cstddef>
#include <list>
#include <type_traits>


namespace xf {

/**
 * \param	begin, end
 *			Sequence of PlaneVector points.
 *			Must define a simple polygon without holes in CCW direction (assuming inside is on the left side when walking
 *			throught the points).
 */
template<class Scalar, class Space, class Iterator>
	inline std::vector<PlaneTriangle<Scalar, Space>>
	triangulate (Iterator const vertices_begin, Iterator const vertices_end)
		requires (std::is_same_v<std::remove_cvref_t<decltype (*std::declval<Iterator>())>, PlaneVector<Scalar, Space>>)
	{
		using Vertex = PlaneVector<Scalar, Space>;
		using Triangle = PlaneTriangle<Scalar, Space>;
		using VertexList = std::list<Vertex>;

		auto const circular_prev = [](typename VertexList::iterator v, VertexList& list)
			-> typename VertexList::iterator
		{
			if (v == begin (list))
				return prev (end (list));
			else
				return prev (v);
		};

		auto const circular_next = [](typename VertexList::iterator v, VertexList& list)
			-> typename VertexList::iterator
		{
			auto n = next (v);

			if (n == end (list))
				return begin (list);
			else
				return n;
		};

		// Returns positive angles for right-turn, negative for left-turn:
		auto const turn = [](Vertex const& v1, Vertex const& v2, Vertex const& v3)
			-> si::Angle
		{
			auto const v2_sub_v1 = v2 - v1;
			auto const v3_sub_v1 = v3 - v1;
			auto const v12 = std::complex (v2_sub_v1[0], v2_sub_v1[1]);
			auto const v13 = std::complex (v3_sub_v1[0], v3_sub_v1[1]);
			auto const a12 = 1_rad * std::arg (v12);
			auto const a13 = 1_rad * std::arg (v13);
			auto const a_range = neutrino::Range<si::Angle> (-180_deg, +180_deg);

			return neutrino::floored_mod (a12 - a13, a_range);
		};

		auto const is_convex = [&turn] (Vertex const& v1, Vertex const& v2, Vertex const& v3)
			-> bool
		{
			return turn (v1, v2, v3) <= 0_rad;
		};

		auto const any_vertex_inside_triangle = [](VertexList const& list, Vertex const& v1, Vertex const& v2, Vertex const& v3)
			-> bool
		{
			auto const is_inside = is_point_2d_inside_triangle_tester (std::array { v1, v2, v3 });

			for (auto const& v: list)
				if (v != v1 && v != v2 && v != v3)
					if (is_inside (v))
						return true;

			return false;
		};

		VertexList vertices (vertices_begin, vertices_end);
		std::vector<Triangle> result;

		size_t test_count = 0;
		auto v = begin (vertices);

		// Find an ear and remove it multiple times:
		while (vertices.size() > 3 && test_count < vertices.size())
		{
			auto vp = circular_prev (v, vertices);
			auto vn = circular_next (v, vertices);

			if (is_convex (*vp, *v, *vn))
			{
				// Check if no other vertex is inside triangle [p_prev, p, p_next]
				if (!any_vertex_inside_triangle (vertices, *vp, *v, *vn))
				{
					test_count = 0;
					result.push_back ({ *vp, *v, *vn });
					v = vertices.erase (v);

					if (v == vertices.end())
						v = vertices.begin();
				}
				else
					v = circular_next (v, vertices);
			}
			else
				v = circular_next (v, vertices);

			++test_count;
		}

		if (test_count < vertices.size())
		{
			// The last remaining triangle:
			v = begin (vertices);
			auto vn1 = next (v);
			auto vn2 = next (vn1);
			result.push_back ({ *v, *vn1, *vn2 });

			return result;
		}
		else
			return {};
	}

} // namespace xf

#endif

