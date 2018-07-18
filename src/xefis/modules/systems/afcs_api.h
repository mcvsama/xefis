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

enum class SpeedMode
{
	// Manual A/T setting.
	None		= 0,
	// Maintain constant thrust.
	Thrust		= 1,
	// Maintain constant airspeed.
	Airspeed	= 2,
};


static constexpr std::string_view	kThrustMode_None		= "None";
static constexpr std::string_view	kThrustMode_TO_GA		= "TO/GA";
static constexpr std::string_view	kThrustMode_Continuous	= "CONT";
static constexpr std::string_view	kThrustMode_Idle		= "IDLE";
static constexpr std::string_view	kThrustMode_KIAS		= "KIAS";
static constexpr std::string_view	kThrustMode_Mach		= "MACH";

static constexpr std::string_view	kRollMode_None			= "None";
static constexpr std::string_view	kRollMode_Heading		= "HDG";
static constexpr std::string_view	kRollMode_Track			= "TRK";
static constexpr std::string_view	kRollMode_WingsLevel	= "WNG LVL";
static constexpr std::string_view	kRollMode_Localizer		= "LOC";
static constexpr std::string_view	kRollMode_LNAV			= "LNAV";

static constexpr std::string_view	kPitchMode_None			= "None";
static constexpr std::string_view	kPitchMode_TO_GA		= "TO/GA";
static constexpr std::string_view	kPitchMode_KIAS			= "KIAS";
static constexpr std::string_view	kPitchMode_Mach			= "MACH";
static constexpr std::string_view	kPitchMode_Altitude		= "ALT";
static constexpr std::string_view	kPitchMode_VS			= "V/S";
static constexpr std::string_view	kPitchMode_FPA			= "FPA";
static constexpr std::string_view	kPitchMode_VNAVPath		= "VNAV PTH";
static constexpr std::string_view	kPitchMode_GS			= "G/S";
static constexpr std::string_view	kPitchMode_Flare		= "FLARE";

static constexpr std::string_view	kSpeedMode_None			= "None";
static constexpr std::string_view	kSpeedMode_Thrust		= "Thrust";
static constexpr std::string_view	kSpeedMode_Airspeed		= "Airspeed";


constexpr std::string_view
to_string (ThrustMode mode)
{
	switch (mode)
	{
		case ThrustMode::None:			return kThrustMode_None;
		case ThrustMode::TO_GA:			return kThrustMode_TO_GA;
		case ThrustMode::Continuous:	return kThrustMode_Continuous;
		case ThrustMode::Idle:			return kThrustMode_Idle;
		case ThrustMode::KIAS:			return kThrustMode_KIAS;
		case ThrustMode::Mach:			return kThrustMode_Mach;
	}

	return "";
}


constexpr std::string_view
to_string (RollMode mode)
{
	switch (mode)
	{
		case RollMode::None:			return kRollMode_None;
		case RollMode::Heading:			return kRollMode_Heading;
		case RollMode::Track:			return kRollMode_Track;
		case RollMode::WingsLevel:		return kRollMode_WingsLevel;
		case RollMode::Localizer:		return kRollMode_Localizer;
		case RollMode::LNAV:			return kRollMode_LNAV;
	}

	return "";
}


constexpr std::string_view
to_string (PitchMode mode)
{
	switch (mode)
	{
		case PitchMode::None:			return kPitchMode_None;
		case PitchMode::TO_GA:			return kPitchMode_TO_GA;
		case PitchMode::KIAS:			return kPitchMode_KIAS;
		case PitchMode::Mach:			return kPitchMode_Mach;
		case PitchMode::Altitude:		return kPitchMode_Altitude;
		case PitchMode::VS:				return kPitchMode_VS;
		case PitchMode::FPA:			return kPitchMode_FPA;
		case PitchMode::VNAVPath:		return kPitchMode_VNAVPath;
		case PitchMode::GS:				return kPitchMode_GS;
		case PitchMode::Flare:			return kPitchMode_Flare;
	}

	return "";
}


constexpr std::string_view
to_string (SpeedMode mode)
{
	switch (mode)
	{
		case SpeedMode::None:			return kSpeedMode_None;
		case SpeedMode::Thrust:			return kSpeedMode_Thrust;
		case SpeedMode::Airspeed:		return kSpeedMode_Airspeed;
	}

	return "";
}


void
parse (std::string_view const&, ThrustMode&);


void
parse (std::string_view const&, RollMode&);


void
parse (std::string_view const&, PitchMode&);


void
parse (std::string_view const&, SpeedMode&);

} // namespace afcs_api

#endif
