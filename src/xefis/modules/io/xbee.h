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

#ifndef XEFIS__MODULES__IO__XBEE_H__INCLUDED
#define XEFIS__MODULES__IO__XBEE_H__INCLUDED

// Standard:
#include <cstddef>
#include <map>

// Qt:
#include <QtCore/QSocketNotifier>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/module.h>
#include <xefis/core/v2/property.h>
#include <xefis/core/v2/setting.h>
#include <xefis/utility/v2/actions.h>
#include <xefis/utility/smoother.h>


class XBeeIO: public v2::ModuleIO
{
  public:
	/*
	 * Settings
	 */

	v2::Setting<bool>				debug			{ this, "debug", false };
	v2::Setting<std::string>		device_path		{ this, "device_path" };
	v2::Setting<unsigned int>		baud_rate		{ this, "baud_rate", 9600 };
	v2::Setting<int>				channel			{ this, "channel" };
	v2::Setting<uint16_t>			pan_id			{ this, "pan_id", 0x0000 };
	v2::Setting<uint16_t>			local_address	{ this, "local_address" };
	v2::Setting<uint16_t>			remote_address	{ this, "remote_address" };
	v2::Setting<uint16_t>			power_level		{ this, "power_level" }; // TODO Optional

	/*
	 * Input
	 */

	v2::PropertyIn<std::string>		send			{ this, "/send" };

	/*
	 * Output
	 */

	v2::PropertyOut<bool>			serviceable		{ this, "/serviceable" };
	v2::PropertyOut<std::string>	receive			{ this, "/receive" };
	v2::PropertyOut<int64_t>		input_errors	{ this, "/input-errors" };
	v2::PropertyOut<int64_t>		failures		{ this, "/failures" };
	v2::PropertyOut<int64_t>		cca_failures	{ this, "/clear-channel-failures" };
	v2::PropertyOut<si::Power>		rssi			{ this, "/rssi" };
};


/**
 * Warning: This module is not IO-safe: it uses IO commands in the main thread, which may block.
 *
 * XBee Pro modem. Supports only the API mode 1 (non-escaped chars).  Use XBee firmware that starts in correct API mode
 * by default, or prepare the modem by issuing "ATAP1" AT command and writing config permanently with "ATWR".
 */
