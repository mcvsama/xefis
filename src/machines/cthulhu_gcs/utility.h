/* vim:ts=4
 *
 * Copyleft 2008…2017  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MACHINES__CTHULHU_GCS__UTILITY_H__INCLUDED
#define XEFIS__MACHINES__CTHULHU_GCS__UTILITY_H__INCLUDED

// Standard:
#include <cstddef>
#include <optional>
#include <cmath>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>


inline bool
compute_disagree_flag (std::optional<si::Angle> const& first, std::optional<si::Angle> const& second, si::Angle threshold)
{
	using std::abs;

	return !first || !second || abs (*first - *second) > threshold;
}

#endif

