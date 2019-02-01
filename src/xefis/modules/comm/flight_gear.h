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

#ifndef XEFIS__MODULES__COMM__FLIGHT_GEAR_H__INCLUDED
#define XEFIS__MODULES__COMM__FLIGHT_GEAR_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>

// Qt:
#include <QtCore/QTimer>
#include <QtCore/QObject>
#include <QtNetwork/QUdpSocket>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/core/setting.h>


class FlightGearIO: public xf::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<bool>						input_enabled					{ this, "input_enabled", true };
	xf::Setting<std::string>				input_host						{ this, "input_host" };
	xf::Setting<uint16_t>					input_port						{ this, "input_port" };
	xf::Setting<bool>						output_enabled					{ this, "output_enabled", true };
	xf::Setting<std::string>				output_host						{ this, "output_host" };
	xf::Setting<uint16_t>					output_port						{ this, "output_port" };

	/*
	 * Input
	 */

	xf::PropertyIn<double>					ailerons						{ this, "ailerons" };
	xf::PropertyIn<double>					elevator						{ this, "elevator" };
	xf::PropertyIn<double>					rudder							{ this, "rudder" };
	xf::PropertyIn<double>					throttle_1						{ this, "throttle-1" };
	xf::PropertyIn<double>					throttle_2						{ this, "throttle-2" };
	xf::PropertyIn<double>					flaps							{ this, "flaps" };

	/*
	 * Output
	 */

	xf::PropertyOut<si::AngularVelocity>	rotation_x						{ this, "rotation/x" };
	xf::PropertyOut<si::AngularVelocity>	rotation_y						{ this, "rotation/y" };
	xf::PropertyOut<si::AngularVelocity>	rotation_z						{ this, "rotation/z" };
	xf::PropertyOut<si::Acceleration>		acceleration_x					{ this, "acceleration/x" };
	xf::PropertyOut<si::Acceleration>		acceleration_y					{ this, "acceleration/y" };
	xf::PropertyOut<si::Acceleration>		acceleration_z					{ this, "acceleration/z" };
	xf::PropertyOut<si::Angle>				aoa_alpha_maximum				{ this, "aoa/alpha.maximum" };
	xf::PropertyOut<si::Angle>				aoa_alpha_minimum				{ this, "aoa/alpha.minimum" };
	xf::PropertyOut<si::Angle>				aoa_alpha						{ this, "aoa/alpha" };
	xf::PropertyOut<si::Velocity>			ias								{ this, "speeds/ias" };
	xf::PropertyOut<si::Velocity>			ias_lookahead					{ this, "speeds/ias.lookahead" };
	xf::PropertyOut<si::Velocity>			minimum_ias						{ this, "speeds/ias.minimum" };
	xf::PropertyOut<si::Velocity>			maximum_ias						{ this, "speeds/ias.maximum" };
	xf::PropertyOut<bool>					ias_serviceable					{ this, "speeds/ias.serviceable" };
	xf::PropertyOut<si::Velocity>			gs								{ this, "speeds/gs" };
	xf::PropertyOut<si::Velocity>			tas								{ this, "speeds/tas" };
	xf::PropertyOut<double>					mach							{ this, "speeds/mach" };
	xf::PropertyOut<si::Angle>				ahrs_pitch						{ this, "orientation/pitch" };
	xf::PropertyOut<si::Angle>				ahrs_roll						{ this, "orientation/roll" };
	xf::PropertyOut<si::Angle>				ahrs_magnetic_heading			{ this, "orientation/heading.magnetic" };
	xf::PropertyOut<si::Angle>				ahrs_true_heading				{ this, "orientation/heading.true" };
	xf::PropertyOut<bool>					ahrs_serviceable				{ this, "orientation/serviceable" };
	xf::PropertyOut<si::Acceleration>		slip_skid						{ this, "slip-skid" };
	xf::PropertyOut<si::Angle>				fpm_alpha						{ this, "fpm/alpha" };
	xf::PropertyOut<si::Angle>				fpm_beta						{ this, "fpm/beta" };
	xf::PropertyOut<si::Angle>				magnetic_track					{ this, "track/magnetic" };
	xf::PropertyOut<bool>					standard_pressure				{ this, "standard-pressure" };
	xf::PropertyOut<si::Length>				altitude						{ this, "altitude" };
	xf::PropertyOut<si::Length>				radar_altimeter_altitude_agl	{ this, "radar-altimeter/altitude.agl" };
	xf::PropertyOut<bool>					radar_altimeter_serviceable		{ this, "radar-altimeter/serviceable" };
	xf::PropertyOut<si::Velocity>			cbr								{ this, "cbr" };
	xf::PropertyOut<si::Pressure>			pressure						{ this, "pressure/pressure" };
	xf::PropertyOut<bool>					pressure_serviceable			{ this, "pressure/serviceable" };
	xf::PropertyOut<si::Length>				cmd_alt_setting					{ this, "cmd/altitude-setting" };
	xf::PropertyOut<si::Velocity>			cmd_speed_setting				{ this, "cmd/speed-setting" };
	xf::PropertyOut<si::Angle>				cmd_heading_setting				{ this, "cmd/heading-setting" };
	xf::PropertyOut<si::Velocity>			cmd_cbr_setting					{ this, "cmd/cbr-setting" };
	xf::PropertyOut<si::Angle>				flight_director_pitch			{ this, "flight-director/pitch" };
	xf::PropertyOut<si::Angle>				flight_director_roll			{ this, "flight-director/roll" };
	xf::PropertyOut<bool>					navigation_needles_visible		{ this, "navigation-needles/visible" };
	xf::PropertyOut<si::Angle>				lateral_deviation				{ this, "navigation-needles/lateral-deviation" };
	xf::PropertyOut<si::Angle>				vertical_deviation				{ this, "navigation-needles/vertical-deviation" };
	xf::PropertyOut<si::Length>				dme_distance					{ this, "dme/distance" };
	xf::PropertyOut<si::Temperature>		total_air_temperature			{ this, "total-air-temperature" };
	xf::PropertyOut<double>					engine_throttle_pct				{ this, "engine-throttle-pct" };
	xf::PropertyOut<si::Force>				engine_1_thrust					{ this, "engine/1/thrust" };
	xf::PropertyOut<si::AngularVelocity>	engine_1_rpm					{ this, "engine/1/rpm" };
	xf::PropertyOut<si::Angle>				engine_1_pitch					{ this, "engine/1/pitch" };
	xf::PropertyOut<double>					engine_1_epr					{ this, "engine/1/epr" };
	xf::PropertyOut<double>					engine_1_n1_pct					{ this, "engine/1/n1-pct" };
	xf::PropertyOut<double>					engine_1_n2_pct					{ this, "engine/1/n2-pct" };
	xf::PropertyOut<si::Temperature>		engine_1_egt					{ this, "engine/1/egt" };
	xf::PropertyOut<si::Force>				engine_2_thrust					{ this, "engine/2/thrust" };
	xf::PropertyOut<si::AngularVelocity>	engine_2_rpm					{ this, "engine/2/rpm" };
	xf::PropertyOut<si::Angle>				engine_2_pitch					{ this, "engine/2/pitch" };
	xf::PropertyOut<double>					engine_2_epr					{ this, "engine/2/epr" };
	xf::PropertyOut<double>					engine_2_n1_pct					{ this, "engine/2/n1-pct" };
	xf::PropertyOut<double>					engine_2_n2_pct					{ this, "engine/2/n2-pct" };
	xf::PropertyOut<si::Temperature>		engine_2_egt					{ this, "engine/2/egt" };
	xf::PropertyOut<si::Angle>				gps_latitude					{ this, "gps/latitude" };
	xf::PropertyOut<si::Angle>				gps_longitude					{ this, "gps/longitude" };
	xf::PropertyOut<si::Length>				gps_amsl						{ this, "gps/amsl" };
	xf::PropertyOut<si::Length>				gps_lateral_stddev				{ this, "gps/lateral-stddev" };
	xf::PropertyOut<si::Length>				gps_vertical_stddev				{ this, "gps/vertical-stddev" };
	xf::PropertyOut<bool>					gps_serviceable					{ this, "gps/serviceable" };
	xf::PropertyOut<std::string>			gps_source						{ this, "gps/source" };
	xf::PropertyOut<si::Angle>				wind_from_magnetic_heading		{ this, "wind/heading-from.magnetic" };
	xf::PropertyOut<si::Velocity>			wind_tas						{ this, "wind/tas" };
	xf::PropertyOut<bool>					gear_setting_down				{ this, "gear/setting-down" };
	xf::PropertyOut<bool>					gear_nose_up					{ this, "gear/nose-up" };
	xf::PropertyOut<bool>					gear_nose_down					{ this, "gear/nose-down" };
	xf::PropertyOut<bool>					gear_left_up					{ this, "gear/left-up" };
	xf::PropertyOut<bool>					gear_left_down					{ this, "gear/left-down" };
	xf::PropertyOut<bool>					gear_right_up					{ this, "gear/right-up" };
	xf::PropertyOut<bool>					gear_right_down					{ this, "gear/right-down" };
};


class FlightGear:
	public QObject,
	public xf::Module<FlightGearIO>
{
	Q_OBJECT

  public:
	// Ctor
	explicit
	FlightGear (std::unique_ptr<FlightGearIO>, std::string_view const& instance = {});

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
	std::unique_ptr<QTimer>				_timeout_timer;
	QHostAddress						_input_address;
	std::unique_ptr<QUdpSocket>			_input;
	QByteArray							_input_datagram;
	QHostAddress						_output_address;
	std::unique_ptr<QUdpSocket>			_output;
	std::vector<xf::BasicPropertyOut*>	_output_properties;
	std::vector<xf::PropertyOut<bool>*>	_serviceable_flags;
};

#endif
