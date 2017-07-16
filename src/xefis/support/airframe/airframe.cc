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
#include <xefis/core/v1/config_reader.h>
#include <xefis/core/stdexcept.h>
#include <xefis/core/xefis.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/qdom_iterator.h>

// Local:
#include "airframe.h"


namespace xf {

Airframe::Airframe (Xefis*, QDomElement const& config)
{
	QDomElement settings_element;

	if (!config.isNull())
	{
		for (QDomElement const& e: xf::iterate_sub_elements (config))
		{
			if (e == "flaps")
				_flaps = std::make_unique<Flaps> (e);
			else if (e == "spoilers")
				_spoilers = std::make_unique<Spoilers> (e);
			else if (e == "lift")
				_lift = std::make_unique<Lift> (e);
			else if (e == "drag")
				_drag = std::make_unique<Drag> (e);
			else if (e == "settings")
				settings_element = e;
		}

		if (settings_element.isNull())
			throw MissingDomElement (config, "settings");
		else
		{
			double min_g;
			double max_g;

			v1::ConfigReader::SettingsParser settings_parser ({
				{ "wings-area", _wings_area, true },
				{ "wings-chord", _wings_chord, true },
				{ "max-negative-load-factor", min_g, true },
				{ "max-positive-load-factor", max_g, true },
				{ "safe-aoa-correction", _safe_aoa_correction, true },
			});
			settings_parser.parse (settings_element);

			_load_factor_limits = { min_g, max_g };
		}

		_defined_aoa_range = lift().get_aoa_range().extended (drag().get_aoa_range());
	}
}


LiftCoefficient
Airframe::get_cl (Angle const& aoa, FlapsAngle const& flaps_angle, SpoilersAngle const& spoilers_angle) const
{
	Angle total_aoa = aoa + flaps().get_aoa_correction (flaps_angle.value()) + spoilers().get_aoa_correction (spoilers_angle.value());
	return LiftCoefficient (lift().get_cl (total_aoa));
}


DragCoefficient
Airframe::get_cd (Angle const& aoa, FlapsAngle const& flaps_angle, SpoilersAngle const& spoilers_angle) const
{
	Angle total_aoa = aoa + flaps().get_aoa_correction (flaps_angle.value()) + spoilers().get_aoa_correction (spoilers_angle.value());
	return DragCoefficient (drag().get_cd (total_aoa));
}


Optional<Angle>
Airframe::get_aoa_in_normal_regime (LiftCoefficient const& cl, FlapsAngle const& flaps_angle, SpoilersAngle const& spoilers_angle) const
{
	Optional<Angle> normal_aoa = lift().get_aoa_in_normal_regime (cl);
	if (normal_aoa)
	{
		Angle flaps_aoa_correction = flaps().get_aoa_correction (flaps_angle.value());
		Angle spoilers_aoa_correction = spoilers().get_aoa_correction (spoilers_angle.value());
		return *normal_aoa - flaps_aoa_correction - spoilers_aoa_correction;
	}
	else
		return { };
}


Angle
Airframe::get_critical_aoa (FlapsAngle const& flaps_angle, SpoilersAngle const& spoilers_angle) const
{
	Angle critical_aoa = lift().critical_aoa();
	critical_aoa -= flaps().find_setting (flaps_angle.value()).aoa_correction();
	critical_aoa += spoilers().find_setting (spoilers_angle.value()).aoa_correction();
	return critical_aoa;
}


Angle
Airframe::get_max_safe_aoa (FlapsAngle const& flaps_angle, SpoilersAngle const& spoilers_angle) const
{
	return get_critical_aoa (flaps_angle, spoilers_angle) + safe_aoa_correction();
}

} // namespace xf

