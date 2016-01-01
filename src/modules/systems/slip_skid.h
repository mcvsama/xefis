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

#ifndef XEFIS__MODULES__SYSTEMS__SLIP_SKID_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__SLIP_SKID_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>


/**
 * Computes slip-skid value from three-axis accelerometer values.
 */
class SlipSkid: public xf::Module
{
  public:
	// Ctor
	SlipSkid (xf::ModuleManager*, QDomElement const& config);

  private:
	void
	data_updated() override;

	void
	compute_slip_skid();

  private:
	// Input:
	xf::PropertyAcceleration	_y_acceleration;
	xf::PropertyAcceleration	_z_acceleration;
	// Output:
	xf::PropertyFloat			_slip_skid;
	// Other:
	xf::PropertyObserver		_slip_skid_computer;
};

#endif
