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

// Standard:
#include <cstddef>
#include <memory>
#include <ctime>

// System:
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

// Lib:
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/optional.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/hextable.h>
#include <xefis/utility/resource.h>
#include <xefis/utility/serial_port.h>

// Local:
#include "gps.h"


XEFIS_REGISTER_MODULE_CLASS ("io/gps", GPS);


std::array<std::string, 9>			GPS::_fix_quality_strings;
bool								GPS::_fix_quality_strings_initialized = false;
std::map<std::string, std::string>	GPS::_pmtk_hints;
bool								GPS::_pmtk_hints_initialized = false;


GPS::GPS (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	_buffer.reserve (256);

	// Set NMEA packet frequencies:
	// 0 - GLL		0 - disabled
	// 1 - RMC		1…5 - output every one…5 position fixes
	// 2 - VTG
	// 3 - GGA
	// 4 - GSA
	// 5 - GSV
	// ..
	// 18 - CHN
	_pmtk_commands.push_back ("PMTK314,0,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0");

	parse_settings (config, {
		{ "debug", _debug_mode, false },
		{ "device", _device_path, true },
		{ "receiver-accuracy", _receiver_accuracy, true },
		{ "synchronize-system-clock", _synchronize_system_clock, false },
		{ "default-baud-rate", _default_baud_rate, true },
		{ "baud-rate", _target_baud_rate, true },
	});

	parse_properties (config, {
		{ "serviceable", _serviceable, true },
		{ "read-errors", _read_errors, true },
		{ "fix-quality", _fix_quality, true },
		{ "type-of-fix", _type_of_fix, true },
		{ "latitude", _latitude, true },
		{ "longitude", _longitude, true },
		{ "altitude-amsl", _altitude_amsl, true },
		{ "altitude-above-wgs84", _altitude_above_wgs84, true },
		{ "groundspeed", _groundspeed, true },
		{ "track", _track, true },
		{ "tracked-satellites", _tracked_satellites, true },
		{ "hdop", _hdop, true },
		{ "vdop", _vdop, true },
		{ "lateral-accuracy", _lateral_accuracy, true },
		{ "vertical-accuracy", _vertical_accuracy, true },
		{ "dgps-station-id", _dgps_station_id, true },
		{ "update-timestamp", _update_timestamp, true },
		{ "epoch-time", _epoch_time, true },
	});

	for (QDomElement& e: config)
	{
		if (e == "initialization")
		{
			for (QDomElement& pmtk: e)
			{
				if (pmtk == "pmtk")
					_pmtk_commands.push_back (pmtk.text().toStdString());
				else
					throw Xefis::Exception (QString ("element <%1> not supported in <initialization>").arg (pmtk.tagName()));
			}
		}
	}

	_current_baud_rate = _default_baud_rate;

	_restart_timer = std::make_unique<QTimer> (this);
	_restart_timer->setInterval (500);
	_restart_timer->setSingleShot (true);
	QObject::connect (_restart_timer.get(), SIGNAL (timeout()), this, SLOT (open_device()));

	_alive_check_timer = std::make_unique<QTimer> (this);
	_alive_check_timer->setInterval (2000);
	_alive_check_timer->setSingleShot (false);
	QObject::connect (_alive_check_timer.get(), SIGNAL (timeout()), this, SLOT (failure()));

	open_device();
}


GPS::~GPS()
{
	if (_device)
		::close (_device);
}


std::string
GPS::describe_fix_quality (int code)
{
	if (!_fix_quality_strings_initialized)
	{
		_fix_quality_strings[0] = "Invalid";
		_fix_quality_strings[1] = "GPS";
		_fix_quality_strings[2] = "DGPS";
		_fix_quality_strings[3] = "PPS";
		_fix_quality_strings[4] = "RTK";
		_fix_quality_strings[5] = "Floa_t RTK";
		_fix_quality_strings[6] = "Esti_mated";
		_fix_quality_strings[7] = "Manu_al";
		_fix_quality_strings[8] = "Simu_lated";
		_fix_quality_strings_initialized = true;
	}

	if (code < 0 || 8 < code)
		code = 0;
	return _fix_quality_strings[code];
}


