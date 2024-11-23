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

#ifndef XEFIS__SUPPORT__MATH__COORDINATE_SYSTEMS_H__INCLUDED
#define XEFIS__SUPPORT__MATH__COORDINATE_SYSTEMS_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/math/concepts.h>

// Standard:
#include <cstddef>


namespace xf {

// Earth-centered Earth-fixed frame of reference:
class ECEFSpace: public neutrino::math::CoordinateSystemBase
{ };

// Local-tangent-plane frame of reference:
class NEDSpace: public neutrino::math::CoordinateSystemBase
{ };

// Simulated body frame of reference (X points to the front, Y to the right, Z down the body):
class AirframeSpace: public neutrino::math::CoordinateSystemBase
{ };

// Body space coordinate system, origin at body's center of mass:
struct BodyCOM: public math::CoordinateSystemBase
{ };

// Body space coordinate system, origin at body-origin:
struct BodyOrigin: public math::CoordinateSystemBase
{ };

// World (aka global aka absolute) space coordinate system:
struct WorldSpace: public math::CoordinateSystemBase
{ };

} // namespace xf

#endif

