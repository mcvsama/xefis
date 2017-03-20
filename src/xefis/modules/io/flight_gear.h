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

#ifndef XEFIS__MODULES__IO__FLIGHT_GEAR_H__INCLUDED
#define XEFIS__MODULES__IO__FLIGHT_GEAR_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>

// Qt:
#include <QtCore/QTimer>
#include <QtCore/QObject>
#include <QtNetwork/QUdpSocket>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>


class FlightGearIO:
	public QObject,
	public x2::Module
{
  public:
	/*
	 * Settings
	 */

	x2::Setting<bool>						setting_input_enabled				{ this, true };
	x2::Setting<std::string>				setting_input_host					{ this };
	x2::Setting<uint16_t>					setting_input_port					{ this };
	x2::Setting<bool>						setting_output_enabled				{ this, true };
	x2::Setting<std::string>				setting_output_host					{ this };
	x2::Setting<uint16_t>					setting_output_port					{ this };

	/*
	 * Input
	 */

	x2::PropertyIn<double>					input_ailerons						{ this, "/ailerons" }; // TODO
	x2::PropertyIn<double>					input_elevator						{ this, "/elevator" };
	x2::PropertyIn<double>					input_rudder						{ this, "/rudder" };
	x2::PropertyIn<double>					input_throttle_1					{ this, "/throttle-1" };
	x2::PropertyIn<double>					input_throttle_2					{ this, "/throttle-2" };
	x2::PropertyIn<double>					input_flaps							{ this, "/flaps" };

	/*
	 * Output
	 */

	x2::PropertyOut<si::AngularVelocity>	output_rotation_x					{ this, "/rotation/x" };
	x2::PropertyOut<si::AngularVelocity>	output_rotation_y					{ this, "/rotation/y" };
	x2::PropertyOut<si::AngularVelocity>	output_rotation_z					{ this, "/rotation/z" };
	x2::PropertyOut<si::Acceleration>		output_acceleration_x				{ this, "/acceleration/x" };
	x2::PropertyOut<si::Acceleration>		output_acceleration_y				{ this, "/acceleration/y" };
	x2::PropertyOut<si::Acceleration>		output_acceleration_z				{ this, "/acceleration/z" };
	x2::PropertyOut<si::Angle>				output_aoa_alpha_maximum			{ this, "/aoa/alpha.maximum" };
	x2::PropertyOut<si::Angle>				output_aoa_alpha_minimum			{ this, "/aoa/alpha.minimum" };
	x2::PropertyOut<si::Angle>				output_aoa_alpha					{ this, "/aoa/alpha" };
	x2::PropertyOut<si::Velocity>			output_ias							{ this, "/speeds/ias" };
	x2::PropertyOut<si::Velocity>			output_ias_lookahead				{ this, "/speeds/ias.lookahead" };
	x2::PropertyOut<si::Velocity>			output_minimum_ias					{ this, "/speeds/ias.minimum" };
	x2::PropertyOut<si::Velocity>			output_maximum_ias					{ this, "/speeds/ias.maximum" };
	x2::PropertyOut<bool>					output_ias_serviceable				{ this, "/speeds/ias.serviceable" };
	x2::PropertyOut<si::Velocity>			output_gs							{ this, "/speeds/gs" };
	x2::PropertyOut<si::Velocity>			output_tas							{ this, "/speeds/tas" };
	x2::PropertyOut<double>					output_mach							{ this, "/speeds/mach" };
	x2::PropertyOut<si::Angle>				output_ahrs_pitch					{ this, "/orientation/pitch" };
	x2::PropertyOut<si::Angle>				output_ahrs_roll					{ this, "/orientation/roll" };
	x2::PropertyOut<si::Angle>				output_ahrs_magnetic_heading		{ this, "/orientation/heading.magnetic" };
	x2::PropertyOut<si::Angle>				output_ahrs_true_heading			{ this, "/orientation/heading.true" };
	x2::PropertyOut<bool>					output_ahrs_serviceable				{ this, "/orientation/serviceable" };
	x2::PropertyOut<si::Acceleration>		output_slip_skid					{ this, "/slip-skid" };
	x2::PropertyOut<si::Angle>				output_fpm_alpha					{ this, "/fpm/alpha" };
	x2::PropertyOut<si::Angle>				output_fpm_beta						{ this, "/fpm/beta" };
	x2::PropertyOut<si::Angle>				output_magnetic_track				{ this, "/track/magnetic" };
	x2::PropertyOut<bool>					output_standard_pressure			{ this, "/standard-pressure" };
	x2::PropertyOut<si::Length>				output_altitude						{ this, "/altitude" };
	x2::PropertyOut<si::Length>				output_radar_altimeter_altitude_agl	{ this, "/radar-altimeter/altitude.agl" };
	x2::PropertyOut<bool>					output_radar_altimeter_serviceable	{ this, "/radar-altimeter/serviceable" };
	x2::PropertyOut<si::Velocity>			output_cbr							{ this, "/cbr" };
	x2::PropertyOut<si::Pressure>			output_pressure						{ this, "/pressure/pressure" };
	x2::PropertyOut<bool>					output_pressure_serviceable			{ this, "/pressure/serviceable" };
	x2::PropertyOut<si::Length>				output_cmd_alt_setting				{ this, "/cmd/altitude-setting" };
	x2::PropertyOut<si::Velocity>			output_cmd_speed_setting			{ this, "/cmd/speed-setting" };
	x2::PropertyOut<si::Angle>				output_cmd_heading_setting			{ this, "/cmd/heading-setting" };
	x2::PropertyOut<si::Velocity>			output_cmd_cbr_setting				{ this, "/cmd/cbr-setting" };
	x2::PropertyOut<si::Angle>				output_flight_director_pitch		{ this, "/flight-director/pitch" };
	x2::PropertyOut<si::Angle>				output_flight_director_roll			{ this, "/flight-director/roll" };
	x2::PropertyOut<bool>					output_navigation_needles_visible	{ this, "/navigation-needles/visible" };
	x2::PropertyOut<si::Angle>				output_lateral_deviation			{ this, "/navigation-needles/lateral-deviation" };
	x2::PropertyOut<si::Angle>				output_vertical_deviation			{ this, "/navigation-needles/vertical-deviation" };
	x2::PropertyOut<si::Length>				output_dme_distance					{ this, "/dme/distance" };
	x2::PropertyOut<si::Temperature>		output_total_air_temperature		{ this, "/total-air-temperature" };
	x2::PropertyOut<double>					output_engine_throttle_pct			{ this, "/engine-throttle-pct" };
	x2::PropertyOut<si::Force>				output_engine_1_thrust				{ this, "/engine/1/thrust" };
	x2::PropertyOut<si::AngularVelocity>	output_engine_1_rpm					{ this, "/engine/1/rpm" };
	x2::PropertyOut<si::Angle>				output_engine_1_pitch				{ this, "/engine/1/pitch" };
	x2::PropertyOut<double>					output_engine_1_epr					{ this, "/engine/1/epr" };
	x2::PropertyOut<double>					output_engine_1_n1_pct				{ this, "/engine/1/n1-pct" };
	x2::PropertyOut<double>					output_engine_1_n2_pct				{ this, "/engine/1/n2-pct" };
	x2::PropertyOut<si::Temperature>		output_engine_1_egt					{ this, "/engine/1/egt" };
	x2::PropertyOut<si::Force>				output_engine_2_thrust				{ this, "/engine/2/thrust" };
	x2::PropertyOut<si::AngularVelocity>	output_engine_2_rpm					{ this, "/engine/2/rpm" };
	x2::PropertyOut<si::Angle>				output_engine_2_pitch				{ this, "/engine/2/pitch" };
	x2::PropertyOut<double>					output_engine_2_epr					{ this, "/engine/2/epr" };
	x2::PropertyOut<double>					output_engine_2_n1_pct				{ this, "/engine/2/n1-pct" };
	x2::PropertyOut<double>					output_engine_2_n2_pct				{ this, "/engine/2/n2-pct" };
	x2::PropertyOut<si::Temperature>		output_engine_2_egt					{ this, "/engine/2/egt" };
	x2::PropertyOut<si::Angle>				output_gps_latitude					{ this, "/gps/latitude" };
	x2::PropertyOut<si::Angle>				output_gps_longitude				{ this, "/gps/longitude" };
	x2::PropertyOut<si::Length>				output_gps_amsl						{ this, "/gps/amsl" };
	x2::PropertyOut<si::Length>				output_gps_lateral_stddev			{ this, "/gps/lateral-stddev" };
	x2::PropertyOut<si::Length>				output_gps_vertical_stddev			{ this, "/gps/vertical-stddev" };
	x2::PropertyOut<bool>					output_gps_serviceable				{ this, "/gps/serviceable" };
	x2::PropertyOut<std::string>			output_gps_source					{ this, "/gps/source" };
	x2::PropertyOut<si::Angle>				output_wind_from_magnetic_heading	{ this, "/wind/heading-from.magnetic" };
	x2::PropertyOut<si::Velocity>			output_wind_tas						{ this, "/wind/tas" };
	x2::PropertyOut<bool>					output_gear_setting_down			{ this, "/gear/setting-down" };
	x2::PropertyOut<bool>					output_gear_nose_up					{ this, "/gear/nose-up" };
	x2::PropertyOut<bool>					output_gear_nose_down				{ this, "/gear/nose-down" };
	x2::PropertyOut<bool>					output_gear_left_up					{ this, "/gear/left-up" };
	x2::PropertyOut<bool>					output_gear_left_down				{ this, "/gear/left-down" };
	x2::PropertyOut<bool>					output_gear_right_up				{ this, "/gear/right-up" };
	x2::PropertyOut<bool>					output_gear_right_down				{ this, "/gear/right-down" };

  private:
	Q_OBJECT

  public:
	// Ctor
	explicit
	FlightGearIO (std::string const& instance = {});

	// Module API
	void
	initialize() override;

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
	QHostAddress						_input_address;
	Unique<QUdpSocket>					_input;
	QByteArray							_input_datagram;
	QHostAddress						_output_address;
	Unique<QUdpSocket>					_output;
	std::vector<x2::BasicProperty*>		_output_properties;
	std::vector<x2::Property<bool>*>	_serviceable_flags;
};

#endif
