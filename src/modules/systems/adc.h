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

#ifndef XEFIS__MODULES__SYSTEMS__ADC_H__INCLUDED
#define XEFIS__MODULES__SYSTEMS__ADC_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/utility/datatable2d.h>
#include <xefis/utility/smoother.h>
#include <xefis/utility/lookahead.h>


/**
 * Computations are reliable up to 36,000 ft altitude
 * and about 0.3 mach speed.
 */
class AirDataComputer: public xf::Module
{
  public:
	// Ctor
	AirDataComputer (xf::ModuleManager*, QDomElement const& config);

  private:
	void
	data_updated() override;

	void
	compute_altitude();

	void
	compute_ias();

	void
	compute_ias_lookahead();

	void
	compute_mach();

	void
	compute_sat_and_viscosity();

	void
	compute_density_altitude();

	void
	compute_sound_speed();

	void
	compute_tas();

	void
	compute_vertical_speed();

	void
	compute_reynolds();

  private:
	bool					_ias_in_valid_range					= false;
	bool					_prev_use_standard_pressure			= false;
	Time					_hide_alt_lookahead_until			= 0_s;
	Length					_prev_altitude_amsl					= 0_ft;
	Unique<xf::Datatable2D<Temperature, double>>
							_temperature_to_dynamic_viscosity;
	// Note: PropertyObservers depend on Smoothers, so first Smoothers must be defined,
	// then PropertyObservers, to ensure correct order of destruction.
	xf::Smoother<double>	_vertical_speed_smoother			= 1_s;
	xf::Smoother<double>	_altitude_amsl_smoother				= 500_ms;
	xf::Smoother<double>	_altitude_amsl_qnh_smoother			= 500_ms;
	xf::Smoother<double>	_altitude_amsl_std_smoother			= 500_ms;
	xf::Smoother<double>	_speed_ias_smoother					= 100_ms;
	xf::Smoother<double>	_altitude_amsl_lookahead_i_smoother	= 100_ms;
	xf::Smoother<double>	_altitude_amsl_lookahead_o_smoother	= 500_ms;
	xf::Smoother<double>	_speed_ias_lookahead_i_smoother		= 100_ms;
	xf::Smoother<double>	_speed_ias_lookahead_o_smoother		= 1000_ms;
	xf::Lookahead<double>	_altitude_amsl_estimator			= xf::Lookahead<double> (10_s);
	xf::Lookahead<double>	_speed_ias_estimator				= xf::Lookahead<double> (10_s);
	// Settings:
	Speed					_ias_valid_minimum;
	Speed					_ias_valid_maximum;
	bool					_using_ias_sensor					= false;
	// Input:
	xf::PropertyBoolean		_pressure_use_std;
	xf::PropertyPressure	_pressure_qnh;
	xf::PropertyBoolean		_pressure_static_serviceable;
	xf::PropertyPressure	_pressure_static;
	xf::PropertyPressure	_pressure_total;
	xf::PropertyBoolean		_ias_serviceable;
	xf::PropertySpeed		_ias;
	xf::PropertyTemperature	_total_air_temperature;
	// Output:
	xf::PropertyPressure	_pressure_dynamic;
	xf::PropertyBoolean		_altitude_amsl_serviceable;
	xf::PropertyLength		_altitude_amsl;
	xf::PropertyLength		_altitude_amsl_lookahead;
	xf::PropertyLength		_altitude_amsl_qnh;
	xf::PropertyLength		_altitude_amsl_std;
	xf::PropertyLength		_density_altitude;
	xf::PropertyDensity		_air_density_static;
	xf::PropertyBoolean		_speed_ias_serviceable;
	xf::PropertySpeed		_speed_ias;
	xf::PropertySpeed		_speed_ias_lookahead;
	xf::PropertySpeed		_speed_tas;
	xf::PropertyFloat		_speed_mach;
	xf::PropertySpeed		_speed_sound;
	xf::PropertyBoolean		_vertical_speed_serviceable;
	xf::PropertySpeed		_vertical_speed;
	xf::PropertyTemperature	_static_air_temperature;
	xf::PropertyFloat		_dynamic_viscosity;
	xf::PropertyFloat		_reynolds_number;
	// Other:
	xf::PropertyObserver	_altitude_computer;
	xf::PropertyObserver	_density_altitude_computer;
	xf::PropertyObserver	_ias_computer;
	xf::PropertyObserver	_ias_lookahead_computer;
	xf::PropertyObserver	_sound_speed_computer;
	xf::PropertyObserver	_tas_computer;
	xf::PropertyObserver	_mach_computer;
	xf::PropertyObserver	_sat_computer;
	xf::PropertyObserver	_vertical_speed_computer;
	xf::PropertyObserver	_reynolds_computer;
};

#endif
