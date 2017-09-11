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
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>
#include <xefis/core/system.h>
#include <xefis/support/bus/serial_port.h>
#include <xefis/support/protocols/nmea/parser.h>


class GPS_IO: public v2::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	v2::Setting<std::vector<std::string>>	boot_pmtk_commands			{ this, "boot_pmtk_commands", { } };
	v2::Setting<unsigned int>				default_baud_rate			{ this, "default_baud_rate", 9600 };
	v2::Setting<unsigned int>				target_baud_rate			{ this, "target_baud_rate", 9600 };
	v2::Setting<si::Length>					receiver_accuracy			{ this, "receiver_accuracy" };
	v2::Setting<bool>						synchronize_system_clock	{ this, "synchronize_system_clock", false };

	/*
	 * Output
	 */

	// Number of serial read failures.
	v2::PropertyOut<int64_t>				read_errors					{ this, "/read-errors" };	// Managed by Connection object.
	// True if GPS device is serviceable:
	v2::PropertyOut<bool>					serviceable					{ this, "/serviceable" };	// Managed by Connection object.
	// Manager power to the GPS device:
	v2::PropertyOut<bool>					power_on					{ this, "/power-on" };		// Managed by PowerCycle object.

	v2::PropertyOut<std::string>			fix_quality					{ this, "/gps/fix-quality" };
	v2::PropertyOut<std::string>			fix_mode					{ this, "/gps/mode" };		// "2D" or "3D"
	v2::PropertyOut<si::Angle>				latitude					{ this, "/gps/latitude" };
	v2::PropertyOut<si::Angle>				longitude					{ this, "/gps/longitude" };
	v2::PropertyOut<si::Length>				altitude_amsl				{ this, "/gps/altitude-amsl" };
	v2::PropertyOut<si::Length>				geoid_height				{ this, "/gps/geoid-height" };
	v2::PropertyOut<si::Velocity>			ground_speed				{ this, "/gps/ground-speed" };
	v2::PropertyOut<si::Angle>				track_true					{ this, "/gps/track.true" };
	v2::PropertyOut<int64_t>				tracked_satellites			{ this, "/gps/tracked-satellites" };
	v2::PropertyOut<si::Angle>				magnetic_declination		{ this, "/gps/magnetic-declination" };
	v2::PropertyOut<double>					hdop						{ this, "/gps/hdop" };
	v2::PropertyOut<double>					vdop						{ this, "/gps/vdop" };
	v2::PropertyOut<double>					pdop						{ this, "/gps/pdop" };
	v2::PropertyOut<si::Length>				lateral_stddev				{ this, "/gps/lateral-stddev" };
	v2::PropertyOut<si::Length>				vertical_stddev				{ this, "/gps/vertical-stddev" };
	v2::PropertyOut<si::Length>				position_stddev				{ this, "/gps/position-stddev" };
	v2::PropertyOut<int64_t>				dgps_station_id				{ this, "/gps/dgps-station-id" };
	v2::PropertyOut<si::Time>				fix_system_timestamp		{ this, "/gps/fix/system-timestamp" };
	v2::PropertyOut<si::Time>				fix_gps_timestamp			{ this, "/gps/fix/gps-timestamp" };
};


/**
 * Warning: this module uses I/O in main thread, which may block.
 *
 * Read NMEA 0183 GPS data from a serial port.
 * TODO make a thread-object that handles the device in a separate thread.
 */
