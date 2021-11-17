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
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class FlightGearIO: public xf::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<bool>					input_enabled					{ this, "input_enabled", true };
	xf::Setting<std::string>			input_host						{ this, "input_host" };
	xf::Setting<uint16_t>				input_port						{ this, "input_port" };
	xf::Setting<bool>					output_enabled					{ this, "output_enabled", true };
	xf::Setting<std::string>			output_host						{ this, "output_host" };
	xf::Setting<uint16_t>				output_port						{ this, "output_port" };

	/*
	 * Input
	 */

	xf::ModuleIn<double>				ailerons						{ this, "ailerons" };
	xf::ModuleIn<double>				elevator						{ this, "elevator" };
	xf::ModuleIn<double>				rudder							{ this, "rudder" };
	xf::ModuleIn<double>				throttle_1						{ this, "throttle-1" };
	xf::ModuleIn<double>				throttle_2						{ this, "throttle-2" };
	xf::ModuleIn<double>				flaps							{ this, "flaps" };

	/*
	 * Output
	 */

	xf::ModuleOut<si::AngularVelocity>	rotation_x						{ this, "rotation/x" };
	xf::ModuleOut<si::AngularVelocity>	rotation_y						{ this, "rotation/y" };
	xf::ModuleOut<si::AngularVelocity>	rotation_z						{ this, "rotation/z" };
	xf::ModuleOut<si::Acceleration>		acceleration_x					{ this, "acceleration/x" };
	xf::ModuleOut<si::Acceleration>		acceleration_y					{ this, "acceleration/y" };
	xf::ModuleOut<si::Acceleration>		acceleration_z					{ this, "acceleration/z" };
	xf::ModuleOut<si::Angle>			aoa_alpha_maximum				{ this, "aoa/alpha.maximum" };
	xf::ModuleOut<si::Angle>			aoa_alpha_minimum				{ this, "aoa/alpha.minimum" };
	xf::ModuleOut<si::Angle>			aoa_alpha						{ this, "aoa/alpha" };
	xf::ModuleOut<si::Velocity>			ias								{ this, "speeds/ias" };
	xf::ModuleOut<si::Velocity>			ias_lookahead					{ this, "speeds/ias.lookahead" };
	xf::ModuleOut<si::Velocity>			minimum_ias						{ this, "speeds/ias.minimum" };
	xf::ModuleOut<si::Velocity>			maximum_ias						{ this, "speeds/ias.maximum" };
	xf::ModuleOut<bool>					ias_serviceable					{ this, "speeds/ias.serviceable" };
	xf::ModuleOut<si::Velocity>			gs								{ this, "speeds/gs" };
	xf::ModuleOut<si::Velocity>			tas								{ this, "speeds/tas" };
	xf::ModuleOut<double>				mach							{ this, "speeds/mach" };
	xf::ModuleOut<si::Angle>			ahrs_pitch						{ this, "orientation/pitch" };
	xf::ModuleOut<si::Angle>			ahrs_roll						{ this, "orientation/roll" };
	xf::ModuleOut<si::Angle>			ahrs_magnetic_heading			{ this, "orientation/heading.magnetic" };
	xf::ModuleOut<si::Angle>			ahrs_true_heading				{ this, "orientation/heading.true" };
	xf::ModuleOut<bool>					ahrs_serviceable				{ this, "orientation/serviceable" };
	xf::ModuleOut<si::Acceleration>		slip_skid						{ this, "slip-skid" };
	xf::ModuleOut<si::Angle>			fpm_alpha						{ this, "fpm/alpha" };
	xf::ModuleOut<si::Angle>			fpm_beta						{ this, "fpm/beta" };
	xf::ModuleOut<si::Angle>			magnetic_track					{ this, "track/magnetic" };
	xf::ModuleOut<bool>					standard_pressure				{ this, "standard-pressure" };
	xf::ModuleOut<si::Length>			altitude						{ this, "altitude" };
	xf::ModuleOut<si::Length>			radar_altimeter_altitude_agl	{ this, "radar-altimeter/altitude.agl" };
	xf::ModuleOut<bool>					radar_altimeter_serviceable		{ this, "radar-altimeter/serviceable" };
	xf::ModuleOut<si::Velocity>			cbr								{ this, "cbr" };
	xf::ModuleOut<si::Pressure>			pressure						{ this, "pressure/pressure" };
	xf::ModuleOut<bool>					pressure_serviceable			{ this, "pressure/serviceable" };
	xf::ModuleOut<si::Length>			cmd_alt_setting					{ this, "cmd/altitude-setting" };
	xf::ModuleOut<si::Velocity>			cmd_speed_setting				{ this, "cmd/speed-setting" };
	xf::ModuleOut<si::Angle>			cmd_heading_setting				{ this, "cmd/heading-setting" };
	xf::ModuleOut<si::Velocity>			cmd_cbr_setting					{ this, "cmd/cbr-setting" };
	xf::ModuleOut<si::Angle>			flight_director_pitch			{ this, "flight-director/pitch" };
	xf::ModuleOut<si::Angle>			flight_director_roll			{ this, "flight-director/roll" };
	xf::ModuleOut<bool>					navigation_needles_visible		{ this, "navigation-needles/visible" };
	xf::ModuleOut<si::Angle>			lateral_deviation				{ this, "navigation-needles/lateral-deviation" };
	xf::ModuleOut<si::Angle>			vertical_deviation				{ this, "navigation-needles/vertical-deviation" };
	xf::ModuleOut<si::Length>			dme_distance					{ this, "dme/distance" };
	xf::ModuleOut<si::Temperature>		total_air_temperature			{ this, "total-air-temperature" };
	xf::ModuleOut<double>				engine_throttle_pct				{ this, "engine-throttle-pct" };
	xf::ModuleOut<si::Force>			engine_1_thrust					{ this, "engine/1/thrust" };
	xf::ModuleOut<si::AngularVelocity>	engine_1_rpm					{ this, "engine/1/rpm" };
	xf::ModuleOut<si::Angle>			engine_1_pitch					{ this, "engine/1/pitch" };
	xf::ModuleOut<double>				engine_1_epr					{ this, "engine/1/epr" };
	xf::ModuleOut<double>				engine_1_n1_pct					{ this, "engine/1/n1-pct" };
	xf::ModuleOut<double>				engine_1_n2_pct					{ this, "engine/1/n2-pct" };
	xf::ModuleOut<si::Temperature>		engine_1_egt					{ this, "engine/1/egt" };
	xf::ModuleOut<si::Force>			engine_2_thrust					{ this, "engine/2/thrust" };
	xf::ModuleOut<si::AngularVelocity>	engine_2_rpm					{ this, "engine/2/rpm" };
	xf::ModuleOut<si::Angle>			engine_2_pitch					{ this, "engine/2/pitch" };
	xf::ModuleOut<double>				engine_2_epr					{ this, "engine/2/epr" };
	xf::ModuleOut<double>				engine_2_n1_pct					{ this, "engine/2/n1-pct" };
	xf::ModuleOut<double>				engine_2_n2_pct					{ this, "engine/2/n2-pct" };
	xf::ModuleOut<si::Temperature>		engine_2_egt					{ this, "engine/2/egt" };
	xf::ModuleOut<si::Angle>			gps_latitude					{ this, "gps/latitude" };
	xf::ModuleOut<si::Angle>			gps_longitude					{ this, "gps/longitude" };
	xf::ModuleOut<si::Length>			gps_amsl						{ this, "gps/amsl" };
	xf::ModuleOut<si::Length>			gps_lateral_stddev				{ this, "gps/lateral-stddev" };
	xf::ModuleOut<si::Length>			gps_vertical_stddev				{ this, "gps/vertical-stddev" };
	xf::ModuleOut<bool>					gps_serviceable					{ this, "gps/serviceable" };
	xf::ModuleOut<std::string>			gps_source						{ this, "gps/source" };
	xf::ModuleOut<si::Angle>			wind_from_magnetic_heading		{ this, "wind/heading-from.magnetic" };
	xf::ModuleOut<si::Velocity>			wind_tas						{ this, "wind/tas" };
	xf::ModuleOut<bool>					gear_setting_down				{ this, "gear/setting-down" };
	xf::ModuleOut<bool>					gear_nose_up					{ this, "gear/nose-up" };
	xf::ModuleOut<bool>					gear_nose_down					{ this, "gear/nose-down" };
	xf::ModuleOut<bool>					gear_left_up					{ this, "gear/left-up" };
	xf::ModuleOut<bool>					gear_left_down					{ this, "gear/left-down" };
	xf::ModuleOut<bool>					gear_right_up					{ this, "gear/right-up" };
	xf::ModuleOut<bool>					gear_right_down					{ this, "gear/right-down" };
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
	 * Set all input sockets as invalid.
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
	std::vector<xf::BasicModuleOut*>	_output_sockets;
	std::vector<xf::ModuleOut<bool>*>	_serviceable_flags;
};

#endif
