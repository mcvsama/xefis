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

#ifndef XEFIS__MODULES__SYSTEMS__AFCS_API_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__AFCS_API_H__INCLUDED

// Standard:
#include <cstddef>


namespace afcs_api {

enum class ThrustMode
{
	// No thrust control.
	None,
	// Maximum temporary thrust.
	TO_GA,
	// Maximum continuous thrust.
	Continuous,
	// Minimum thrust.
	Idle,
	// Try to follow speed set in CMD KIAS property.
	KIAS,
	// Try to follow speed set in CMD Mach property.
	Mach,
};

enum class RollMode
{
	// No lateral movement control.
	None,
	// Follow heading from CMD HDG property.
	Heading,
	// Follow heading from CMD TRK property.
	Track,
	// Fly laterally with wings level. Don't hold onto any heading.
	WingsLevel,
	// Fly laterally according to heading supplied by LOC input
	// (ILS, VOR or whatever).
	Localizer,
	// Fly laterally according to heading supplied by LNAV module.
	LNAV,
};

enum class PitchMode
{
	// No vertical movement control.
	None,
	// Pitch for TO/GA.
	TO_GA,
	// Control airspeed to match value from CMD IAS property.
	KIAS,
	// Control airspeed to match value from CMD Mach property.
	Mach,
	// Control altitude to match value provided in CMD ALT property.
	Altitude,
	// Control vertical speed to match value provided in CMD VS property.
	VS,
	// Control vertical speed to match value provided in CMD FPA property.
	FPA,
	// Control FPA to follow path provided by the VNAV module.
	VNAVPath,
	// Control FPA to follow path provided by the G/S input.
	GS,
	// Control pitch to flare the aircraft before touchdown.
	Flare,
};


std::string
to_string (ThrustMode);


std::string
to_string (RollMode);


std::string
to_string (PitchMode);

} // namespace afcs_api

#endif
