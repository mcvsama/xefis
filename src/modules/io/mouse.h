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

#ifndef XEFIS__MODULES__IO__MOUSE_H__INCLUDED
#define XEFIS__MODULES__IO__MOUSE_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>


class Mouse:
	public QObject,
	public xf::Module
{
	Q_OBJECT

  public:
	// Ctor
	Mouse (xf::ModuleManager*, QDomElement const& config);

  private slots:
	void
	check();

  private:
	float
	remove_dead_zone (float input, float dead_deflection);

  private:
	float				_dead_zone_x	= 0.2;
	float				_dead_zone_y	= 0.2;
	float				_speed_x		= 1.0f;
	float				_speed_y		= 1.0f;
	float				_acceleration_x	= 2.0f;
	float				_acceleration_y	= 2.0f;
	bool				_clicked		= false;
	// Input:
	xf::PropertyFloat	_axis_x;
	xf::PropertyFloat	_axis_y;
	xf::PropertyBoolean	_button;
};

#endif
