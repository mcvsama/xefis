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

#ifndef XEFIS__MODULES__HELPERS__NAVAID_SELECTOR_H__INCLUDED
#define XEFIS__MODULES__HELPERS__NAVAID_SELECTOR_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>


class NavaidSelector: public Xefis::Module
{
	static constexpr int MaxInputs = 8;

  public:
	// Ctor
	NavaidSelector (Xefis::ModuleManager*, QDomElement const& config);

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
	Xefis::PropertyInteger							_selected_input;
	std::array<Xefis::PropertyString, MaxInputs>	_inputs_reference;
	std::array<Xefis::PropertyString, MaxInputs>	_inputs_identifier;
	std::array<Xefis::PropertyAngle, MaxInputs>		_inputs_radial_magnetic;
	std::array<Xefis::PropertyAngle, MaxInputs>		_inputs_reciprocal_magnetic;
	std::array<Xefis::PropertyLength, MaxInputs>	_inputs_distance;
	std::array<Xefis::PropertyTime, MaxInputs>		_inputs_eta;
	std::array<Xefis::PropertyAngle, MaxInputs>		_inputs_deviation;
	std::array<Xefis::PropertyBoolean, MaxInputs>	_inputs_to_flag;
	// Output:
	Xefis::PropertyString							_output_reference;
	Xefis::PropertyString							_output_identifier;
	Xefis::PropertyAngle							_output_radial_magnetic;
	Xefis::PropertyAngle							_output_reciprocal_magnetic;
	Xefis::PropertyLength							_output_distance;
	Xefis::PropertyTime								_output_eta;
	Xefis::PropertyAngle							_output_deviation;
	Xefis::PropertyBoolean							_output_to_flag;
};

#endif
