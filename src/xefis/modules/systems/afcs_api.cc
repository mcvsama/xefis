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

// Local:
#include "afcs_api.h"

// Xefis:
#include <xefis/config/all.h>

// Standard:
#include <cstddef>


namespace afcs {

void
parse (std::string_view const& str, ThrustMode& thrust_mode)
{
	if (str == kThrustMode_TO_GA)
		thrust_mode = ThrustMode::TO_GA;
	else if (str == kThrustMode_Continuous)
		thrust_mode = ThrustMode::Continuous;
	else if (str == kThrustMode_Idle)
		thrust_mode = ThrustMode::Idle;
	else if (str == kThrustMode_KIAS)
		thrust_mode = ThrustMode::KIAS;
	else if (str == kThrustMode_Mach)
		thrust_mode = ThrustMode::Mach;
	else if (str == "")
		thrust_mode = ThrustMode::None;
}


void
parse (std::string_view const& str, RollMode& roll_mode)
{
	if (str == kRollMode_Heading)
		roll_mode = RollMode::Heading;
	else if (str == kRollMode_Track)
		roll_mode = RollMode::Track;
	else if (str == kRollMode_WingsLevel)
		roll_mode = RollMode::WingsLevel;
	else if (str == kRollMode_Localizer)
		roll_mode = RollMode::Localizer;
	else if (str == kRollMode_LNAV)
		roll_mode = RollMode::LNAV;
	else if (str == "")
		roll_mode = RollMode::None;
}


void
parse (std::string_view const& str, PitchMode& pitch_mode)
{
	if (str == kPitchMode_TO_GA)
		pitch_mode = PitchMode::TO_GA;
	else if (str == kPitchMode_KIAS)
		pitch_mode = PitchMode::KIAS;
	else if (str == kPitchMode_Mach)
		pitch_mode = PitchMode::Mach;
	else if (str == kPitchMode_Altitude)
		pitch_mode = PitchMode::Altitude;
	else if (str == kPitchMode_VS)
		pitch_mode = PitchMode::VS;
	else if (str == kPitchMode_FPA)
		pitch_mode = PitchMode::FPA;
	else if (str == kPitchMode_VNAVPath)
		pitch_mode = PitchMode::VNAVPath;
	else if (str == kPitchMode_GS)
		pitch_mode = PitchMode::GS;
	else if (str == kPitchMode_Flare)
		pitch_mode = PitchMode::Flare;
	else if (str == "")
		pitch_mode = PitchMode::None;
}


void
parse (std::string_view const& str, SpeedMode& speed_mode)
{
	if (str == kSpeedMode_Thrust)
		speed_mode = SpeedMode::Thrust;
	else if (str == kSpeedMode_Airspeed)
		speed_mode = SpeedMode::Airspeed;
	else if (str == "")
		speed_mode = SpeedMode::None;
}

} // namespace afcs

