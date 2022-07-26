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

#ifndef XEFIS__SUPPORT__AERODYNAMICS__ANGLE_OF_ATTACK_H__INCLUDED
#define XEFIS__SUPPORT__AERODYNAMICS__ANGLE_OF_ATTACK_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace xf {

struct AngleOfAttack
{
	si::Angle	alpha	{ 0_deg };
	si::Angle	beta	{ 0_deg };
};

} // namespace xf

#endif

