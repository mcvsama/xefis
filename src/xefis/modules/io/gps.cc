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

// Local:
#include "gps.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/protocols/nmea/parser.h>
#include <xefis/support/protocols/nmea/mtk.h>

// Neutrino:
#include <neutrino/bus/serial_port.h>
#include <neutrino/stdexcept.h>
#include <neutrino/time_helper.h>

// Qt:
#include <QTimer>

// Standard:
#include <cstddef>
#include <format>
#include <memory>
#include <string>
#include <ctime>


using namespace xf::exception_ops;


GPS::Connection::Connection (GPS& gps_module, PowerCycle& power_cycle, unsigned int baud_rate):
	_gps_module (gps_module),
	_power_cycle (power_cycle),
	// Store requested baud rate separately, since _serial_port_config may change it
	// (align to nearest allowed baud rate):
	_requested_physical_baud_rate (baud_rate)
{
	_gps_module.logger() << "Create GPS serial connection" << std::endl;

	_alive_check_timer = std::make_unique<QTimer> (this);
	_alive_check_timer->setInterval (kAliveCheckInterval.in<si::Millisecond>());
	_alive_check_timer->setSingleShot (true);
	QObject::connect (_alive_check_timer.get(), &QTimer::timeout, this, &Connection::alive_check_failed);

	_serial_port_config = gps_module._serial_port_config;
	_serial_port_config.set_baud_rate (baud_rate);

	_serial_port = std::make_unique<xf::SerialPort> (std::bind (&GPS::Connection::serial_data_ready, this),
													 std::bind (&GPS::Connection::serial_failure, this));
	_serial_port->set_max_read_failures (3);
	_serial_port->set_logger (_gps_module.logger().with_context ("serial port"));

	open_device();
}


GPS::Connection::~Connection()
{
	_gps_module.logger() << "Stop GPS serial connection" << std::endl;
	_gps_module.reset_data_sockets();
	_gps_module._io.serviceable = false;
	_reliable_fix_quality = false;
}


void
GPS::Connection::process()
{
	if (_serial_port && !_serial_port->flushed())
		_serial_port->flush_async();
}


unsigned int
GPS::Connection::requested_physical_baud_rate() const
{
	return _requested_physical_baud_rate;
}


void
GPS::Connection::open_device()
{
	_serial_port->set_configuration (_serial_port_config);

	bool has_thrown = xf::Exception::catch_and_log (_gps_module._logger, [&] {
		_alive_check_timer->start();
		_gps_module.logger() << "Opening device " << _gps_module._serial_port_config.device_path() << " at " << _serial_port_config.baud_rate() << " bps" << std::endl;

		if (_serial_port->open())
			initialize_device();
		else
			failure ("couldn't open serial port");
	});

	if (has_thrown)
		failure ("exception in open_device()");
}


void
GPS::Connection::initialize_device()
{
	_gps_module.logger() << "Sending initialization commands." << std::endl;
	_serial_port->write (xf::nmea::make_mtk_sentence (get_nmea_frequencies_setup_messages (_serial_port_config.baud_rate())));
	// Now send user setup commands:
	for (auto const& s: *_gps_module._io.boot_pmtk_commands)
	{
		std::string pmtk = xf::nmea::make_mtk_sentence (s);
		_serial_port->write (pmtk);
		// Even if all data can't be written now, it shall be flushed
		// eventually with _serial_port->flush_async() in process().
	}
}


void
GPS::Connection::request_new_baud_rate (unsigned int baud_rate)
{
	using namespace std::literals;

	_gps_module.logger() << "Requesting baud-rate switch from " << _serial_port_config.baud_rate() << " to " << baud_rate << std::endl;
	std::string set_baud_rate_message = xf::nmea::make_mtk_sentence (MTK_SET_NMEA_BAUDRATE + ","s + std::to_string (baud_rate));
	_serial_port->write (set_baud_rate_message);
	_serial_port->flush();
	_serial_port->close();
}


