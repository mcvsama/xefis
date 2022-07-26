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

#ifndef XEFIS__SUPPORT__AIRFRAME__TYPES_H__INCLUDED
#define XEFIS__SUPPORT__AIRFRAME__TYPES_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/strong_type.h>

// Standard:
#include <cstddef>


namespace xf {

using LiftCoefficient	= double;
using DragCoefficient	= double;
using FlapsAngle		= StrongType<si::Angle, struct FlapsAngleType>;
using SpoilersAngle		= StrongType<si::Angle, struct SpoilersAngleType>;

} // namespace xf

#endif

