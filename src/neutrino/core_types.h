/* vim:ts=4
 *
 * Copyleft 2012…2016  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef NEUTRINO__CORE_TYPES_H__INCLUDED
#define NEUTRINO__CORE_TYPES_H__INCLUDED

// Standard:
#include <stdint.h>
#include <string>
#include <vector>
#include <memory>
#include <optional>

// Qt: TODO move to support/qt
#include <QtCore/QString>

// Neutrino:
#include <neutrino/si/si.h>


// TODO remove this using namespaces
using namespace si;
using namespace si::units;
using namespace si::quantities;
using namespace si::literals;


// TODO move to support/qt
inline QString
operator"" _qstr (const char* string, size_t)
{
    return QString (string);
}


constexpr std::size_t
operator"" _bit (unsigned long long bits)
{
	return 1ull << bits;
}


using float32_t		= float;
using float64_t		= double;
using float128_t	= long double;

using Blob			= std::vector<uint8_t>;

// C-compatibility:
typedef bool _Bool;

#endif

