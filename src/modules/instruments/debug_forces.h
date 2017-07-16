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

#ifndef XEFIS__MODULES__INSTRUMENTS__DEBUG_FORCES_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__DEBUG_FORCES_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Qt:
#include <QtWidgets/QWidget>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v1/instrument.h>
#include <xefis/core/instrument_aids.h>
#include <xefis/core/v1/property.h>


class DebugForces:
	public v1::Instrument,
	protected xf::InstrumentAids
{
  public:
	// Ctor
	DebugForces (v1::ModuleManager*, QDomElement const& config);

	// Module
	void
	data_updated() override;

  protected:
	// QWidget
	void
	paintEvent (QPaintEvent*) override;

  private:
	v1::PropertyAngle			_input_orientation_pitch;
	v1::PropertyAngle			_input_orientation_roll;
	v1::PropertyAngle			_input_orientation_magnetic_heading;
	v1::PropertyAcceleration	_input_measured_accel_x;
	v1::PropertyAcceleration	_input_measured_accel_y;
	v1::PropertyAcceleration	_input_measured_accel_z;
	v1::PropertyAcceleration	_input_centrifugal_accel_x;
	v1::PropertyAcceleration	_input_centrifugal_accel_y;
	v1::PropertyAcceleration	_input_centrifugal_accel_z;
};

#endif