void
GPS::Connection::alive_check_failed()
{
	failure ("alive check failed");
}


void
GPS::Connection::failure (std::string_view const& reason)
{
	auto log = _gps_module.logger() << "Failure detected";

	if (!reason.empty())
		log << ": " << reason;

	log << ", closing device " << _gps_module._serial_port_config.device_path() << std::endl;
	_power_cycle.notify_connection_failure();
}


void
GPS::Connection::serial_data_ready()
{
	_nmea_parser.feed (_serial_port->input_buffer());
	_serial_port->input_buffer().clear();

	bool try_next = true;

	do {
		try_next = true;

		try {
			auto const gps_message = _nmea_parser.process_next();

			std::visit (xf::overload {
				[&] (xf::nmea::GPGGA const& msg) {
					process_nmea_sentence (msg);
				},
				[&] (xf::nmea::GPGSA const& msg) {
					process_nmea_sentence (msg);
				},
				[&] (xf::nmea::GPRMC const& msg) {
					process_nmea_sentence (msg);
				},
				[&] (xf::nmea::PMTKACK const& msg) {
					process_nmea_sentence (msg);
				},
				[&] (std::monostate) noexcept {
					try_next = false;
				},
			}, gps_message);

			if (try_next)
				_alive_check_timer->start();
		}
		catch (...)
		{
			_gps_module._io.read_errors = *_gps_module._io.read_errors + 1;
			_gps_module.logger() << "Exception when processing NMEA sentence: " << std::current_exception() << std::endl;
		}
	} while (try_next);
}


void
GPS::Connection::serial_failure()
{
	failure ("serial communication error");
}


void
GPS::Connection::process_nmea_sentence (xf::nmea::GPGGA const& sentence)
{
	message_received();

	_gps_module._io.latitude = sentence.latitude;
	_gps_module._io.longitude = sentence.longitude;

	using xf::nmea::to_string;

	if (sentence.fix_quality)
		_gps_module._io.fix_quality = to_string (*sentence.fix_quality);
	else
		_gps_module._io.fix_quality = xf::nil;

	_gps_module._io.tracked_satellites = sentence.tracked_satellites;
	_gps_module._io.altitude_amsl = sentence.altitude_amsl;
	_gps_module._io.geoid_height = sentence.geoid_height;
	_gps_module._io.dgps_station_id = sentence.dgps_station_id;
	// Use system time as reference:
	_gps_module._io.fix_system_timestamp = xf::TimeHelper::now();
	_gps_module._reliable_fix_quality = sentence.reliable_fix_quality();
}


void
GPS::Connection::process_nmea_sentence (xf::nmea::GPGSA const& sentence)
{
	message_received();

	if (sentence.fix_mode)
	{
		switch (*sentence.fix_mode)
		{
			case xf::nmea::GPSFixMode::Fix2D:
				_gps_module._io.fix_mode = "2D";
				break;

			case xf::nmea::GPSFixMode::Fix3D:
				_gps_module._io.fix_mode = "3D";
				break;

			default:
				_gps_module._io.fix_mode = xf::nil;
		}
	}
	else
		_gps_module._io.fix_mode = xf::nil;

	_gps_module._io.pdop = sentence.pdop;
	_gps_module._io.vdop = sentence.vdop;
	_gps_module._io.hdop = sentence.hdop;

	if (sentence.hdop)
		_gps_module._io.lateral_stddev = *_gps_module._io.receiver_accuracy * *sentence.hdop;
	else
		_gps_module._io.lateral_stddev = xf::nil;

	if (sentence.vdop)
		_gps_module._io.vertical_stddev = *_gps_module._io.receiver_accuracy * *sentence.vdop;
	else
		_gps_module._io.vertical_stddev = xf::nil;

	if (sentence.hdop && sentence.vdop)
		_gps_module._io.position_stddev = *_gps_module._io.receiver_accuracy * std::max (*sentence.hdop, *sentence.vdop);
	else
		_gps_module._io.position_stddev = xf::nil;
}


