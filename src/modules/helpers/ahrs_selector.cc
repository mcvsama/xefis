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
#include <cmath>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>

// Local:
#include "ahrs_selector.h"


XEFIS_REGISTER_MODULE_CLASS ("helpers/ahrs-selector", AHRSSelector)


AHRSSelector::AHRSSelector (v1::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	v1::ConfigReader::PropertiesParser::PropertiesList properties_list;

	parse_settings (config, {
		{ "pitch-disagree-threshold", _setting_pitch_disagree_threshold, true },
		{ "roll-disagree-threshold", _setting_roll_disagree_threshold, true },
		{ "magnetic-heading-disagree-threshold", _setting_magnetic_heading_disagree_threshold, true },
	});

	for (std::size_t i = 0; i < MaxInputs; ++i)
	{
		QString i_str = QString ("%1").arg (i);

#define XEFIS_SELECTOR_DEF_PROP(str, name) \
		properties_list.emplace_back ("input." + i_str + "." str, _inputs_##name[i], false)

		XEFIS_SELECTOR_DEF_PROP ("serviceable", serviceable);
		XEFIS_SELECTOR_DEF_PROP ("orientation.pitch", orientation_pitch);
		XEFIS_SELECTOR_DEF_PROP ("orientation.roll", orientation_roll);
		XEFIS_SELECTOR_DEF_PROP ("orientation.heading.magnetic", orientation_magnetic_heading);

#undef XEFIS_SELECTOR_DEF_PROP
	}

	properties_list.insert (properties_list.end(), {
		{ "output.serviceable", _output_serviceable, true },
		{ "output.orientation.pitch", _output_orientation_pitch, true },
		{ "output.orientation.roll", _output_orientation_roll, true },
		{ "output.orientation.heading.magnetic", _output_orientation_magnetic_heading, true },
		{ "output.flags.pitch-disagree", _output_pitch_disagree_flag, true },
		{ "output.flags.roll-disagree", _output_roll_disagree_flag, true },
		{ "output.flags.magnetic-heading-disagree", _output_magnetic_heading_disagree_flag, true },
		{ "output.flags.failover", _output_failover_flag, true },
	});

	parse_properties (config, properties_list);
}


void
AHRSSelector::data_updated()
{
	bool ok = false;

	for (std::size_t i = 0; i < MaxInputs; ++i)
	{
		if (_inputs_serviceable[i].read (false) &&
			_inputs_orientation_pitch[i].valid() &&
			_inputs_orientation_roll[i].valid() &&
			_inputs_orientation_magnetic_heading[i].valid())
		{
			_output_serviceable = true;
			copy_if_different (_inputs_orientation_pitch[i], _output_orientation_pitch);
			copy_if_different (_inputs_orientation_roll[i], _output_orientation_roll);
			copy_if_different (_inputs_orientation_magnetic_heading[i], _output_orientation_magnetic_heading);
			ok = true;
			break;
		}
	}

	_output_failover_flag = !ok;

	if (!ok)
	{
		reset_all();
		_output_serviceable = false;
	}
	else
	{
		// Check disagreements:
		_output_pitch_disagree_flag = compute_disagree_flag (_inputs_orientation_pitch[0],
															 _inputs_orientation_pitch[1],
															 _setting_pitch_disagree_threshold);
		_output_roll_disagree_flag = compute_disagree_flag (_inputs_orientation_roll[0],
															_inputs_orientation_roll[1],
															_setting_roll_disagree_threshold);
		_output_magnetic_heading_disagree_flag = compute_disagree_flag (_inputs_orientation_magnetic_heading[0],
																		_inputs_orientation_magnetic_heading[1],
																		_setting_magnetic_heading_disagree_threshold);
	}

	_output_serviceable = std::any_of (_inputs_serviceable.begin(), _inputs_serviceable.end(), [](v1::PropertyBoolean const& x) { return *x; });
}


void
AHRSSelector::reset_all()
{
	_output_serviceable.set_nil();
	_output_orientation_pitch.set_nil();
	_output_orientation_roll.set_nil();
	_output_orientation_magnetic_heading.set_nil();
	_output_pitch_disagree_flag.set_nil();
	_output_roll_disagree_flag.set_nil();
	_output_magnetic_heading_disagree_flag.set_nil();
}


template<class PropertyType>
	inline void
	AHRSSelector::copy_if_different (PropertyType& input_property, PropertyType& output_property)
	{
		if (*input_property != *output_property || output_property.is_nil())
			output_property = *input_property;
	}


inline bool
AHRSSelector::compute_disagree_flag (v1::PropertyAngle const& first, v1::PropertyAngle const& second, Angle threshold)
{
	using std::abs;
	return !first.valid() || !second.valid() || abs (*first - *second) > threshold;
}

