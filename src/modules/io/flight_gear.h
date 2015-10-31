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
	public xf::Module
{
	Q_OBJECT

  public:
	// Ctor
	FlightGearIO (xf::ModuleManager*, QDomElement const& config);

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
	Unique<QTimer>						_timeout_timer;
	QString								_input_host;
	int									_input_port;
	Unique<QUdpSocket>					_input;
	QByteArray							_input_datagram;
	QString								_output_host;
	int									_output_port;
	Unique<QUdpSocket>					_output;
	std::string							_property_path;
	bool								_input_enabled				= false;
	bool								_output_enabled				= false;
	std::vector<xf::GenericProperty*>	_output_properties;
	std::vector<xf::PropertyBoolean*>	_serviceable_flags;
	// Input:
	xf::PropertyFrequency				_rotation_x;
	xf::PropertyFrequency				_rotation_y;
	xf::PropertyFrequency				_rotation_z;
	xf::PropertyAcceleration			_acceleration_x;
	xf::PropertyAcceleration			_acceleration_y;
	xf::PropertyAcceleration			_acceleration_z;
	xf::PropertyAngle					_aoa_alpha_maximum;
	xf::PropertyAngle					_aoa_alpha_minimum;
	xf::PropertyAngle					_aoa_alpha;
	xf::PropertySpeed					_ias;
	xf::PropertySpeed					_ias_lookahead;
	xf::PropertySpeed					_minimum_ias;
	xf::PropertySpeed					_maximum_ias;
	xf::PropertyBoolean					_ias_serviceable;
	xf::PropertySpeed					_gs;
	xf::PropertySpeed					_tas;
	xf::PropertyFloat					_mach;
	xf::PropertyAngle					_ahrs_pitch;
	xf::PropertyAngle					_ahrs_roll;
	xf::PropertyAngle					_ahrs_magnetic_heading;
	xf::PropertyAngle					_ahrs_true_heading;
	xf::PropertyBoolean					_ahrs_serviceable;
	xf::PropertyFloat					_slip_skid_g;
	xf::PropertyAngle					_fpm_alpha;
	xf::PropertyAngle					_fpm_beta;
	xf::PropertyAngle					_magnetic_track;
	xf::PropertyBoolean					_standard_pressure;
	xf::PropertyLength					_altitude;
	xf::PropertyLength					_radar_altimeter_altitude_agl;
	xf::PropertyBoolean					_radar_altimeter_serviceable;
	xf::PropertySpeed					_cbr;
	xf::PropertyPressure				_pressure;
	xf::PropertyBoolean					_pressure_serviceable;
	xf::PropertyLength					_cmd_alt_setting;
	xf::PropertySpeed					_cmd_speed_setting;
	xf::PropertyAngle					_cmd_heading_setting;
	xf::PropertySpeed					_cmd_cbr_setting;
	xf::PropertyAngle					_flight_director_pitch;
	xf::PropertyAngle					_flight_director_roll;
	xf::PropertyBoolean					_navigation_needles_visible;
	xf::PropertyAngle					_lateral_deviation;
	xf::PropertyAngle					_vertical_deviation;
	xf::PropertyLength					_dme_distance;
	xf::PropertyTemperature				_total_air_temperature;
	xf::PropertyFloat					_engine_throttle_pct;
	xf::PropertyForce					_engine_1_thrust;
	xf::PropertyFrequency				_engine_1_rpm;
	xf::PropertyAngle					_engine_1_pitch;
	xf::PropertyFloat					_engine_1_epr;
	xf::PropertyFloat					_engine_1_n1_pct;
	xf::PropertyFloat					_engine_1_n2_pct;
	xf::PropertyTemperature				_engine_1_egt;
	xf::PropertyForce					_engine_2_thrust;
	xf::PropertyFrequency				_engine_2_rpm;
	xf::PropertyAngle					_engine_2_pitch;
	xf::PropertyFloat					_engine_2_epr;
	xf::PropertyFloat					_engine_2_n1_pct;
	xf::PropertyFloat					_engine_2_n2_pct;
	xf::PropertyTemperature				_engine_2_egt;
	xf::PropertyAngle					_gps_latitude;
	xf::PropertyAngle					_gps_longitude;
	xf::PropertyLength					_gps_amsl;
	xf::PropertyLength					_gps_lateral_stddev;
	xf::PropertyLength					_gps_vertical_stddev;
	xf::PropertyBoolean					_gps_serviceable;
	xf::PropertyString					_gps_source;
	xf::PropertyAngle					_wind_from_magnetic_heading;
	xf::PropertySpeed					_wind_tas;
	xf::PropertyBoolean					_gear_setting_down;
	xf::PropertyBoolean					_gear_nose_up;
	xf::PropertyBoolean					_gear_nose_down;
	xf::PropertyBoolean					_gear_left_up;
	xf::PropertyBoolean					_gear_left_down;
	xf::PropertyBoolean					_gear_right_up;
	xf::PropertyBoolean					_gear_right_down;
	// Output:
	xf::PropertyFloat					_ailerons;
	xf::PropertyFloat					_elevator;
	xf::PropertyFloat					_rudder;
	xf::PropertyFloat					_throttle_1;
	xf::PropertyFloat					_throttle_2;
	xf::PropertyFloat					_flaps;
};

#endif