void
GPS::Connection::process_nmea_sentence (xf::nmea::GPRMC const& sentence)
{
	message_received();

	// If values weren't updated by GGA message, use
	// position info from RMC:
	if (_gps_module._io.latitude.valid_age() > 1.5_s)
		_gps_module._io.latitude = sentence.latitude;

	if (_gps_module._io.longitude.valid_age() > 1.5_s)
		_gps_module._io.longitude = sentence.longitude;

	_gps_module._io.ground_speed = sentence.ground_speed;
	_gps_module._io.track_true = sentence.track_true;
	_gps_module._io.magnetic_declination = sentence.magnetic_variation;

	if (sentence.fix_date && sentence.fix_time)
		_gps_module._io.fix_gps_timestamp = to_unix_time (*sentence.fix_date, *sentence.fix_time);
	else
		_gps_module._io.fix_gps_timestamp = xf::nil;

	if (sentence.receiver_status == xf::nmea::GPSReceiverStatus::Active)
		if (_gps_module._reliable_fix_quality)
			if (sentence.fix_date && sentence.fix_time)
				_gps_module.update_clock (*sentence.fix_date, *sentence.fix_time);
}


void
GPS::Connection::process_nmea_sentence (xf::nmea::PMTKACK const& sentence)
{
	message_received();

	std::string command_hint;

	if (sentence.command)
	{
		command_hint = xf::nmea::describe_mtk_command_by_id (*sentence.command);
		if (command_hint.empty())
			command_hint = *sentence.command;
	}

	if (sentence.result)
	{
		switch (*sentence.result)
		{
			case xf::nmea::MTKResult::InvalidCommand:
				_gps_module.logger() << "Invalid command/packet: " << command_hint << std::endl;
				break;

			case xf::nmea::MTKResult::UnsupportedCommand:
				_gps_module.logger() << "Unsupported command/packet: " << command_hint << std::endl;
				break;

			case xf::nmea::MTKResult::Failure:
				_gps_module.logger() << "Valid command, but action failed for: " << command_hint << std::endl;
				break;

			case xf::nmea::MTKResult::Success:
				_gps_module.logger() << "Command result: " << command_hint << ": OK" << std::endl;
				break;
		}
	}
	else
		_gps_module.logger() << "Unrecognizable MTK ACK message (no result flag): " << sentence.contents() << std::endl;
}


