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

#ifndef XEFIS__SUPPORT__ATMOSPHERIC_MODEL_H__INCLUDED
#define XEFIS__SUPPORT__ATMOSPHERIC_MODEL_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/air/air.h>
#include <xefis/support/math/geometry.h>

// Standard:
#include <cstddef>


namespace xf {

/**
 * General atmosphere model.
 */
class Atmosphere
{
  public:
	// Dtor
	virtual
	~Atmosphere() = default;

	[[nodiscard]]
	virtual Air<ECEFSpace>
	air_at (SpaceVector<si::Length, ECEFSpace> const& position) const = 0;
};

} // namespace xf

#endif

