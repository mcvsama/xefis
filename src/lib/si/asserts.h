/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef SI__ASSERTS_H__INCLUDED
#define SI__ASSERTS_H__INCLUDED

#include <type_traits>

static_assert (std::is_literal_type<SI::Angle>::value, "SI::Angle is not literal type");
static_assert (std::is_literal_type<SI::Pressure>::value, "SI::Pressure is not literal type");
static_assert (std::is_literal_type<SI::Frequency>::value, "SI::Frequency is not literal type");
static_assert (std::is_literal_type<SI::Length>::value, "SI::Length is not literal type");
static_assert (std::is_literal_type<SI::Time>::value, "SI::Time is not literal type");
static_assert (std::is_literal_type<SI::Speed>::value, "SI::Speed is not literal type");
static_assert (std::is_literal_type<SI::LonLat>::value, "SI::LonLat is not literal type");
static_assert (std::is_literal_type<SI::Temperature>::value, "SI::Temperature is not literal type");

#endif