std::string
GPS::Connection::get_nmea_frequencies_setup_messages (unsigned int baud_rate)
{
	// Set NMEA packet frequencies.
	// Index (name):
	//   0 - GLL
	//   1 - RMC
	//   2 - VTG
	//   3 - GGA
	//   4 - GSA
	//   5 - GSV
	//   ..
	//   18 - CHN
	// Available values:
	//   0 - disabled
	//   1…5 - output a packet every 1…5 position fixes

	// *_period arguments are 1 to 5 inclusive (output every 1 to 5 position fixes).
	auto get_required_baud_rate = [](si::Time fix_interval, int gga_period, int gsa_period, int rmc_period) -> unsigned int
	{
		// Required messages and their maximum lengths:
		constexpr signed int header = 6;
		constexpr signed int epilog = 5;
		constexpr signed int GGA_maxlen = header + 10 + 9 + 1 + 9 + 1 + 1 + 2 + 4 + 7 + 1 + 7 + 1 + 5 + epilog + 14;
		constexpr signed int GSA_maxlen = header + 1 + 1 + (12 * 2) + (3 * 4) + epilog + 17;
		constexpr signed int RMC_maxlen = header + 10 + 1 + 9 + 1 + 9 + 1 + 6 + 6 + 6 + 6 + 1 + 1 + epilog + 12;

		si::Frequency gga_per_second = 1.0 / fix_interval / gga_period;
		si::Frequency gsa_per_second = 1.0 / fix_interval / gsa_period;
		si::Frequency rmc_per_second = 1.0 / fix_interval / rmc_period;

		si::Frequency byte_freq = GGA_maxlen * gga_per_second + GSA_maxlen * gsa_per_second + RMC_maxlen * rmc_per_second;
		return static_cast<unsigned int> (std::ceil (8 * byte_freq.in<si::Hertz>()));
	};

	si::Time fix_interval = 100_ms;
	int gga_period = 1;
	int gsa_period = 1;
	int rmc_period = 1;

	// Decrease frequency of messages until they fit in the baud-rate.
	// Most important are GGA, then RMC, then GSA.
	// Start with least-important messages.
	while (get_required_baud_rate (fix_interval, gga_period, gsa_period, rmc_period) > baud_rate)
	{
		if (rmc_period < 5)
			rmc_period++;
		else if (gsa_period < 5)
			gsa_period++;
		else if (gga_period < 5)
			gga_period++;
		else
		{
			gga_period = 1;
			gsa_period = 1;
			rmc_period = 1;
			fix_interval += 100_ms;
		}
	}

	unsigned int fix_interval_rounded_to_100ms = static_cast<unsigned int> (fix_interval.in<si::Millisecond>()) / 100 * 100;
	std::string mtk_set_nmea_frequencies_body = std::format ("{},0,{},0,{},{},0,0,0,0,0,0,0,0,0,0,0,0,0,0", MTK_SET_NMEA_FREQUENCIES, rmc_period, gga_period, gsa_period);
	std::string mtk_set_nmea_frequencies = xf::nmea::make_mtk_sentence (mtk_set_nmea_frequencies_body);
	std::string mtk_set_nmea_position_fix_interval_body = std::string (MTK_SET_NMEA_POSITION_FIX_INTERVAL) + "," + std::to_string (fix_interval_rounded_to_100ms);
	std::string mtk_set_nmea_position_fix_interval = xf::nmea::make_mtk_sentence (mtk_set_nmea_position_fix_interval_body);

	return mtk_set_nmea_frequencies + mtk_set_nmea_position_fix_interval;
}


inline void
GPS::Connection::message_received()
{
	if (_first_message_received)
		return;

	_first_message_received = true;
	_power_cycle.notify_connection_established();
}


GPS::PowerCycle::PowerCycle (GPS& gps_module):
	_gps_module (gps_module)
{
	// Turn on power to the device.
	_gps_module.logger() << "GPS power on" << std::endl;
	_gps_module._io.power_on = true;

	// Connection will be created in process().
}


GPS::PowerCycle::~PowerCycle()
{
	_connection.reset();
	// Turn off power to the device.
	_gps_module.logger() << "GPS power off" << std::endl;
	_gps_module._io.power_on = false;
}


void
GPS::PowerCycle::process()
{
	// _connection management is done here, since this method is called
	// from main Qt event loop, and not directly from the Connection object
	// itself.

	if (_restart_connection)
	{
		_restart_connection = false;
		_connection.reset();
	}

	if (!_connection)
	{
		++_connection_attempts;
		unsigned int baud_rate = (_connection_attempts % 2 == 0)
			? *_gps_module._io.target_baud_rate
			: *_gps_module._io.default_baud_rate;
		_connection = std::make_unique<Connection> (_gps_module, *this, baud_rate);
	}

	_connection->process();
}


void
GPS::PowerCycle::notify_connection_failure()
{
	_gps_module.logger() << "Serial connection failure." << std::endl;

	if (_connection_attempts >= kConnectionAttemptsPerPowerCycle)
		_gps_module.request_power_cycle();
	else
		_restart_connection = true;
}


