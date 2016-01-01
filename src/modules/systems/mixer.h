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

#ifndef XEFIS__MODULES__SYSTEMS__MIXER_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__MIXER_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>


class Mixer: public xf::Module
{
  public:
	// Ctor
	Mixer (xf::ModuleManager*, QDomElement const& config);

  private:
	// Module API
	void
	data_updated() override;

  private:
	// Settings:
	double				_input_0_factor	= 1.0;
	double				_input_1_factor	= 1.0;
	Optional<double>	_output_minimum;
	Optional<double>	_output_maximum;
	// Input:
	xf::PropertyFloat	_input_0_value;
	xf::PropertyFloat	_input_1_value;
	// Output:
	xf::PropertyFloat	_output_value;
};

#endif
