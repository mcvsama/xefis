/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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


XEFIS_REGISTER_MODULE_CLASS ("helpers/navaid-selector", NavaidSelector);


NavaidSelector::NavaidSelector (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_properties (config, {
		{ "input.selected", _selected_input, true },
		{ "input.left.reference", _input_l_reference, true },
		{ "input.left.identifier", _input_l_identifier, true },
		{ "input.left.radial.magnetic", _input_l_radial_magnetic, true },
		{ "input.left.reciprocal.magnetic", _input_l_reciprocal_magnetic, true },
		{ "input.left.distance", _input_l_distance, true },
		{ "input.left.eta", _input_l_eta, true },
		{ "input.right.reference", _input_r_reference, true },
		{ "input.right.identifier", _input_r_identifier, true },
		{ "input.right.radial.magnetic", _input_r_radial_magnetic, true },
		{ "input.right.reciprocal.magnetic", _input_r_reciprocal_magnetic, true },
		{ "input.right.distance", _input_r_distance, true },
		{ "input.right.eta", _input_r_eta, true },
		{ "output.reference", _output_reference, true },
		{ "output.identifier", _output_identifier, true },
		{ "output.radial.magnetic", _output_radial_magnetic, true },
		{ "output.reciprocal.magnetic", _output_reciprocal_magnetic, true },
		{ "output.distance", _output_distance, true },
		{ "output.eta", _output_eta, true },
	});
}


void
NavaidSelector::data_updated()
{
	if (_selected_input.valid())
	{
		int input = Xefis::limit (*_selected_input, 0L, 1L);
		bool sel_fresh = _selected_input.fresh();
#define XEFIS_SELECTOR_COPY(name) copy (sel_fresh, input, _input_l_##name, _input_r_##name, _output_##name)
		XEFIS_SELECTOR_COPY (reference);
		XEFIS_SELECTOR_COPY (identifier);
		XEFIS_SELECTOR_COPY (radial_magnetic);
		XEFIS_SELECTOR_COPY (reciprocal_magnetic);
		XEFIS_SELECTOR_COPY (distance);
		XEFIS_SELECTOR_COPY (eta);
#undef XEFIS_SELECTOR_COPY
	}
	else
	{
		_output_reference.set_nil();
		_output_identifier.set_nil();
		_output_radial_magnetic.set_nil();
		_output_reciprocal_magnetic.set_nil();
		_output_distance.set_nil();
		_output_eta.set_nil();
	}
}


template<class PropertyType>
	inline void
	NavaidSelector::copy (bool selector_fresh, int input, PropertyType& left, PropertyType& right, PropertyType& output)
	{
		if (input == 0)//&& (left.fresh() || selector_fresh))
			output.copy (left);
		else// if (right.fresh() || selector_fresh)
			output.copy (right);
	}