std::string
GPS::describe_pmtk_command (std::string command)
{
	if (!_pmtk_hints_initialized)
	{
		_pmtk_hints["PMTK101"] = "hot start";
		_pmtk_hints["PMTK102"] = "warm start";
		_pmtk_hints["PMTK103"] = "cold start";
		_pmtk_hints["PMTK104"] = "full cold start";
		_pmtk_hints["PMTK220"] = "set NMEA update rate";
		_pmtk_hints["PMTK251"] = "set baud rate";
		_pmtk_hints["PMTK286"] = "enable/disable AIC mode";
		_pmtk_hints["PMTK300"] = "set fixing rate";
		_pmtk_hints["PMTK301"] = "set DGPS mode";
		_pmtk_hints["PMTK313"] = "enable/disable SBAS";
		_pmtk_hints["PMTK314"] = "set NMEA frequencies";
		_pmtk_hints["PMTK319"] = "set SBAS mode";
		_pmtk_hints["PMTK513"] = "enable/disable SBAS";
		_pmtk_hints_initialized = true;
	}

	auto h = _pmtk_hints.find (command);
	if (h != _pmtk_hints.end())
		return h->second;
	return std::string();
}


void
GPS::read()
{
	bool err = false;
	bool exc = Xefis::Exception::guard ([&] {
		// Read as much as possible:
		for (;;)
		{
			std::string::size_type prev_size = _buffer.size();
			std::string::size_type try_read = 1024;
			_buffer.resize (prev_size + try_read);
			int n = ::read (_device, &_buffer[prev_size], try_read);

			if (n < 0)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
				{
					// Nothing to read (read would block)
					_buffer.resize (prev_size);
					break;
				}
				else
				{
					log() << "Error while reading from serial port: " << strerror (errno) << std::endl;
					err = true;
					break;
				}
			}
			else
			{
				_buffer.resize (prev_size + n);
				if (n == 0)
					break;
			}
		}

		if (!err)
		{
			// Initial synchronization - discard everything up till first '$':
			if (_synchronize_input)
			{
				std::string::size_type p = _buffer.find ("$GP");
				if (p != std::string::npos)
				{
					_buffer.erase (0, p);
					_synchronize_input = false;
					synchronized();
				}
			}

			if (!_synchronize_input)
				process();
		}
	});

	if (exc || err)
		failure ("read()");
}


