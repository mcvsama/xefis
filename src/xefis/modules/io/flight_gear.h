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


class FlightGearIO: public v2::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	v2::Setting<bool>						input_enabled					{ this, "input_enabled", true };
	v2::Setting<std::string>				input_host						{ this, "input_host" };
	v2::Setting<uint16_t>					input_port						{ this, "input_port" };
	v2::Setting<bool>						output_enabled					{ this, "output_enabled", true };
	v2::Setting<std::string>				output_host						{ this, "output_host" };
	v2::Setting<uint16_t>					output_port						{ this, "output_port" };

	/*
	 * Input
	 */

	v2::PropertyIn<double>					ailerons						{ this, "/ailerons" }; // TODO
	v2::PropertyIn<double>					elevator						{ this, "/elevator" };
	v2::PropertyIn<double>					rudder							{ this, "/rudder" };
	v2::PropertyIn<double>					throttle_1						{ this, "/throttle-1" };
	v2::PropertyIn<double>					throttle_2						{ this, "/throttle-2" };
	v2::PropertyIn<double>					flaps							{ this, "/flaps" };

	/*
	 * Output
	 */

	v2::PropertyOut<si::AngularVelocity>	rotation_x						{ this, "/rotation/x" };
	v2::PropertyOut<si::AngularVelocity>	rotation_y						{ this, "/rotation/y" };
	v2::PropertyOut<si::AngularVelocity>	rotation_z						{ this, "/rotation/z" };
	v2::PropertyOut<si::Acceleration>		acceleration_x					{ this, "/acceleration/x" };
	v2::PropertyOut<si::Acceleration>		acceleration_y					{ this, "/acceleration/y" };
	v2::PropertyOut<si::Acceleration>		acceleration_z					{ this, "/acceleration/z" };
	v2::PropertyOut<si::Angle>				aoa_alpha_maximum				{ this, "/aoa/alpha.maximum" };
	v2::PropertyOut<si::Angle>				aoa_alpha_minimum				{ this, "/aoa/alpha.minimum" };
	v2::PropertyOut<si::Angle>				aoa_alpha						{ this, "/aoa/alpha" };
	v2::PropertyOut<si::Velocity>			ias								{ this, "/speeds/ias" };
	v2::PropertyOut<si::Velocity>			ias_lookahead					{ this, "/speeds/ias.lookahead" };
	v2::PropertyOut<si::Velocity>			minimum_ias						{ this, "/speeds/ias.minimum" };
	v2::PropertyOut<si::Velocity>			maximum_ias						{ this, "/speeds/ias.maximum" };
	v2::PropertyOut<bool>					ias_serviceable					{ this, "/speeds/ias.serviceable" };
	v2::PropertyOut<si::Velocity>			gs								{ this, "/speeds/gs" };
	v2::PropertyOut<si::Velocity>			tas								{ this, "/speeds/tas" };
	v2::PropertyOut<double>					mach							{ this, "/speeds/mach" };
	v2::PropertyOut<si::Angle>				ahrs_pitch						{ this, "/orientation/pitch" };
	v2::PropertyOut<si::Angle>				ahrs_roll						{ this, "/orientation/roll" };
	v2::PropertyOut<si::Angle>				ahrs_magnetic_heading			{ this, "/orientation/heading.magnetic" };
	v2::PropertyOut<si::Angle>				ahrs_true_heading				{ this, "/orientation/heading.true" };
	v2::PropertyOut<bool>					ahrs_serviceable				{ this, "/orientation/serviceable" };
	v2::PropertyOut<si::Acceleration>		slip_skid						{ this, "/slip-skid" };
	v2::PropertyOut<si::Angle>				fpm_alpha						{ this, "/fpm/alpha" };
	v2::PropertyOut<si::Angle>				fpm_beta						{ this, "/fpm/beta" };
	v2::PropertyOut<si::Angle>				magnetic_track					{ this, "/track/magnetic" };
	v2::PropertyOut<bool>					standard_pressure				{ this, "/standard-pressure" };
	v2::PropertyOut<si::Length>				altitude						{ this, "/altitude" };
	v2::PropertyOut<si::Length>				radar_altimeter_altitude_agl	{ this, "/radar-altimeter/altitude.agl" };
	v2::PropertyOut<bool>					radar_altimeter_serviceable		{ this, "/radar-altimeter/serviceable" };
	v2::PropertyOut<si::Velocity>			cbr								{ this, "/cbr" };
	v2::PropertyOut<si::Pressure>			pressure						{ this, "/pressure/pressure" };
	v2::PropertyOut<bool>					pressure_serviceable			{ this, "/pressure/serviceable" };
	v2::PropertyOut<si::Length>				cmd_alt_setting					{ this, "/cmd/altitude-setting" };
	v2::PropertyOut<si::Velocity>			cmd_speed_setting				{ this, "/cmd/speed-setting" };
	v2::PropertyOut<si::Angle>				cmd_heading_setting				{ this, "/cmd/heading-setting" };
	v2::PropertyOut<si::Velocity>			cmd_cbr_setting					{ this, "/cmd/cbr-setting" };
	v2::PropertyOut<si::Angle>				flight_director_pitch			{ this, "/flight-director/pitch" };
	v2::PropertyOut<si::Angle>				flight_director_roll			{ this, "/flight-director/roll" };
	v2::PropertyOut<bool>					navigation_needles_visible		{ this, "/navigation-needles/visible" };
	v2::PropertyOut<si::Angle>				lateral_deviation				{ this, "/navigation-needles/lateral-deviation" };
	v2::PropertyOut<si::Angle>				vertical_deviation				{ this, "/navigation-needles/vertical-deviation" };
	v2::PropertyOut<si::Length>				dme_distance					{ this, "/dme/distance" };
	v2::PropertyOut<si::Temperature>		total_air_temperature			{ this, "/total-air-temperature" };
	v2::PropertyOut<double>					engine_throttle_pct				{ this, "/engine-throttle-pct" };
	v2::PropertyOut<si::Force>				engine_1_thrust					{ this, "/engine/1/thrust" };
	v2::PropertyOut<si::AngularVelocity>	engine_1_rpm					{ this, "/engine/1/rpm" };
	v2::PropertyOut<si::Angle>				engine_1_pitch					{ this, "/engine/1/pitch" };
	v2::PropertyOut<double>					engine_1_epr					{ this, "/engine/1/epr" };
	v2::PropertyOut<double>					engine_1_n1_pct					{ this, "/engine/1/n1-pct" };
	v2::PropertyOut<double>					engine_1_n2_pct					{ this, "/engine/1/n2-pct" };
	v2::PropertyOut<si::Temperature>		engine_1_egt					{ this, "/engine/1/egt" };
	v2::PropertyOut<si::Force>				engine_2_thrust					{ this, "/engine/2/thrust" };
	v2::PropertyOut<si::AngularVelocity>	engine_2_rpm					{ this, "/engine/2/rpm" };
	v2::PropertyOut<si::Angle>				engine_2_pitch					{ this, "/engine/2/pitch" };
	v2::PropertyOut<double>					engine_2_epr					{ this, "/engine/2/epr" };
	v2::PropertyOut<double>					engine_2_n1_pct					{ this, "/engine/2/n1-pct" };
	v2::PropertyOut<double>					engine_2_n2_pct					{ this, "/engine/2/n2-pct" };
	v2::PropertyOut<si::Temperature>		engine_2_egt					{ this, "/engine/2/egt" };
	v2::PropertyOut<si::Angle>				gps_latitude					{ this, "/gps/latitude" };
	v2::PropertyOut<si::Angle>				gps_longitude					{ this, "/gps/longitude" };
	v2::PropertyOut<si::Length>				gps_amsl						{ this, "/gps/amsl" };
	v2::PropertyOut<si::Length>				gps_lateral_stddev				{ this, "/gps/lateral-stddev" };
	v2::PropertyOut<si::Length>				gps_vertical_stddev				{ this, "/gps/vertical-stddev" };
	v2::PropertyOut<bool>					gps_serviceable					{ this, "/gps/serviceable" };
	v2::PropertyOut<std::string>			gps_source						{ this, "/gps/source" };
	v2::PropertyOut<si::Angle>				wind_from_magnetic_heading		{ this, "/wind/heading-from.magnetic" };
	v2::PropertyOut<si::Velocity>			wind_tas						{ this, "/wind/tas" };
	v2::PropertyOut<bool>					gear_setting_down				{ this, "/gear/setting-down" };
	v2::PropertyOut<bool>					gear_nose_up					{ this, "/gear/nose-up" };
	v2::PropertyOut<bool>					gear_nose_down					{ this, "/gear/nose-down" };
	v2::PropertyOut<bool>					gear_left_up					{ this, "/gear/left-up" };
	v2::PropertyOut<bool>					gear_left_down					{ this, "/gear/left-down" };
	v2::PropertyOut<bool>					gear_right_up					{ this, "/gear/right-up" };
	v2::PropertyOut<bool>					gear_right_down					{ this, "/gear/right-down" };
};


class FlightGear:
	public QObject,
	public v2::Module<FlightGearIO>
{
	Q_OBJECT

  public:
	// Ctor
	explicit
	FlightGear (std::unique_ptr<FlightGearIO>, std::string const& instance = {});

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
	std::vector<v2::BasicProperty*>		_output_properties;
	std::vector<v2::Property<bool>*>	_serviceable_flags;
};

#endif
