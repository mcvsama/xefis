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

#ifndef XEFIS__MODULES__HELPERS__LONLAT_TRANSLATOR_H__INCLUDED
#define XEFIS__MODULES__HELPERS__LONLAT_TRANSLATOR_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/module.h>
#include <xefis/core/v1/property.h>


/**
 * Translate Property<Angle> to PropertyString in DMS format.
 */
class LonLatTranslator: public v1::Module
{
  public:
	// Ctor
	LonLatTranslator (v1::ModuleManager*, QDomElement const& config);

  protected:
	void
	data_updated() override;

  private:
	v1::PropertyAngle	_input_longitude;
	v1::PropertyAngle	_input_latitude;
	v1::PropertyString	_output_longitude;
	v1::PropertyString	_output_latitude;
};

#endif
