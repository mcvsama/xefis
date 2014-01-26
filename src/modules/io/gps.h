/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__IO__GPS_H__INCLUDED
#define XEFIS__MODULES__IO__GPS_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>
#include <map>

// Qt:
#include <QtCore/QSocketNotifier>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <xefis/core/module.h>


/**
 * Read NMEA 0183 GPS data from a serial port.
 */
class GPS:
	public QObject,
	public Xefis::Module
{
	Q_OBJECT

	static constexpr const char* SET_NMEA_BAUDRATE = "PMTK251";

  public:
	// Ctor
	GPS (Xefis::ModuleManager* module_manager, QDomElement const& config);

	// Dtor
	~GPS();

	/**
	 * Return string for given fix quality code.
	 */
	static std::string
	describe_fix_quality (int code);

	/**
	 * Return string describing PMTK command.
	 * Command must be of form "PMTKnnn".
	 */
	static std::string
	describe_pmtk_command (std::string command);

  private slots:
	/**
	 * Called when there's data to read from a serial device.
	 */
	void
	read();

	/**
	 * Open device and start processing data.
	 */
	void
	open_device();

	/**
	 * Indicate failure. Try to reopen device, perhaps
	 * with other baud-rate setting.
	 */
	void
	failure (std::string const& reason);

	/**
	 * Overloaded failure to work as slot.
	 */
	void
	failure();

	/**
	 * Try to restart operation after failure was detected.
	 */
	void
	restart();

  private:
	/**
	 * Reset buffer and state. A must after a failure of some sort.
	 */
	void
	reset();

	/**
	 * Set all data properties to nil.
	 */
	void
	reset_properties();

	/**
	 * Set serial port device options, eg. baud-rate.
	 */
	bool
	set_device_options (bool use_target_baud_rate);

	/**
	 * Called when stream is synchronized and it's safe
	 * to send commands.
	 */
	void
	synchronized();

	/**
	 * Send packet requesting baud rate change
	 * and reopen device with new baud rate.
	 */
	void
	switch_baud_rate_request();

	/**
	 * Send parsed initialization commands.
	 */
	void
	initialization_commands();

	/**
	 * Process buffered messages.
	 */
	void
	process();

	/**
	 * Process single message. Message must not contain trailing \r\n.
	 * Return false if processing fails.
	 */
	bool
	process_message (std::string message);

	/**
	 * Process message: GPGGA - Global Positioning System Fix Data
	 * Return false if processing fails.
	 */
	bool
	process_gpgga (std::string message_contents);

	/**
	 * Process message: GPGSA - GPS DOP and active satellites
	 * Return false if processing fails.
	 */
	bool
	process_gpgsa (std::string message_contents);

	/**
	 * Process message: GPRMC - Recommended minimum specific GPS/Transit data
	 * Return false if processing fails.
	 */
	bool
	process_gprmc (std::string message_contents);

	/**
	 * Process PMTK ACK message.
	 */
	bool
	process_pmtk_ack (std::string message_contents);

	/**
	 * Get substring up to next comma (',') and place it into shared
	 * buffer _value. Return position of the next value (one char after
	 * the comma) or npos.
	 */
	std::string::size_type
	read_value (std::string const&, std::string::size_type start_pos);

	/**
	 * Convert ASCII digit to int.
	 */
	int
	digit_from_ascii (char c);

	/**
	 * Set system time.
	 * Take date from _date_string/_date_timestamp.
	 */
	void
	synchronize_system_clock (std::string const& date_string, std::string const& time_string);

	/**
	 * Create PMTK message. Data must include message name: PMTKnnn,
	 * where nnn is message ID.
	 */
	static std::string
	make_pmtk (std::string data);

	/**
	 * Return two-character hex checksum of given data.
	 */
	static std::string
	make_checksum (std::string data);

  private:
	static std::array<std::string, 9>			_fix_quality_strings;
	static bool									_fix_quality_strings_initialized;
	static std::map<std::string, std::string>	_pmtk_hints;
	static bool									_pmtk_hints_initialized;

	Unique<QTimer>				_restart_timer;
	Unique<QTimer>				_alive_check_timer;
	std::map<int, int>			_baud_rates_map;
	std::string					_default_baud_rate			= "9600";
	std::string					_current_baud_rate			= "9600";
	std::string					_target_baud_rate			= "9600";
	std::vector<std::string>	_pmtk_commands;
	bool						_debug_mode					= false;
	QString						_device_path;
	int							_device						= 0;
	bool						_synchronize_input			= true;
	bool						_initialization_commands	= true;
	bool						_synchronize_system_clock	= false;
	Unique<QSocketNotifier>		_notifier;
	std::string					_buffer;
	std::string					_value;
	Length						_receiver_accuracy;
	int							_failure_count				= 0;

	Xefis::PropertyBoolean		_serviceable;
	Xefis::PropertyInteger		_read_errors;
	Xefis::PropertyInteger		_fix_quality;
	Xefis::PropertyInteger		_type_of_fix;
	Xefis::PropertyAngle		_latitude;
	Xefis::PropertyAngle		_longitude;
	Xefis::PropertyLength		_altitude_amsl;
	Xefis::PropertyLength		_altitude_above_wgs84;
	Xefis::PropertySpeed		_groundspeed;
	Xefis::PropertyAngle		_track;
	Xefis::PropertyInteger		_tracked_satellites;
	Xefis::PropertyFloat		_hdop;
	Xefis::PropertyFloat		_vdop;
	Xefis::PropertyLength		_lateral_accuracy;
	Xefis::PropertyLength		_vertical_accuracy;
	Xefis::PropertyString		_dgps_station_id;
	Xefis::PropertyTime			_update_timestamp;
	Xefis::PropertyTime			_epoch_time;
};


inline void
GPS::failure()
{
	failure ("");
}


inline int
GPS::digit_from_ascii (char c)
{
	if ('0' <= c && c <= '9')
		return c - '0';
	return 0;
}

#endif

