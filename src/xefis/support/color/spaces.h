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

#ifndef XEFIS__SUPPORT__COLOR__SPACES_H__INCLUDED
#define XEFIS__SUPPORT__COLOR__SPACES_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/math/concepts.h>

// Standard:
#include <cstddef>


namespace xf {

class RGBSpace: public math::CoordinateSystemBase
{ };

} // namespace xf

#endif