void
GPS::open_device()
{
	try {
		_alive_check_timer->start();

		log() << "Opening device " << _device_path.toStdString() << std::endl;

		reset();

		_device = ::open (_device_path.toStdString().c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

		if (_device < 0)
		{
			log() << "Could not open device file " << _device_path.toStdString() << ": " << strerror (errno) << std::endl;
			restart();
		}
		else
		{
			if (!set_device_options (_current_baud_rate == _target_baud_rate))
				failure ("set_device_options()");
			else
			{
				_notifier = std::make_unique<QSocketNotifier> (_device, QSocketNotifier::Read, this);
				_notifier->setEnabled (true);
				QObject::connect (_notifier.get(), SIGNAL (activated (int)), this, SLOT (read()));
			}
		}
	}
	catch (...)
	{
		failure ("exception in open_device()");
	}
}


void
GPS::failure (std::string const& reason)
{
	log() << "Failure detected" << (reason.empty() ? "" : (": " + reason)) << ", closing device " << _device_path.toStdString() << std::endl;

	_alive_check_timer->stop();

	_notifier.release();
	::close (_device);

	reset_properties();
	_serviceable.write (false);
	_failure_count += 1;

	// First: try again. If fails again, try other baud rate.
	// Use target baud rate on Odd number of _failure_count,
	// and default baud rate on even number of _failure_count.
	if (_failure_count % 2 == 0)
		_current_baud_rate = _default_baud_rate;
	else
		_current_baud_rate = _target_baud_rate;

	restart();
}


void
GPS::restart()
{
	_restart_timer->start();
}


void
GPS::reset()
{
	_synchronize_input = true;
	_buffer.clear();
}


void
GPS::reset_properties()
{
	_read_errors.set_nil();
	_fix_quality.set_nil();
	_type_of_fix.set_nil();
	_latitude.set_nil();
	_longitude.set_nil();
	_altitude_amsl.set_nil();
	_altitude_above_wgs84.set_nil();
	_groundspeed.set_nil();
	_track.set_nil();
	_tracked_satellites.set_nil();
	_hdop.set_nil();
	_vdop.set_nil();
	_lateral_accuracy.set_nil();
	_vertical_accuracy.set_nil();
	_dgps_station_id.set_nil();
	_update_timestamp.set_nil();
	_epoch_time.set_nil();
}


bool
GPS::set_device_options (bool use_target_baud_rate)
{
	log() << "Setting baud rate to " << (use_target_baud_rate ? _target_baud_rate : _current_baud_rate) << std::endl;

	reset();

	termios options;

	if (tcgetattr (_device, &options) != 0)
	{
		log() << "Failed to read serial port configuration" << std::endl;
		return false;
	}

	int baud_rate_const = use_target_baud_rate
		? Xefis::SerialPort::termios_baud_rate (_target_baud_rate)
		: Xefis::SerialPort::termios_baud_rate (_current_baud_rate);
	cfsetispeed (&options, baud_rate_const);
	cfsetospeed (&options, baud_rate_const);
	options.c_cflag |= CLOCAL | CREAD;
	// Disable parity bit:
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	// Mask the character size bits:
	options.c_cflag &= ~CSIZE;
	// Select 8 data bits:
	options.c_cflag |= CS8;
	// Disable hardware flow control:
	options.c_cflag &= ~CRTSCTS;
	// Disable XON XOFF (for transmit and receive):
	options.c_iflag &= ~(IXON | IXOFF | IXANY);
	// Min charachters to be read:
	options.c_cc[VMIN] = 0;
	// Time to wait for data (tenths of seconds):
	options.c_cc[VTIME] = 0;
	// Set output and local modes to defaults:
	options.c_oflag = 0;
	options.c_lflag = 0;

	tcflush (_device, TCIFLUSH);

	if (tcsetattr (_device, TCSANOW, &options) != 0)
	{
		log() << "Could not setup serial port: " << _device_path.toStdString() << ": " << strerror (errno) << std::endl;
		return false;
	}

	return true;
}


void
GPS::synchronized()
{
	log() << "Stream synchronized" << std::endl;
	if (_current_baud_rate != _target_baud_rate)
		switch_baud_rate_request();
	else if (_initialization_commands)
		initialization_commands();
}


void
GPS::switch_baud_rate_request()
{
	log() << "Switching baud rate from " << _current_baud_rate << " to " << _target_baud_rate << std::endl;
	std::string set_baud_rate_message = make_pmtk (SET_NMEA_BAUDRATE + ","_str + _target_baud_rate);
	::write (_device, set_baud_rate_message.data(), set_baud_rate_message.size());
	::fsync (_device);
	::close (_device);
	_current_baud_rate = _target_baud_rate;
	open_device();
}


void
GPS::initialization_commands()
{
	log() << "Sending initialization commands" << std::endl;
	for (auto s: _pmtk_commands)
	{
		std::string pmtk = make_pmtk (s);
		::write (_device, pmtk.data(), pmtk.size());
	}
	::fsync (_device);
}


void
GPS::process()
{
	// Process all messages terminated with "\r\n".
	std::string::size_type start = 0;
	std::string::size_type crlf = 0;
	std::string::size_type parsed = 0;

	Xefis::Resource remove_parsed_properties ([&] {
		_buffer.erase (0, parsed);
	});

	for (;;)
	{
		crlf = _buffer.find ("\r\n", start);
		if (crlf == std::string::npos)
			break;
		parsed = crlf + 2;

		Xefis::Exception::guard ([&] {
			if (process_message (_buffer.substr (start, crlf - start)))
			{
				_serviceable.write (true);
				_failure_count = 0;
				_alive_check_timer->start();
			}
			else
			{
				if (_read_errors.configured())
					_read_errors.write (*_read_errors + 1);
			}
		});

		start = parsed;
	}
}


bool
GPS::process_message (std::string message)
{
	static Xefis::HexTable hextable;

	// Must be at least 5 bytes long to calculate checksum:
	if (message.size() < 5)
	{
		log() << "Read error: packet too short: " << message.size() << " characters" << std::endl;
		return false;
	}

	// Prologue:
	if (message[0] != '$')
	{
		log() << "Read error: packet does not start with '$'" << std::endl;
		return false;
	}

	// Checksum:
	if (message[message.size() - 3] != '*')
	{
		log() << "Read error: missing '*' at the end of packet" << std::endl;
		return false;
	}

	// Message checksum:
	char c1 = message[message.size() - 2];
	char c2 = message[message.size() - 1];
	if (!std::isxdigit (c1) || !std::isxdigit (c2))
	{
		log() << "Read error: checksum characters not valid hexadecimal values" << std::endl;
		return false;
	}
	uint8_t checksum = hextable[c1] * 16 + hextable[c2];

	// Our checksum:
	uint8_t sum = 0;
	for (std::string::iterator c = message.begin() + 1; c != message.end() && *c != '*'; ++c)
		sum ^= *c;

	if (sum != checksum)
	{
		log() << "Read error: checksum invalid" << std::endl;
		return false;
	}

	std::string contents = message.substr (1, message.size() - 1 - 3);
	std::string type = contents.substr (0, contents.find (','));

	if (_debug_mode)
		log() << "Read: " << contents << std::endl;

	bool result = true;

	Xefis::Exception::guard ([&] {
		try {
			if (type == "GPGGA")
				result = process_gpgga (contents);
			else if (type == "GPGSA")
				result = process_gpgsa (contents);
			else if (type == "GPRMC")
				result = process_gprmc (contents);
			else if (type == "PMTK001")
				result = process_pmtk_ack (contents);
			// Silently ignore unsupported messages.
		}
		catch (...)
		{
			result = false;
			throw;
		}
	});

	if (!result)
		log() << "Failed to process message: " << contents << std::endl;

	return result;
}


bool
GPS::process_gpgga (std::string message_contents)
{
	// Skip message name:
	std::string::size_type p = read_value (message_contents, 0);

	// UTC time - skip. Will be read from RMC message.
	p = read_value (message_contents, p);

	// Latitude:
	Optional<Angle> latitude;
	p = read_value (message_contents, p);
	if (_value.size() >= 3)
	{
		latitude = 1_deg * (digit_from_ascii (_value[0]) * 10 +
							digit_from_ascii (_value[1]));
		try {
			latitude = *latitude + 1_deg * boost::lexical_cast<double> (_value.substr (2)) / 60.0;
		}
		catch (boost::bad_lexical_cast&)
		{
			latitude.reset();
		}
	}

	// N-S
	p = read_value (message_contents, p);
	if (_value == "S")
		latitude = -1 * *latitude;
	else if (_value != "N")
		latitude.reset();

	// Longitude:
	Optional<Angle> longitude;
	p = read_value (message_contents, p);
	if (_value.size() >= 4)
	{
		longitude = 1_deg * (digit_from_ascii (_value[0]) * 100 +
							 digit_from_ascii (_value[1]) * 10 +
							 digit_from_ascii (_value[2]));
		try {
			longitude = *longitude + 1_deg * boost::lexical_cast<double> (_value.substr (3)) / 60.0;
		}
		catch (boost::bad_lexical_cast&)
		{
			longitude.reset();
		}
	}

	// E-W
	p = read_value (message_contents, p);
	if (_value == "W")
		longitude = -1 * *longitude;
	else if (_value != "E")
		longitude.reset();

	// Fix quality:
	p = read_value (message_contents, p);
	int fix_quality = 0;
	if (_value.size() == 1 && std::isdigit (_value[0]))
		fix_quality = digit_from_ascii (_value[0]);

	// Number of tracked satellites:
	Optional<Xefis::PropertyInteger::Type> tracked_satellites;
	p = read_value (message_contents, p);
	try {
		tracked_satellites = boost::lexical_cast<Xefis::PropertyInteger::Type> (_value);
	}
	catch (boost::bad_lexical_cast&)
	{
		tracked_satellites.reset();
	}

	// Horizontal dilusion of precision - skip. Will be taken from GSA message.
	p = read_value (message_contents, p);

	// Altitude above mean sea level (meters):
	Optional<Length> altitude_amsl;
	p = read_value (message_contents, p);
	try {
		altitude_amsl = 1_m * boost::lexical_cast<double> (_value);
	}
	catch (boost::bad_lexical_cast&)
	{
		altitude_amsl.reset();
	}
	// Read unit: M
	p = read_value (message_contents, p);
	if (_value != "M")
		altitude_amsl.reset();

	// Height above WGS84 geoid (meters):
	Optional<Length> altitude_above_wgs84;
	p = read_value (message_contents, p);
	try {
		altitude_above_wgs84 = 1_m * boost::lexical_cast<double> (_value);
	}
	catch (boost::bad_lexical_cast&)
	{
		altitude_above_wgs84.reset();
	}
	// Read unit: M
	p = read_value (message_contents, p);
	if (_value != "M")
		altitude_above_wgs84.reset();

	// Time since last DGPS update:
	p = read_value (message_contents, p);

	// DGPS station identifier:
	p = read_value (message_contents, p);
	std::string dgps_station_id = _value;

	// Set properties:

	_fix_quality.write (fix_quality);
	_latitude.write (latitude);
	_longitude.write (longitude);
	_altitude_amsl.write (altitude_amsl);
	_altitude_above_wgs84.write (altitude_above_wgs84);
	_tracked_satellites.write (tracked_satellites);
	_dgps_station_id.write (dgps_station_id);
	_update_timestamp.write (Time::now());

	return true;
}


bool
GPS::process_gpgsa (std::string message_contents)
{
	// Skip message name:
	std::string::size_type p = read_value (message_contents, 0);
	// Skip A/M (Auto/Manual selection of 2D/3D fix)
	p = read_value (message_contents, p);

	// Type of fix: none, 2D, 3D:
	Optional<Xefis::PropertyInteger::Type> type_of_fix;
	p = read_value (message_contents, p);
	try {
		type_of_fix = boost::lexical_cast<Xefis::PropertyInteger::Type> (_value);
	}
	catch (boost::bad_lexical_cast&)
	{ }

	if (!type_of_fix || (*type_of_fix != 2 && *type_of_fix != 3))
		type_of_fix = 0;

	// Skip PRNs of satellites used for the fix:
	for (int i = 0; i < 12; ++i)
		p = read_value (message_contents, p);

	// PDOP - skip:
	p = read_value (message_contents, p);

	// HDOP - if available, use it:
	Optional<double> hdop;
	p = read_value (message_contents, p);
	try {
		hdop = boost::lexical_cast<double> (_value);
	}
	catch (boost::bad_lexical_cast&)
	{ }

	// VDOP - Vertical Dilusion of Precision:
	Optional<double> vdop;
	p = read_value (message_contents, p);
	try {
		vdop = boost::lexical_cast<double> (_value);
	}
	catch (boost::bad_lexical_cast&)
	{ }

	// Set properties:

	if (type_of_fix)
		_type_of_fix.write (type_of_fix);
	else
		_type_of_fix.set_nil();

	_hdop.write (hdop);
	if (hdop)
		_lateral_accuracy.write (_receiver_accuracy * *hdop);
	else
		_lateral_accuracy.set_nil();

	_vdop.write (vdop);
	if (vdop)
		_vertical_accuracy.write (_receiver_accuracy * *vdop);
	else
		_vertical_accuracy.set_nil();

	return true;
}


bool
GPS::process_gprmc (std::string message_contents)
{
	// Skip message name:
	std::string::size_type p = read_value (message_contents, 0);

	// UTC time:
	p = read_value (message_contents, p);
	std::string time_string = _value;

	// Skip status (A or V):
	p = read_value (message_contents, p);

	// Skip lat/N-S/lon/E-W:
	p = read_value (message_contents, p);
	p = read_value (message_contents, p);
	p = read_value (message_contents, p);
	p = read_value (message_contents, p);

	// Groundspeed:
	Optional<Speed> groundspeed;
	p = read_value (message_contents, p);
	try {
		groundspeed = 1_kt * boost::lexical_cast<double> (_value);
	}
	catch (boost::bad_lexical_cast&)
	{ }

	// Track:
	Optional<Angle> track;
	p = read_value (message_contents, p);
	try {
		track = 1_deg * boost::lexical_cast<double> (_value);
	}
	catch (boost::bad_lexical_cast&)
	{ }

	// Date:
	p = read_value (message_contents, p);
	std::string date_string = _value;

	// Rest is magnetic variation - skip.

	// Set properties:

	_groundspeed.write (groundspeed);
	_track.write (track);

	// Synchronize system clock only if there's a fix:
	int type_of_fix = *_type_of_fix;
	if (type_of_fix == 2 || type_of_fix == 3)
		synchronize_system_clock (date_string, time_string);

	return true;
}


bool
GPS::process_pmtk_ack (std::string message_contents)
{
	// Skip message name:
	std::string::size_type p = read_value (message_contents, 0);

	// PMTK command number:
	p = read_value (message_contents, p);
	std::string command = "PMTK" + _value;
	std::string command_hint = describe_pmtk_command (command);
	if (command_hint.empty())
		command_hint = command;

	// Result:
	p = read_value (message_contents, p);
	std::string result = _value;

	if (result == "0")
		log() << "Invalid command/packet: " << command_hint << std::endl;
	else if (result == "1")
		log() << "Unsupported command/packet: " << command_hint << std::endl;
	else if (result == "2")
		log() << "Valid command, but action failed: " << command_hint << std::endl;
	else if (result == "3")
		log() << "Command OK: " << command_hint << std::endl;

	return true;
}


std::string::size_type
GPS::read_value (std::string const& source, std::string::size_type start_pos)
{
	std::string::size_type comma = source.find (',', start_pos);
	if (comma == std::string::npos)
	{
		_value.resize (source.size() - start_pos);
		std::copy (source.begin() + start_pos, source.end(), _value.begin());
		return std::string::npos;
	}
	else
	{
		_value.resize (comma - start_pos);
		std::copy (source.begin() + start_pos, source.begin() + comma, _value.begin());
		return comma + 1;
	}
}


void
GPS::synchronize_system_clock (std::string const& date_string, std::string const& time_string)
{
	// Xefis executable needs CAP_SYS_TIME capability, set with "setcap cap_sys_time+ep xefis".

	if (time_string.size() < 6 || date_string.size() != 6)
	{
		log() << "Could not parse time value from GPS message" << std::endl;
		return;
	}

	try {
		int hh = boost::lexical_cast<int> (time_string.substr (0, 2));
		int mm = boost::lexical_cast<int> (time_string.substr (2, 2));
		int ss = boost::lexical_cast<int> (time_string.substr (4, 2));
		double fraction = boost::lexical_cast<double> ("0" + time_string.substr (6));

		int dd = boost::lexical_cast<int> (date_string.substr (0, 2));
		int mo = boost::lexical_cast<int> (date_string.substr (2, 2));
		int yy = boost::lexical_cast<int> (date_string.substr (4, 2));

		struct tm timeinfo;
		timeinfo.tm_sec = ss;
		timeinfo.tm_min = mm;
		timeinfo.tm_hour = hh;
		timeinfo.tm_mday = dd;
		timeinfo.tm_mon = mo - 1;
		timeinfo.tm_year = 2000 + yy - 1900;
		timeinfo.tm_wday = -1;
		timeinfo.tm_yday = -1;
		timeinfo.tm_isdst = -1;

		time_t now = mktime (&timeinfo);
		if (now >= 0)
		{
			_epoch_time.write (1_s * (1.0 * now + fraction));

			if (_synchronize_system_clock)
			{
				::timeval tv = { now, 0 };
				if (::settimeofday (&tv, nullptr) < 0)
				{
					log() << "Could not setup system time: settimeofday() failed with error '" << strerror (errno) << "'; "
							 "ensure that Xefis executable has cap_sys_time capability set with 'setcap cap_sys_time+ep path-to-xefis-executable'" << std::endl;
				}
				else
					log() << "System clock synchronization OK" << std::endl;

				_synchronize_system_clock = false;
			}
		}
		else
		{
			log() << "Could not convert time information from GPS to system time" << std::endl;
			_epoch_time.set_nil();
		}
	}
	catch (boost::bad_lexical_cast&)
	{ }
}


std::string
GPS::make_pmtk (std::string data)
{
	return "$" + data + "*" + make_checksum (data) + "\r\n";
}


std::string
GPS::make_checksum (std::string data)
{
	uint8_t sum = 0;
	for (std::string::iterator c = data.begin(); c != data.end(); ++c)
		sum ^= *c;
	return (boost::format ("%02X") % static_cast<int> (sum)).str();
}

