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

#ifndef XEFIS__SUPPORT__MATH__CONCEPTS_H__INCLUDED
#define XEFIS__SUPPORT__MATH__CONCEPTS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/math/concepts.h>

// Standard:
#include <concepts>
#include <cstddef>
#include <type_traits>


namespace xf {

template<class P>
	concept PointConcept = requires (P p) {
		p[0];
		p[1]; // TODO -> si::FloatingPointOrQuantity<std::remove_cvref_t<decltype (p[1])>>;
	};


template<class T>
	concept TriangleConcept = requires (T t) {
		{ t[0] } -> PointConcept;
		{ t[1] } -> PointConcept;
		{ t[2] } -> PointConcept;
		{ std::size (t) } -> std::same_as<std::size_t>;
	};

} // namespace xf

#endif

