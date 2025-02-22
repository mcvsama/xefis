/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__MATH__ALGORITHMS_H__INCLUDED
#define XEFIS__SUPPORT__MATH__ALGORITHMS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <concepts>
#include <cmath>
#include <cstddef>
#include <optional>
#include <utility>


namespace xf {

/**
 * Solve the quadratic equation a * x^2 + b * x + c = 0 using a numerically stable method.
 *
 * \tparam	Value Numeric type for the coefficients and solutions.
 * \param	a Quadratic coefficient.
 * \param	b Linear coefficient.
 * \param	c Constant term.
 * \return	Optional pair containing the two real solutions if they exist; std::nullopt if no real solutions exist.
 *
 * This function accounts for the special case when b is zero, which corresponds to the scenario
 * where the ray direction is perpendicular to the vector from the ray origin to the sphere centre.
 * For non-zero b, it computes the discriminant and uses a stable formulation to determine the roots.
 */
template<std::floating_point Value>
	inline std::optional<std::pair<Value, Value>>
	solve_quadratic (Value const a, Value const b, Value const c)
	{
		if (b == 0.0)
		{
			// When ray direction and vector from origin to the sphere center are perpendicular:
			if (a == 0.0)
				return std::nullopt;

			return {{ 0.0, std::sqrt (-c / a) }};
		}
		else
		{
			auto discr = b * b - 4 * a * c;

			if (discr < 0.0)
				return std::nullopt;

			auto q = (b < 0.0)
				? -0.5 * (b - std::sqrt (discr))
				: -0.5 * (b + std::sqrt (discr));

			return {{ q / a, c / q }};
		}
	}

} // namespace xf

#endif

