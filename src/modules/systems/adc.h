/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__SYSTEMS__ADC_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__ADC_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/utility/smoother.h>
#include <xefis/utility/lookahead.h>


/**
 * Computations are reliable up to 36,000 ft altitude
 * and about 0.3 mach speed.
 */
class AirDataComputer: public Xefis::Module
{
  public:
	// Ctor
	AirDataComputer (Xefis::ModuleManager*, QDomElement const& config);

  private:
	void
	data_updated() override;

	void
	compute_altitude();

	void
	compute_density_altitude();

	void
	compute_ias();

	void
	compute_ias_lookahead();

	void
	compute_sound_speed();

	void
	compute_tas();

	void
	compute_mach();

	void
	compute_vertical_speed();

  private:
	bool						_ias_in_valid_range					= false;
	bool						_prev_use_standard_pressure			= false;
	Time						_hide_alt_lookahead_until			= 0_s;
	Length						_prev_altitude_amsl					= 0_ft;
	// Note: PropertyObservers depend on Smoothers, so first Smoothers must be defined,
	// then PropertyObservers, to ensure correct order of destruction.
	Xefis::Smoother<double>		_vertical_speed_smoother			= 1_s;
	Xefis::Smoother<double>		_altitude_amsl_smoother				= 500_ms;
	Xefis::Smoother<double>		_altitude_amsl_qnh_smoother			= 500_ms;
	Xefis::Smoother<double>		_altitude_amsl_std_smoother			= 500_ms;
	Xefis::Smoother<double>		_speed_ias_smoother					= 100_ms;
	Xefis::Smoother<double>		_altitude_amsl_lookahead_i_smoother	= 100_ms;
	Xefis::Smoother<double>		_altitude_amsl_lookahead_o_smoother	= 500_ms;
	Xefis::Smoother<double>		_speed_ias_lookahead_i_smoother		= 100_ms;
	Xefis::Smoother<double>		_speed_ias_lookahead_o_smoother		= 1000_ms;
	Xefis::Lookahead<double>	_altitude_amsl_estimator			= Xefis::Lookahead<double> (10_s);
	Xefis::Lookahead<double>	_speed_ias_estimator				= Xefis::Lookahead<double> (10_s);
	Xefis::PropertyObserver		_altitude_computer;
	Xefis::PropertyObserver		_density_altitude_computer;
	Xefis::PropertyObserver		_ias_computer;
	Xefis::PropertyObserver		_ias_lookahead_computer;
	Xefis::PropertyObserver		_sound_speed_computer;
	Xefis::PropertyObserver		_tas_computer;
	Xefis::PropertyObserver		_mach_computer;
	Xefis::PropertyObserver		_vertical_speed_computer;
	// Settings:
	Speed						_ias_valid_minimum;
	Speed						_ias_valid_maximum;
	// Input:
	Xefis::PropertyBoolean		_pressure_use_std;
	Xefis::PropertyPressure		_pressure_qnh;
	Xefis::PropertyBoolean		_pressure_static_serviceable;
	Xefis::PropertyPressure		_pressure_static;
	Xefis::PropertyBoolean		_ias_serviceable;
	Xefis::PropertySpeed		_ias;
	Xefis::PropertyTemperature	_static_air_temperature;
	// Output:
	Xefis::PropertyBoolean		_altitude_amsl_serviceable;
	Xefis::PropertyLength		_altitude_amsl;
	Xefis::PropertyLength		_altitude_amsl_lookahead;
	Xefis::PropertyLength		_altitude_amsl_qnh;
	Xefis::PropertyLength		_altitude_amsl_std;
	Xefis::PropertyLength		_density_altitude;
	Xefis::PropertyBoolean		_speed_ias_serviceable;
	Xefis::PropertySpeed		_speed_ias;
	Xefis::PropertySpeed		_speed_ias_lookahead;
	Xefis::PropertySpeed		_speed_tas;
	Xefis::PropertyFloat		_speed_mach;
	Xefis::PropertySpeed		_speed_sound;
	Xefis::PropertyBoolean		_vertical_speed_serviceable;
	Xefis::PropertySpeed		_vertical_speed;
};

#endif
