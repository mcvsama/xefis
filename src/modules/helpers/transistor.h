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

#ifndef XEFIS__MODULES__HELPERS__TRANSISTOR_H__INCLUDED
#define XEFIS__MODULES__HELPERS__TRANSISTOR_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/v1/property.h>
#include <xefis/core/v1/property_observer.h>
#include <xefis/utility/transistor.h>


/**
 * Linearly transition between two values.
 */
class Transistor: public xf::Module
{
  public:
	// Ctor
	Transistor (xf::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

	void
	input_changed();

  private:
	Time					_transition_time	= 1_s;
	// I/O:
	xf::PropertyFloat		_input_0_value;
	xf::PropertyFloat		_input_1_value;
	xf::PropertyInteger		_input_selected;
	xf::PropertyFloat		_output_value;
	// Other:
	xf::PropertyFloat::Type	_last_0_value		= xf::PropertyFloat::Type();
	xf::PropertyFloat::Type	_last_1_value		= xf::PropertyFloat::Type();
	xf::PropertyObserver	_observer;
	Unique<xf::Transistor<xf::PropertyFloat::Type>>
							_transistor;
};

#endif
