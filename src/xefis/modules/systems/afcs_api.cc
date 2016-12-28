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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "afcs_api.h"


namespace afcs_api {

std::string
to_string (ThrustMode mode)
{
	switch (mode)
	{
		case ThrustMode::None:			return "None";
		case ThrustMode::TO_GA:			return "TO/GA";
		case ThrustMode::Continuous:	return "CONT";
		case ThrustMode::Idle:			return "IDLE";
		case ThrustMode::KIAS:			return "KIAS";
		case ThrustMode::Mach:			return "MACH";
	}

	return "";
}


std::string
to_string (RollMode mode)
{
	switch (mode)
	{
		case RollMode::None:			return "None";
		case RollMode::Heading:			return "HDG";
		case RollMode::Track:			return "TRK";
		case RollMode::WingsLevel:		return "WNG LVL";
		case RollMode::Localizer:		return "LOC";
		case RollMode::LNAV:			return "LNAV";
	}

	return "";
}


std::string
to_string (PitchMode mode)
{
	switch (mode)
	{
		case PitchMode::None:			return "None";
		case PitchMode::TO_GA:			return "TO/GA";
		case PitchMode::KIAS:			return "KIAS";
		case PitchMode::Mach:			return "MACH";
		case PitchMode::Altitude:		return "ALT";
		case PitchMode::VS:				return "V/S";
		case PitchMode::FPA:			return "FPA";
		case PitchMode::VNAVPath:		return "VNAV PTH";
		case PitchMode::GS:				return "G/S";
		case PitchMode::Flare:			return "FLARE";
	}

	return "";
}

} // namespace afcs_api

