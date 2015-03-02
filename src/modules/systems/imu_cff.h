/* vim:ts=4
 *
 * Copyleft 2012…2015  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__SYSTEMS__IMU_CFF_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__IMU_CFF_H__INCLUDED

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
 * Compute centrifugal force from IAS and gyro information.
 */
class IMUCFF: public xf::Module
{
  public:
	// Ctor
	IMUCFF (xf::ModuleManager*, QDomElement const& config);

  private:
	void
	data_updated() override;

	void
	compute_centrifugal();

  private:
	xf::Smoother<double>		_smooth_cf_y;
	xf::Smoother<double>		_smooth_cf_z;
	// Input:
	xf::PropertyFrequency		_rotation_x;
	xf::PropertyFrequency		_rotation_y;
	xf::PropertyFrequency		_rotation_z;
	xf::PropertySpeed			_ias_x;
	xf::PropertySpeed			_ias_y;
	xf::PropertySpeed			_ias_z;
	// Output:
	xf::PropertyAcceleration	_centrifugal_x;
	xf::PropertyAcceleration	_centrifugal_y;
	xf::PropertyAcceleration	_centrifugal_z;
	// Other:
	xf::PropertyObserver		_centrifugal_computer;
};

#endif