void
GPS::PowerCycle::notify_connection_established()
{
	_gps_module.logger() << "Stable connection established." << std::endl;
	_gps_module._io.serviceable = true;

	// Try to use target baud rate.
	// If power cycles goes beyond max allowed number, don't try to reconnect
	// if a working connection is established. Use what we have.
	if (_gps_module._power_cycle_attempts <= kMaxRestartAttempts)
	{
		// If connection attempts < kConnectionAttemptsPerPowerCycle and power cycles num <= kMaxRestartAttempts,
		// then aim for the target-baud-rate. Otherwise try to settle at the default baud rate.
		if ((_connection_attempts <= kConnectionAttemptsPerPowerCycle) == (_connection->requested_physical_baud_rate() != *_gps_module._io.target_baud_rate))
		{
			_connection->request_new_baud_rate (*_gps_module._io.target_baud_rate);
			_restart_connection = true;
		}
	}
	else
		_gps_module.logger() << "Max connection attempts achieved, not retrying anymore." << std::endl;
}


GPS::GPS (xf::ProcessingLoop& loop, xf::System* system, xf::SerialPort::Configuration const& serial_port_config, xf::Logger const& logger, std::string_view const& instance):
	GPS_IO (loop, instance),
	_logger (logger.with_context (std::string (kLoggerScope) + "#" + instance)),
	_system (system),
	_serial_port_config (serial_port_config)
{
	// TODO check logic that setting_target_baud_rate >= setting_default_baud_rate

	_power_cycle_timer = std::make_unique<QTimer> (this);
	_power_cycle_timer->setInterval (kPowerRestartDelay.in<si::Millisecond>());
	_power_cycle_timer->setSingleShot (true);
	QObject::connect (_power_cycle_timer.get(), &QTimer::timeout, this, &GPS::power_on);

	_io.read_errors = 0;
	_io.serviceable = false;
	_io.power_on = false;
}


GPS::~GPS()
{
	_power_cycle.reset();
}


void
GPS::initialize()
{
	power_on();
}


void
GPS::process (xf::Cycle const&)
{
	// _power_cycle management is done here, since this method is called from main Qt event loop.

	if (_power_cycle_requested)
	{
		_power_cycle_requested = false;
		reset_data_sockets();
		_power_cycle.reset();
		_power_cycle_timer->start();
	}

	if (_power_cycle)
		_power_cycle->process();
}


void
GPS::power_on()
{
	++_power_cycle_attempts;
	_power_cycle = std::make_unique<PowerCycle> (*this);
}


void
GPS::request_power_cycle()
{
	_power_cycle_requested = true;
}


void
GPS::reset_data_sockets()
{
	_io.fix_quality = xf::nil;
	_io.fix_mode = xf::nil;
	_io.latitude = xf::nil;
	_io.longitude = xf::nil;
	_io.altitude_amsl = xf::nil;
	_io.geoid_height = xf::nil;
	_io.ground_speed = xf::nil;
	_io.track_true = xf::nil;
	_io.tracked_satellites = xf::nil;
	_io.magnetic_declination = xf::nil;
	_io.hdop = xf::nil;
	_io.vdop = xf::nil;
	_io.pdop = xf::nil;
	_io.lateral_stddev = xf::nil;
	_io.vertical_stddev = xf::nil;
	_io.position_stddev = xf::nil;
	_io.dgps_station_id = xf::nil;
	_io.fix_system_timestamp = xf::nil;
	_io.fix_gps_timestamp = xf::nil;
}


void
GPS::update_clock (xf::nmea::GPSDate const& date, xf::nmea::GPSTimeOfDay const& time)
{
	try {
		si::Time unix_time = xf::nmea::to_unix_time (date, time);
		// Synchronize OS clock only once:
		if (*_io.synchronize_system_clock && !_clock_synchronized)
		{
			if (_system->set_clock (unix_time))
				_logger << "System clock synchronized from GPS." << std::endl;
			_clock_synchronized = true;
		}
	}
	catch (xf::nmea::BadDateTime const& e)
	{
		_logger << "Could not use date/time information from GPS (invalid data): error message follows:" << std::endl
				<< "  " + std::string (e.what()) << std::endl;
	}
}

