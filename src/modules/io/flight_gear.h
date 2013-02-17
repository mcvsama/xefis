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
#include <xefis/core/input.h>


class FlightGearIO:
	public QObject,
	public Xefis::Input
{
	Q_OBJECT

  public:
	// Ctor
	FlightGearIO (Xefis::ModuleManager*, QDomElement const& config);

	// Dtor
	~FlightGearIO();

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
	QTimer*					_timeout_timer				= nullptr;
	QString					_input_host;
	int						_input_port;
	QUdpSocket*				_input						= nullptr;
	QByteArray				_input_datagram;
	QString					_output_host;
	int						_output_port;
	QUdpSocket*				_output						= nullptr;
	std::string				_property_path;
	bool					_input_enabled				= false;
	bool					_output_enabled				= false;
	// Input:
	Xefis::PropertyFloat	_ias_kt;
	Xefis::PropertyFloat	_ias_lookahead_kt;
	Xefis::PropertyFloat	_minimum_ias_kt;
	Xefis::PropertyFloat	_maximum_ias_kt;
	Xefis::PropertyFloat	_gs_kt;
	Xefis::PropertyFloat	_tas_kt;
	Xefis::PropertyFloat	_mach;
	Xefis::PropertyFloat	_pitch_deg;
	Xefis::PropertyFloat	_roll_deg;
	Xefis::PropertyFloat	_mag_heading_deg;
	Xefis::PropertyFloat	_true_heading_deg;
	Xefis::PropertyFloat	_slip_skid_g;
	Xefis::PropertyFloat	_fpm_alpha_deg;
	Xefis::PropertyFloat	_fpm_beta_deg;
	Xefis::PropertyFloat	_magnetic_track_deg;
	Xefis::PropertyFloat	_standard_pressure;
	Xefis::PropertyFloat	_altitude_ft;
	Xefis::PropertyFloat	_altitude_agl_ft;
	Xefis::PropertyFloat	_cbr_fpm;
	Xefis::PropertyFloat	_pressure_inhg;
	Xefis::PropertyFloat	_autopilot_alt_setting_ft;
	Xefis::PropertyFloat	_autopilot_speed_setting_kt;
	Xefis::PropertyFloat	_autopilot_heading_setting_deg;
	Xefis::PropertyFloat	_autopilot_cbr_setting_fpm;
	Xefis::PropertyFloat	_flight_director_pitch_deg;
	Xefis::PropertyFloat	_flight_director_roll_deg;
	Xefis::PropertyBoolean	_navigation_needles_visible;
	Xefis::PropertyFloat	_lateral_deviation_deg;
	Xefis::PropertyFloat	_vertical_deviation_deg;
	Xefis::PropertyFloat	_dme_distance_nm;
	Xefis::PropertyFloat	_engine_throttle_pct;
	Xefis::PropertyFloat	_engine_epr;
	Xefis::PropertyFloat	_engine_n1_pct;
	Xefis::PropertyFloat	_engine_n2_pct;
	Xefis::PropertyFloat	_engine_egt_degc;
	Xefis::PropertyFloat	_position_lat_deg;
	Xefis::PropertyFloat	_position_lng_deg;
	Xefis::PropertyFloat	_position_sea_level_radius_ft;
	Xefis::PropertyFloat	_wind_from_mag_heading_deg;
	Xefis::PropertyFloat	_wind_tas_kt;
	// Output:
	Xefis::PropertyFloat	_ailerons;
	Xefis::PropertyFloat	_ailerons_trim;
	Xefis::PropertyFloat	_elevator;
	Xefis::PropertyFloat	_elevator_trim;
	Xefis::PropertyFloat	_rudder;
	Xefis::PropertyFloat	_rudder_trim;
};

#endif
