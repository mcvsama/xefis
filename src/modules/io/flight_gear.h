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

#ifndef XEFIS__MODULES__IO__FLIGHT_GEAR_H__INCLUDED
#define XEFIS__MODULES__IO__FLIGHT_GEAR_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>

// Qt:
#include <QtCore/QTimer>
#include <QtCore/QObject>
#include <QtNetwork/QUdpSocket>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <xefis/core/module.h>


class FlightGearIO:
	public QObject,
	public Xefis::Module
{
	Q_OBJECT

  public:
	// Ctor
	FlightGearIO (Xefis::ModuleManager*, QDomElement const& config);

  private slots:
	/**
	 * Called whenever there's data ready to be read from socket.
	 */
	void
	got_packet();

	/**
	 * Set all input properties as invalid.
	 */
	void
	invalidate_all();

  private:
	/**
	 * Read and apply FlightGear datagrams in binary mode from UDP socket.
	 */
	void
	read_input();

	/**
	 * Write data to configured UDP port.
	 */
	void
	write_output();

  private:
	Unique<QTimer>							_timeout_timer;
	QString									_input_host;
	int										_input_port;
	Unique<QUdpSocket>						_input;
	QByteArray								_input_datagram;
	QString									_output_host;
	int										_output_port;
	Unique<QUdpSocket>						_output;
	std::string								_property_path;
	bool									_input_enabled				= false;
	bool									_output_enabled				= false;
	std::vector<Xefis::GenericProperty*>	_output_properties;
	std::vector<Xefis::PropertyBoolean*>	_serviceable_flags;
	// Input:
	Xefis::PropertyAngle					_aoa_alpha_maximum;
	Xefis::PropertyAngle					_aoa_alpha_minimum;
	Xefis::PropertyAngle					_aoa_alpha;
	Xefis::PropertySpeed					_ias;
	Xefis::PropertySpeed					_ias_lookahead;
	Xefis::PropertySpeed					_minimum_ias;
	Xefis::PropertySpeed					_maximum_ias;
	Xefis::PropertyBoolean					_ias_serviceable;
	Xefis::PropertySpeed					_gs;
	Xefis::PropertySpeed					_tas;
	Xefis::PropertyFloat					_mach;
	Xefis::PropertyAngle					_ahrs_pitch;
	Xefis::PropertyAngle					_ahrs_roll;
	Xefis::PropertyAngle					_ahrs_magnetic_heading;
	Xefis::PropertyAngle					_ahrs_true_heading;
	Xefis::PropertyBoolean					_ahrs_serviceable;
	Xefis::PropertyFloat					_slip_skid_g;
	Xefis::PropertyAngle					_fpm_alpha;
	Xefis::PropertyAngle					_fpm_beta;
	Xefis::PropertyAngle					_magnetic_track;
	Xefis::PropertyBoolean					_standard_pressure;
	Xefis::PropertyLength					_altitude;
	Xefis::PropertyLength					_radar_altimeter_altitude_agl;
	Xefis::PropertyBoolean					_radar_altimeter_serviceable;
	Xefis::PropertySpeed					_cbr;
	Xefis::PropertyPressure					_pressure;
	Xefis::PropertyBoolean					_pressure_serviceable;
	Xefis::PropertyLength					_cmd_alt_setting;
	Xefis::PropertySpeed					_cmd_speed_setting;
	Xefis::PropertyAngle					_cmd_heading_setting;
	Xefis::PropertySpeed					_cmd_cbr_setting;
	Xefis::PropertyAngle					_flight_director_pitch;
	Xefis::PropertyAngle					_flight_director_roll;
	Xefis::PropertyBoolean					_navigation_needles_visible;
	Xefis::PropertyAngle					_lateral_deviation;
	Xefis::PropertyAngle					_vertical_deviation;
	Xefis::PropertyLength					_dme_distance;
	Xefis::PropertyTemperature				_static_air_temperature;
	Xefis::PropertyFloat					_engine_throttle_pct;
	Xefis::PropertyFloat					_engine_1_thrust;
	Xefis::PropertyFloat					_engine_1_rpm;
	Xefis::PropertyAngle					_engine_1_pitch;
	Xefis::PropertyFloat					_engine_1_epr;
	Xefis::PropertyFloat					_engine_1_n1_pct;
	Xefis::PropertyFloat					_engine_1_n2_pct;
	Xefis::PropertyTemperature				_engine_1_egt;
	Xefis::PropertyFloat					_engine_2_thrust;
	Xefis::PropertyFloat					_engine_2_rpm;
	Xefis::PropertyAngle					_engine_2_pitch;
	Xefis::PropertyFloat					_engine_2_epr;
	Xefis::PropertyFloat					_engine_2_n1_pct;
	Xefis::PropertyFloat					_engine_2_n2_pct;
	Xefis::PropertyTemperature				_engine_2_egt;
	Xefis::PropertyAngle					_gps_latitude;
	Xefis::PropertyAngle					_gps_longitude;
	Xefis::PropertyLength					_gps_amsl;
	Xefis::PropertyLength					_gps_accuracy_lateral;
	Xefis::PropertyLength					_gps_accuracy_vertical;
	Xefis::PropertyBoolean					_gps_serviceable;
	Xefis::PropertyString					_gps_source;
	Xefis::PropertyAngle					_wind_from_magnetic_heading;
	Xefis::PropertySpeed					_wind_tas;
	Xefis::PropertyBoolean					_gear_setting_down;
	Xefis::PropertyBoolean					_gear_nose_up;
	Xefis::PropertyBoolean					_gear_nose_down;
	Xefis::PropertyBoolean					_gear_left_up;
	Xefis::PropertyBoolean					_gear_left_down;
	Xefis::PropertyBoolean					_gear_right_up;
	Xefis::PropertyBoolean					_gear_right_down;
	// Output:
	Xefis::PropertyFloat					_ailerons;
	Xefis::PropertyFloat					_elevator;
	Xefis::PropertyFloat					_rudder;
	Xefis::PropertyFloat					_throttle_1;
	Xefis::PropertyFloat					_throttle_2;
	Xefis::PropertyFloat					_flaps;
};

#endif
