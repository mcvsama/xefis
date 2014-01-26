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

#ifndef XEFIS__MODULES__SYSTEMS__RCMS_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__RCMS_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>


class RemoteControlManagementSystem:
	public QObject,
	public Xefis::Module
{
	Q_OBJECT

  public:
	// Ctor
	RemoteControlManagementSystem (Xefis::ModuleManager*, QDomElement const& config);

	// Xefis::Module API.
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
	QWidget*				_configurator_widget	= nullptr;
	bool					_home_acquired			= false;
	Xefis::PropertyObserver	_distance_computer;
	// Input:
	Xefis::PropertyLength	_vlos_caution_distance;
	Xefis::PropertyLength	_vlos_warning_distance;
	Xefis::PropertyAngle	_home_longitude;
	Xefis::PropertyAngle	_home_latitude;
	Xefis::PropertyLength	_home_altitude_amsl;
	Xefis::PropertyAngle	_position_longitude;
	Xefis::PropertyAngle	_position_latitude;
	Xefis::PropertyLength	_position_altitude_amsl;
	// Output:
	Xefis::PropertyLength	_distance_vlos;
	Xefis::PropertyLength	_distance_ground;
	Xefis::PropertyLength	_distance_vertical;
	Xefis::PropertyAngle	_true_home_direction;
};

#endif
