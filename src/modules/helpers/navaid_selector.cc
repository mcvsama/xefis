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
#include <xefis/utility/numeric.h>

// Local:
#include "navaid_selector.h"


XEFIS_REGISTER_MODULE_CLASS ("helpers/navaid-selector", NavaidSelector)


NavaidSelector::NavaidSelector (v1::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	v1::ConfigReader::PropertiesParser::PropertiesList properties_list;

	for (std::size_t i = 0; i < MaxInputs; ++i)
	{
		QString i_str = QString ("%1").arg (i);

#define XEFIS_SELECTOR_DEF_PROP(str, name) \
		properties_list.emplace_back ("input." + i_str + "." str, _inputs_##name[i], false)

		XEFIS_SELECTOR_DEF_PROP ("type", type);
		XEFIS_SELECTOR_DEF_PROP ("reference", reference);
		XEFIS_SELECTOR_DEF_PROP ("identifier", identifier);
		XEFIS_SELECTOR_DEF_PROP ("radial.magnetic", radial_magnetic);
		XEFIS_SELECTOR_DEF_PROP ("reciprocal.magnetic", reciprocal_magnetic);
		XEFIS_SELECTOR_DEF_PROP ("initial-bearing.magnetic", initial_bearing_magnetic);
		XEFIS_SELECTOR_DEF_PROP ("distance", distance);
		XEFIS_SELECTOR_DEF_PROP ("eta", eta);
		XEFIS_SELECTOR_DEF_PROP ("deviation", deviation);
		XEFIS_SELECTOR_DEF_PROP ("to-flag", to_flag);

#undef XEFIS_SELECTOR_DEF_PROP
	}

	properties_list.insert (properties_list.end(), {
		{ "input.selected", _selected_input, true },
		{ "output.type", _output_type, true },
		{ "output.reference", _output_reference, true },
		{ "output.identifier", _output_identifier, true },
		{ "output.radial.magnetic", _output_radial_magnetic, true },
		{ "output.reciprocal.magnetic", _output_reciprocal_magnetic, true },
		{ "output.initial-bearing.magnetic", _output_initial_bearing_magnetic, true },
		{ "output.distance", _output_distance, true },
		{ "output.eta", _output_eta, true },
		{ "output.deviation", _output_deviation, true },
		{ "output.to-flag", _output_to_flag, true },
	});

	parse_properties (config, properties_list);
}


void
NavaidSelector::data_updated()
{
	if (_selected_input.valid())
	{
		bool sel_fresh = _selected_input.fresh();
		auto input = *_selected_input;

		if (input < 0 || input > MaxInputs - 1)
			reset_all();
		else
		{
#define XEFIS_SELECTOR_COPY(name) \
		copy (sel_fresh, _inputs_##name[input], _output_##name)

			XEFIS_SELECTOR_COPY (type);
			XEFIS_SELECTOR_COPY (reference);
			XEFIS_SELECTOR_COPY (identifier);
			XEFIS_SELECTOR_COPY (radial_magnetic);
			XEFIS_SELECTOR_COPY (reciprocal_magnetic);
			XEFIS_SELECTOR_COPY (initial_bearing_magnetic);
			XEFIS_SELECTOR_COPY (distance);
			XEFIS_SELECTOR_COPY (eta);
			XEFIS_SELECTOR_COPY (deviation);
			XEFIS_SELECTOR_COPY (to_flag);

#undef XEFIS_SELECTOR_COPY
		}
	}
	else
		reset_all();
}


void
NavaidSelector::reset_all()
{
	_output_type.set_nil();
	_output_reference.set_nil();
	_output_identifier.set_nil();
	_output_radial_magnetic.set_nil();
	_output_reciprocal_magnetic.set_nil();
	_output_initial_bearing_magnetic.set_nil();
	_output_distance.set_nil();
	_output_eta.set_nil();
	_output_deviation.set_nil();
	_output_to_flag.set_nil();
}


template<class PropertyType>
	inline void
	NavaidSelector::copy (bool selector_fresh, PropertyType& input_property, PropertyType& output_property)
	{
		if (input_property.configured() && (selector_fresh || input_property.fresh()))
			output_property.copy_from (input_property);
	}

