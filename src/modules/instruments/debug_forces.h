/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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
#include <xefis/core/instrument.h>
#include <xefis/core/instrument_aids.h>
#include <xefis/core/property.h>


class DebugForces:
	public Xefis::Instrument,
	protected Xefis::InstrumentAids
{
  public:
	// Ctor
	DebugForces (Xefis::ModuleManager*, QDomElement const& config);

	// Module
	void
	data_updated() override;

  protected:
	// QWidget
	void
	paintEvent (QPaintEvent*) override;

  private:
	Xefis::PropertyAngle		_input_orientation_pitch;
	Xefis::PropertyAngle		_input_orientation_roll;
	Xefis::PropertyAngle		_input_orientation_magnetic_heading;
	Xefis::PropertyAcceleration	_input_measured_accel_x;
	Xefis::PropertyAcceleration	_input_measured_accel_y;
	Xefis::PropertyAcceleration	_input_measured_accel_z;
	Xefis::PropertyAcceleration	_input_centrifugal_accel_x;
	Xefis::PropertyAcceleration	_input_centrifugal_accel_y;
	Xefis::PropertyAcceleration	_input_centrifugal_accel_z;
};

#endif
