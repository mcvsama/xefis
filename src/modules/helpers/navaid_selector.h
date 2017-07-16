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

#ifndef XEFIS__MODULES__HELPERS__NAVAID_SELECTOR_H__INCLUDED
#define XEFIS__MODULES__HELPERS__NAVAID_SELECTOR_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/module.h>
#include <xefis/core/v1/property.h>


/**
 * Select one of many navaids.
 */
class NavaidSelector: public v1::Module
{
	static constexpr int MaxInputs = 8;

  public:
	// Ctor
	NavaidSelector (v1::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private:
	/**
	 * Set all output properties to nil.
	 */
	void
	reset_all();

	/**
	 * Copy property to the output property, if input property is fresh,
	 * or selector property is fresh (selector_fresh is true).
	 */
	template<class PropertyType>
		static void
		copy (bool selector_fresh, PropertyType& input_property, PropertyType& output_property);

  private:
	// Input:
	v1::PropertyInteger							_selected_input;
	std::array<v1::PropertyInteger, MaxInputs>	_inputs_type;
	std::array<v1::PropertyString, MaxInputs>	_inputs_reference;
	std::array<v1::PropertyString, MaxInputs>	_inputs_identifier;
	std::array<v1::PropertyAngle, MaxInputs>	_inputs_radial_magnetic;
	std::array<v1::PropertyAngle, MaxInputs>	_inputs_reciprocal_magnetic;
	std::array<v1::PropertyAngle, MaxInputs>	_inputs_initial_bearing_magnetic;
	std::array<v1::PropertyLength, MaxInputs>	_inputs_distance;
	std::array<v1::PropertyTime, MaxInputs>		_inputs_eta;
	std::array<v1::PropertyAngle, MaxInputs>	_inputs_deviation;
	std::array<v1::PropertyBoolean, MaxInputs>	_inputs_to_flag;
	// Output:
	v1::PropertyInteger							_output_type;
	v1::PropertyString							_output_reference;
	v1::PropertyString							_output_identifier;
	v1::PropertyAngle							_output_radial_magnetic;
	v1::PropertyAngle							_output_reciprocal_magnetic;
	v1::PropertyAngle							_output_initial_bearing_magnetic;
	v1::PropertyLength							_output_distance;
	v1::PropertyTime							_output_eta;
	v1::PropertyAngle							_output_deviation;
	v1::PropertyBoolean							_output_to_flag;
};

#endif
