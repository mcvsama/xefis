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

#ifndef XEFIS__MODULES__SYSTEMS__RCMS_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__RCMS_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>


class RemoteControlManagementSystem:
	public QObject,
	public xf::Module
{
	Q_OBJECT

  public:
	// Ctor
	RemoteControlManagementSystem (xf::ModuleManager*, QDomElement const& config);

	// xf::Module API.
	QWidget*
	configurator_widget() const override;

  private slots:
	void
	acquire_home();

  private:
	void
	create_configurator_widget();

	void
	data_updated() override;

	bool
	home_is_valid() const;

	bool
	position_is_valid() const;

	void
	compute_distances_to_home();

	void
	compute_true_home_direction();

  private:
	Unique<QWidget>			_configurator_widget;
	bool					_home_acquired			= false;
	// Input:
	xf::PropertyLength		_vlos_caution_distance;
	xf::PropertyLength		_vlos_warning_distance;
	xf::PropertyAngle		_home_longitude;
	xf::PropertyAngle		_home_latitude;
	xf::PropertyLength		_home_altitude_amsl;
	xf::PropertyAngle		_position_longitude;
	xf::PropertyAngle		_position_latitude;
	xf::PropertyLength		_position_altitude_amsl;
	// Output:
	xf::PropertyLength		_distance_vlos;
	xf::PropertyLength		_distance_ground;
	xf::PropertyLength		_distance_vertical;
	xf::PropertyAngle		_true_home_direction;
	// Other:
	xf::PropertyObserver	_distance_computer;
};

#endif
