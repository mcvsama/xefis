/* vim:ts=4
 *
 * Copyleft 2019  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__SIMULATION__RIGID_BODY__FRAMES_H__INCLUDED
#define XEFIS__SUPPORT__SIMULATION__RIGID_BODY__FRAMES_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>


namespace xf::rigid_body {

// World (aka global aka absolute) frame of reference:
struct WorldSpace;

// Frame of reference used by bodies (center-of-mass frame):
struct BodySpace;


template<class T>
	concept FrameConcept = std::is_same_v<T, WorldSpace> || std::is_same_v<T, BodySpace>;

} // namespace xf::rigid_body

#endif

