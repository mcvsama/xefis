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
	Xefis::PropertySpeed	_ias;
	Xefis::PropertySpeed	_ias_lookahead;
	Xefis::PropertySpeed	_minimum_ias;
	Xefis::PropertySpeed	_maximum_ias;
	Xefis::PropertySpeed	_gs;
	Xefis::PropertySpeed	_tas;
	Xefis::PropertyFloat	_mach;
	Xefis::PropertyAngle	_pitch;
	Xefis::PropertyAngle	_roll;
	Xefis::PropertyAngle	_magnetic_heading;
	Xefis::PropertyAngle	_true_heading;
	Xefis::PropertyFloat	_slip_skid_g;
	Xefis::PropertyAngle	_fpm_alpha;
	Xefis::PropertyAngle	_fpm_beta;
	Xefis::PropertyAngle	_magnetic_track;
	Xefis::PropertyBoolean	_standard_pressure;
	Xefis::PropertyLength	_altitude;
	Xefis::PropertyLength	_altitude_agl;
	Xefis::PropertySpeed	_cbr;
	Xefis::PropertyPressure	_pressure;
	Xefis::PropertyLength	_cmd_alt_setting;
	Xefis::PropertySpeed	_cmd_speed_setting;
	Xefis::PropertyAngle	_cmd_heading_setting;
	Xefis::PropertySpeed	_cmd_cbr_setting;
	Xefis::PropertyAngle	_flight_director_pitch;
	Xefis::PropertyAngle	_flight_director_roll;
	Xefis::PropertyBoolean	_navigation_needles_visible;
	Xefis::PropertyAngle	_lateral_deviation;
	Xefis::PropertyAngle	_vertical_deviation;
	Xefis::PropertyLength	_dme_distance;
	Xefis::PropertyFloat	_outside_air_temperature_k;
	Xefis::PropertyFloat	_engine_throttle_pct;
	Xefis::PropertyFloat	_engine_epr;
	Xefis::PropertyFloat	_engine_n1_pct;
	Xefis::PropertyFloat	_engine_n2_pct;
	Xefis::PropertyFloat	_engine_egt_degc;
	Xefis::PropertyAngle	_position_lat;
	Xefis::PropertyAngle	_position_lng;
	Xefis::PropertyLength	_position_amsl;
	Xefis::PropertyAngle	_wind_from_magnetic_heading;
	Xefis::PropertySpeed	_wind_tas;
	// Output:
	Xefis::PropertyFloat	_ailerons;
	Xefis::PropertyFloat	_elevator;
	Xefis::PropertyFloat	_rudder;
};

#endif
