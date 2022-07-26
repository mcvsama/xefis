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
#include "airframe.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/qt/qdom.h>
#include <neutrino/qt/qdom_iterator.h>
#include <neutrino/stdexcept.h>

// Standard:
#include <cstddef>


namespace xf {

Airframe::Airframe (AirframeDefinition definition):
	_definition (definition)
{
	_defined_aoa_range = lift().get_aoa_range().extended (drag().get_aoa_range());
}


LiftCoefficient
Airframe::get_cl (si::Angle const aoa, FlapsAngle const flaps_angle, SpoilersAngle const spoilers_angle) const
{
	si::Angle total_aoa = aoa + flaps().get_aoa_correction (flaps_angle) + spoilers().get_aoa_correction (spoilers_angle);
	return LiftCoefficient (lift().get_cl (total_aoa));
}


DragCoefficient
Airframe::get_cd (si::Angle const aoa, FlapsAngle const flaps_angle, SpoilersAngle const spoilers_angle) const
{
	si::Angle total_aoa = aoa + flaps().get_aoa_correction (flaps_angle) + spoilers().get_aoa_correction (spoilers_angle);
	return DragCoefficient (drag().get_cd (total_aoa));
}


std::optional<si::Angle>
Airframe::get_aoa_in_normal_regime (LiftCoefficient const cl, FlapsAngle const flaps_angle, SpoilersAngle const spoilers_angle) const
{
	std::optional<si::Angle> normal_aoa = lift().get_aoa_in_normal_regime (cl);

	if (normal_aoa)
	{
		si::Angle flaps_aoa_correction = flaps().get_aoa_correction (flaps_angle);
		si::Angle spoilers_aoa_correction = spoilers().get_aoa_correction (spoilers_angle);
		return *normal_aoa - flaps_aoa_correction - spoilers_aoa_correction;
	}
	else
		return std::nullopt;
}


si::Angle
Airframe::get_critical_aoa (FlapsAngle const flaps_angle, SpoilersAngle const spoilers_angle) const
{
	si::Angle critical_aoa = lift().critical_aoa();
	critical_aoa -= flaps().find_setting (flaps_angle).aoa_correction();
	critical_aoa += spoilers().find_setting (spoilers_angle).aoa_correction();
	return critical_aoa;
}


si::Angle
Airframe::get_max_safe_aoa (FlapsAngle const flaps_angle, SpoilersAngle const spoilers_angle) const
{
	return get_critical_aoa (flaps_angle, spoilers_angle) + safe_aoa_correction();
}

} // namespace xf