class XBee:
	public QObject,
	public v2::Module<XBeeIO>
{
	Q_OBJECT

	static constexpr int		kMaxReadFailureCount		= 10;
	static constexpr int		kMaxWriteFailureCount		= 10;
	static constexpr size_t		kMaxOutputBufferSize		= 256;

	static constexpr uint8_t	kPacketDelimiter			= 0x7e;
	static constexpr uint8_t	kPeriodicPingFrameID		= 0xfd;
	static constexpr uint8_t	kClearChannelFrameID		= 0xfe;

	static constexpr Time		kCommandTimeout				= 200_ms;
	static constexpr Time		kRestartAfter				= 500_ms;
	static constexpr Time		kPeriodicAliveCheck			= 500_ms;
	static constexpr Time		kPeriodicAliveCheckTimeout	= 500_ms;
	static constexpr Time		kClearChannelCheck			= 2_s;
	static constexpr Time		kAfterRestartGraceTime		= 500_ms;
	static constexpr Time		kRSSITimeout				= 1_s;

	// Modem API frame types:
	enum class SendAPI: uint8_t
	{
		TX64			= 0x00,
		TX16			= 0x01,
		ATCommand		= 0x08,
		QueuedATCommand	= 0x09,
	};

	// Response frame types:
	enum class ResponseAPI: uint8_t
	{
		RX64			= 0x80,
		RX16			= 0x81,
		TXStatus		= 0x89,
		ModemStatus		= 0x8a,
		ATResponse		= 0x88,
	};

	enum class SendResult
	{
		Success,
		Retry,
		Failure,
	};

	enum class ConfigurationStep
	{
		Unconfigured,
		SoftwareReset,
		AfterSoftwareReset,
		DisableIOUART,
		ReadHardwareVersion,
		ReadFirmwareVersion,
		ReadSerialNumberH,
		ReadSerialNumberL,
		DisableSleep,
		DisableEncryption,
		DisableACKs,
		SetAssociationSleepPeriod,
		SetAssociationParams,
		SetChannel,
		SetPersonalAreaNetworkID,
		SetDestinationAddressH,
		SetDestinationAddressL,
		SetLocalAddress,
		SetPowerLevel,
		SetCoordinatorMode,
		Configured,
	};

	enum class ATResponseStatus: uint8_t
	{
		OK						= 0,
		ERROR					= 1,
		InvalidCommand			= 2,
		InvalidParameter		= 3,
		StartConfig				= 0xff,
	};

	enum class ModemStatus: uint8_t
	{
		HardwareReset			= 0,
		WatchdogReset			= 1,
		Associated				= 2,
		Disassociated			= 3,
		SynchronizationLost		= 4,
		CoordinatorRealignment	= 5,
		CoordinatorStarted		= 6,
	};

  public:
	// Ctor
	XBee (std::unique_ptr<XBeeIO>, std::string const& instance = {});

	// Dtor
	~XBee();

	void
	process (v2::Cycle const&) override;

  private slots:
	/**
	 * Called whenever there's data ready to be read from the device.
	 */
	void
	read();

	/**
	 * Open device and start processing data.
	 */
	void
	open_device();

	/**
	 * Indicate failure. Try to reopen device, reconfigure and restart
	 * transmission.
	 */
	void
	failure (std::string const& reason);

	/**
	 * Reset to default state and resets properties.
	 */
	void
	reset();

	/**
	 * Try to restart operation after failure is detected.
	 * Also calls reset().
	 */
	void
	restart();

	/**
	 * Ping modem by requesting AI (association indication) info.
	 * AT response should restart periodic-alive-check-timer.
	 */
	void
	periodic_ping();

	/**
	 * Ask modem for clear-channel-assessment failures.
	 */
	void
	clear_channel_check();

	/**
	 * Called when normal pong times out.
	 */
	void
	pong_timeout();

	/**
	 * Called when periodic pong times out.
	 */
	void
	periodic_pong_timeout();

	/**
	 * Called some time after software reset, to give modem time
	 * to initialize itself.
	 */
	void
	continue_after_reset();

	/**
	 * Called when RSSI value times out and becomes invalid.
	 * Sets RSSI indicator to nil.
	 */
	void
	rssi_timeout();

  private:
	/**
	 * Set serial port device options, eg. baud-rate.
	 */
	bool
	set_device_options();

	/**
	 * Configure modem using AT commands.
	 * If at_response is not empty, that is the AT response
	 * from the modem from previous configuration step.
	 */
	void
	configure_modem (uint8_t frame_id = 0x00, ATResponseStatus status = ATResponseStatus::StartConfig, std::string const& at_response = "");

	/**
	 * Return true if modem is configured.
	 */
	bool
	configured() const noexcept;

	/**
	 * Return XBee protocol code for setting up baud rate.
	 */
	static int
	baud_rate_to_xbee_code (int baud_rate);

	/**
	 * Make API frame without escaped characters from given data.
	 */
	std::string
	make_frame (std::string const& data) const;

	/**
	 * Make API request to send data to 64-bit address.
	 * Up to 100 bytes per packet.
	 * Needs wrapping with make_frame().
	 */
	std::string
	make_tx64_command (uint64_t address, std::string const& data) const;

	/**
	 * Same as make_tx64_command(), but uses 16-bit addressing.
	 */
	std::string
	make_tx16_command (uint16_t address, std::string const& data) const;

	/**
	 * Make AT command.
	 * Remember that AT commands take hexadecimal numbers.
	 * Needs wrapping with make_frame().
	 */
	std::string
	make_at_command (std::string const& at_command, uint8_t frame_id = 0x00);

	/**
	 * Send frame.
	 */
	SendResult
	send_frame (std::string const& frame, int& written);

	/**
	 * Should be called if send_frame() returns Retry.
	 * Return true if failure() and restart should be performed.
	 */
	bool
	send_failed_with_retry();

	/**
	 * Split data into packets no bigger than @size bytes.
	 */
	std::vector<std::string>
	packetize (std::string const& data, std::size_t size) const;

	/**
	 * Convert vector<uint8_t> to uint16_t.
	 * Return true if conversion succeeds.
	 */
	bool
	vector_to_uint16 (std::vector<uint8_t> const& vector, uint16_t& result) const;

	/**
	 * Parse input buffer and react to input packets accordingly.
	 */
	void
	process_input();

	/**
	 * Parse out first packet from input buffer. If no packet can be parsed,
	 * discard data up to the nearest packet delimiter, hoping that in future
	 * more data appended will allow parsing out a packet.
	 * On successful parse, return true and remove the packet from input buffer.
	 */
	bool
	process_packet (std::string& input, ResponseAPI& api, std::string& data);

	/**
	 * Parse RX from 64-bit address.
	 */
	void
	process_rx64_frame (std::string const& data);

	/**
	 * Parse RX from 16-bit address.
	 */
	void
	process_rx16_frame (std::string const& data);

	/**
	 * Parse and process modem status packet.
	 */
	void
	process_modem_status_frame (std::string const& data);

	/**
	 * Parse ATResponse packet.
	 */
	void
	process_at_response_frame (std::string const& data);

	/**
	 * Write output data to the output property.
	 */
	void
	write_output_property (std::string const& data);

	/**
	 * Report RSSI. Add it to data smoother and
	 * update the RSSI property.
	 */
	void
	report_rssi (int dbm);

	/**
	 * Start alive-check-timer. If pong() is not called withing
	 * given time limit, failure() is called.
	 */
	void
	ping (Time timeout = 250_ms);

	/**
	 * Call it to indicate that ping() should be cancelled.
	 */
	void
	pong();

	/**
	 * Indicate that modem has returned answer to ping.
	 * Reset alive-check-timer.
	 */
	void
	periodic_pong (ATResponseStatus, std::string const& data);

	/**
	 * Cancel pending periodic ping, if there's any. Disable pinging
	 * until next periodic_ping() call.
	 */
	void
	stop_periodic_ping();

	/**
	 * Called with result of ATEC command.
	 */
	void
	clear_channel_result (ATResponseStatus, std::string const& result);

	/**
	 * Just like log() but adds "DEBUG" message.
	 */
	std::ostream&
	debug() const;

  private:
	Unique<QSocketNotifier>			_notifier;
	int								_device					= 0;
	QTimer*							_restart_timer			= nullptr;
	QTimer*							_pong_timer				= nullptr;
	QTimer*							_periodic_ping_timer	= nullptr;
	QTimer*							_periodic_pong_timer	= nullptr;
	QTimer*							_clear_channel_timer	= nullptr;
	QTimer*							_after_reset_timer		= nullptr;
	QTimer*							_rssi_timer				= nullptr;
	std::string						_serial_number_bin;
	ConfigurationStep				_configuration_step		= ConfigurationStep::Unconfigured;
	int								_read_failure_count		= 0;
	int								_write_failure_count	= 0;
	std::string						_input_buffer;
	std::string						_output_buffer;
	std::string						_last_at_command;
	xf::Smoother<si::Power>			_rssi_smoother			{ 200_ms };
	Time							_last_rssi_time;
	uint8_t							_at_frame_id			{ 0x00 };
	v2::PropChanged<std::string>	_send_changed			{ io.send };
};


inline bool
XBee::configured() const noexcept
{
	return _configuration_step == ConfigurationStep::Configured;
}


inline std::ostream&
XBee::debug() const
{
	return log() << "DEBUG ";
}

#endif
