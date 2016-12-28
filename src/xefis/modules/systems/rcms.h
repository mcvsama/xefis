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
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/property_observer.h>


class RemoteControlManagementSystem:
	public QObject,
	public x2::Module,
	public x2::Module::HasConfiguratorWidget
{
	Q_OBJECT

  public:
	/*
	 * Input
	 */

	x2::PropertyIn<si::Length>		input_vlos_caution_distance		{ this, "/vlos-caution-distance" };
	x2::PropertyIn<si::Length>		input_vlos_warning_distance		{ this, "/vlos-warning-distance" };
	x2::PropertyIn<si::Angle>		input_home_longitude			{ this, "/home/longitude" };
	x2::PropertyIn<si::Angle>		input_home_latitude				{ this, "/home/latitude" };
	x2::PropertyIn<si::Length>		input_home_altitude_amsl		{ this, "/home/altitude-amsl" };
	x2::PropertyIn<si::Angle>		input_position_longitude		{ this, "/position/longitude" };
	x2::PropertyIn<si::Angle>		input_position_latitude			{ this, "/position/latitude" };
	x2::PropertyIn<si::Length>		input_position_altitude_amsl	{ this, "/position/altitude.amsl" };

	/*
	 * Output
	 */

	x2::PropertyOut<si::Length>		output_distance_vlos			{ this, "/distance/vlos" };
	x2::PropertyOut<si::Length>		output_distance_ground			{ this, "/distance/ground" };
	x2::PropertyOut<si::Length>		output_distance_vertical		{ this, "/distance/vertical" };
	x2::PropertyOut<si::Angle>		output_true_home_direction		{ this, "/home-direction.true" };

  public:
	// Ctor
	explicit RemoteControlManagementSystem (std::string const& instance = {});

	// Module::HasConfiguratorWidget API
	QWidget*
	configurator_widget() override;

	// Module API
	void
	process (x2::Cycle const&) override;

  private slots:
	/**
	 * Set current aircraft position as "HOME" position.
	 */
	void
	acquire_home();

  private:
	void
	prepare_configurator_widget();

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
	x2::PropertyObserver	_distance_computer;
};

#endif