class GPS:
	public QObject,
	public v2::Module<GPS_IO>
{
	Q_OBJECT

  private:
	static constexpr unsigned int	kConnectionAttemptsPerPowerCycle	= 4;
	static constexpr Time			kPowerRestartDelay					= 1_s;
	static constexpr Time			kAliveCheckInterval					= 2_s;
	static constexpr unsigned int	kMaxRestartAttempts					= 2;
	static constexpr const char*	MTK_SET_NMEA_BAUDRATE				= "PMTK251";
	static constexpr const char*	MTK_SET_NMEA_FREQUENCIES			= "PMTK314";
	static constexpr const char*	MTK_SET_NMEA_POSITION_FIX_INTERVAL	= "PMTK220";

	class PowerCycle;

	/**
	 * Represents single GPS connection.
	 * Serializes instructions for connecting to GPS with SerialPort, initializing it, switching
	 * baud-rates, etc.
	 */
	class Connection:
		public QObject,
		public xf::nmea::Parser::Listener
	{
	  public:
		/**
		 * Initializes the GPS device.
		 * Configures it appropriately for requested baud-rate.
		 *
		 * \param	baud_rate
		 *			Use this baud rate when opening serial connection.
		 */
		explicit
		Connection (GPS& gps_module, PowerCycle& power_cycle, unsigned int baud_rate);

		// Dtor
		~Connection();

		/**
		 * Called from GPS::process().
		 */
		void
		process();

		/**
		 * Baud rate as requested during construction.
		 */
		unsigned int
		requested_physical_baud_rate() const;

		/**
		 * Request baud-rate change to desired value over the MTK protocol.
		 * This sends command to the GPS to change the baud-rate. The device then becomes
		 * disconnected and new Connection object needs to be created using new baud-rate.
		 */
		void
		request_new_baud_rate (unsigned int new_baud_rate);

	  private:
		/**
		 * Open device and start processing data.
		 */
		void
		open_device();

		/**
		 * Initialize GPS device.
		 * Send initial MTK commands, etc.
		 */
		void
		initialize_device();

		/**
		 * Send packet requesting baud rate change to the target (high-speed) baud-rate.
		 */
		void
		request_target_baud_rate();

		/**
		 * Called when device doesn't respond for a while.
		 */
		void
		alive_check_failed();

		/**
		 * Indicate failure. Try to reopen device, perhaps with other baud-rate setting.
		 */
		void
		failure (std::string const& reason);

		/**
		 * Callback from SerialPort.
		 */
		void
		serial_data_ready();

		/**
		 * Callback from SerialPort.
		 */
		void
		serial_failure();

		/**
		 * Process message: GPGGA - Global Positioning System Fix Data.
		 */
		void
		process_nmea_sentence (xf::nmea::GPGGA const&);

		/**
		 * Process message: GPGSA - GPS DOP and active satellites.
		 */
		void
		process_nmea_sentence (xf::nmea::GPGSA const&);

		/**
		 * Process message: GPRMC - Recommended minimum specific GPS/Transit data.
		 */
		void
		process_nmea_sentence (xf::nmea::GPRMC const&);

		/**
		 * Process MTK ACK message.
		 */
		void
		process_nmea_sentence (xf::nmea::PMTKACK const&);

		/**
		 * Compute maximum reliable NMEA messages frequency that can be requested from GPS device
		 * without overloading serial port. Result is based on serial port baud rate.
		 */
		static std::string
		get_nmea_frequencies_setup_messages (unsigned int baud_rate);

		/**
		 * Notify (once!) PowerCycle that stable connection is established.
		 */
		void
		message_received();

	  private:
		// Parents:
		GPS&							_gps_module;
		PowerCycle&						_power_cycle;

		// Used to restart after a while if device doesn't respond:
		Unique<QTimer>					_alive_check_timer;
		unsigned int					_requested_physical_baud_rate;
		xf::SerialPort::Configuration	_serial_port_config;
		Unique<xf::SerialPort>			_serial_port;
		Unique<xf::nmea::Parser>		_nmea_parser;
		bool							_reliable_fix_quality	= false;
		bool							_first_message_received	= false;
	};

	/**
	 * Represents single power-on..power-off cycle for the GPS device.
	 * Uses (creates) Connection objects that manage device communication stuff.
	 * Switches baud-rates when appropriate.
	 */
	class PowerCycle
	{
	  public:
		// Ctor
		explicit
		PowerCycle (GPS& gps_module);

		// Dtor
		~PowerCycle();

		/**
		 * Called from GPS::process().
		 * Takes care of actually allocating and destroying of Connections.
		 */
		void
		process();

		/**
		 * Notify that connection error has occured.
		 * It will try to restart the connection with alternate
		 * baud-rates.
		 */
		void
		notify_connection_failure();

		/**
		 * Notify that connection has been established.
		 * It will try to recreate Connection with the target baud
		 * rate, if not yet set.
		 */
		void
		notify_connection_established();

	  private:
		// Parent:
		GPS&				_gps_module;

		Unique<Connection>	_connection;
		// On odd connection attempts, default baud-rate will be used,
		// on even - target baud rate.
		unsigned int		_connection_attempts	= 0;
		// Indicates that Connection restart has been requested:
		bool				_restart_connection		= false;
	};

  public:
	// Ctor
	explicit
	GPS (std::unique_ptr<GPS_IO>, xf::System*, xf::SerialPort::Configuration const&, std::string const& instance = {});

	// Dtor
	~GPS();

	// Module API
	void
	initialize() override;

	// Module API
	void
	process (v2::Cycle const&) override;

  private slots:
	/**
	 * Attempt new power cycle and increase power-on counter.
	 */
	void
	power_on();

  private:
	/**
	 * Power-cycle the device. This destroys PowerCycle object, which
	 * causes setting of 'power-on' property to false (and thus power-manager
	 * should react to this by powering off the GPS).
	 * After a while, create new PowerCycle.
	 *
	 * This method uses _power_cycle_timer.
	 */
	void
	request_power_cycle();

	/**
	 * Set all data properties to nil.
	 */
	void
	reset_data_properties();

	/**
	 * Set system time. Also set Operating System time. For the latter Xefis executable needs
	 * CAP_SYS_TIME capability, set it with "setcap cap_sys_time+ep xefis".
	 */
	void
	update_clock (xf::nmea::GPSDate const&, xf::nmea::GPSTimeOfDay const&);

  private:
	xf::System*						_system;
	Unique<PowerCycle>				_power_cycle;
	// Used to wait a bit after a failure:
	Unique<QTimer>					_power_cycle_timer;
	bool							_power_cycle_requested		= false;
	bool							_reliable_fix_quality		= false;
	unsigned int					_power_cycle_attempts		= 0;
	xf::SerialPort::Configuration	_serial_port_config;
	bool							_clock_synchronized			= false;
};

#endif

