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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/module_socket.h>
#include <xefis/support/sockets/socket_changed.h>
#include <xefis/utility/smoother.h>

// Neutrino:
#include <neutrino/logger.h>

// Qt:
#include <QtCore/QSocketNotifier>

// Standard:
#include <cstddef>
#include <map>


namespace si = neutrino::si;
using namespace neutrino::si::literals;


class XBeeIO: public xf::Module
{
  public:
	/*
	 * Settings
	 */

	xf::Setting<bool>				debug			{ this, "debug", false };
	xf::Setting<std::string>		device_path		{ this, "device_path" };
	xf::Setting<unsigned int>		baud_rate		{ this, "baud_rate", 9600 };
	xf::Setting<int>				channel			{ this, "channel" };
	xf::Setting<uint16_t>			pan_id			{ this, "pan_id", 0x0000 };
	xf::Setting<uint16_t>			local_address	{ this, "local_address" };
	xf::Setting<uint16_t>			remote_address	{ this, "remote_address" };
	xf::Setting<uint16_t>			power_level		{ this, "power_level" }; // TODO Optional

	/*
	 * Input
	 */

	xf::ModuleIn<std::string>		send			{ this, "send" };

	/*
	 * Output
	 */

	xf::ModuleOut<bool>				serviceable		{ this, "serviceable" };
	xf::ModuleOut<std::string>		receive			{ this, "receive" };
	xf::ModuleOut<int64_t>			input_errors	{ this, "input-errors" };
	xf::ModuleOut<int64_t>			failures		{ this, "failures" };
	xf::ModuleOut<int64_t>			cca_failures	{ this, "clear-channel-failures" };
	xf::ModuleOut<si::Power>		rssi			{ this, "rssi" };

  public:
	using xf::Module::Module;
};


/**
 * Warning: This module is not IO-safe: it uses IO commands in the main thread, which may block.
 *
 * XBee Pro modem. Supports only the API mode 1 (non-escaped chars).  Use XBee firmware that starts in correct API mode
 * by default, or prepare the modem by issuing "ATAP1" AT command and writing config permanently with "ATWR".
 */
class XBee:
	public QObject,
	public XBeeIO
{
	Q_OBJECT

	static constexpr char		kLoggerScope[]				= "mod::XBee";

	static constexpr int		kMaxReadFailureCount		= 10;
	static constexpr int		kMaxWriteFailureCount		= 10;
	static constexpr size_t		kMaxOutputBufferSize		= 256;

	static constexpr uint8_t	kPacketDelimiter			= 0x7e;
	static constexpr uint8_t	kPeriodicPingFrameID		= 0xfd;
	static constexpr uint8_t	kClearChannelFrameID		= 0xfe;

	static constexpr si::Time	kCommandTimeout				= 200_ms;
	static constexpr si::Time	kRestartAfter				= 500_ms;
	static constexpr si::Time	kPeriodicAliveCheck			= 500_ms;
	static constexpr si::Time	kPeriodicAliveCheckTimeout	= 500_ms;
	static constexpr si::Time	kClearChannelCheck			= 2_s;
	static constexpr si::Time	kAfterRestartGraceTime		= 500_ms;
	static constexpr si::Time	kRSSITimeout				= 1_s;

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
	XBee (xf::Logger const&, std::string_view const& instance = {});

	// Dtor
	~XBee();

	void
	process (xf::Cycle const&) override;

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
	failure (std::string_view const& reason);

	/**
	 * Reset to default state and resets sockets.
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
	configure_modem (uint8_t frame_id = 0x00, ATResponseStatus status = ATResponseStatus::StartConfig, std::string_view const& at_response = "");

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
	make_frame (std::string_view const& data) const;

	/**
	 * Make API request to send data to 64-bit address.
	 * Up to 100 bytes per packet.
	 * Needs wrapping with make_frame().
	 */
	std::string
	make_tx64_command (uint64_t address, std::string_view const& data) const;

	/**
	 * Same as make_tx64_command(), but uses 16-bit addressing.
	 */
	std::string
	make_tx16_command (uint16_t address, std::string_view const& data) const;

	/**
	 * Make AT command.
	 * Remember that AT commands take hexadecimal numbers.
	 * Needs wrapping with make_frame().
	 */
	std::string
	make_at_command (std::string_view const& at_command, uint8_t frame_id = 0x00);

	/**
	 * Send frame.
	 */
	SendResult
	send_frame (std::string_view const& frame, int& written);

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
	packetize (std::string_view const& data, std::size_t size) const;

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
	process_rx64_frame (std::string_view const& data);

	/**
	 * Parse RX from 16-bit address.
	 */
	void
	process_rx16_frame (std::string_view const& data);

	/**
	 * Parse and process modem status packet.
	 */
	void
	process_modem_status_frame (std::string_view const& data);

	/**
	 * Parse ATResponse packet.
	 */
	void
	process_at_response_frame (std::string_view const& data);

	/**
	 * Write output data to the output socket.
	 */
	void
	write_output_socket (std::string_view const& data);

	/**
	 * Report RSSI. Add it to data smoother and
	 * update the RSSI socket.
	 */
	void
	report_rssi (int dbm);

	/**
	 * Start alive-check-timer. If pong() is not called withing
	 * given time limit, failure() is called.
	 */
	void
	ping (si::Time timeout = 250_ms);

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
	periodic_pong (ATResponseStatus, std::string_view const& data);

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
	clear_channel_result (ATResponseStatus, std::string_view const& result);

	/**
	 * A logger that adds "DEBUG" message.
	 */
	xf::LogBlock
	debug() const;

  private:
	XBeeIO&								_io						{ *this };
	xf::Logger							_logger;
	std::unique_ptr<QSocketNotifier>	_notifier;
	int									_device					{ 0 };
	QTimer*								_restart_timer			{ nullptr };
	QTimer*								_pong_timer				{ nullptr };
	QTimer*								_periodic_ping_timer	{ nullptr };
	QTimer*								_periodic_pong_timer	{ nullptr };
	QTimer*								_clear_channel_timer	{ nullptr };
	QTimer*								_after_reset_timer		{ nullptr };
	QTimer*								_rssi_timer				{ nullptr };
	std::string							_serial_number_bin;
	ConfigurationStep					_configuration_step		{ ConfigurationStep::Unconfigured };
	int									_read_failure_count		{ 0 };
	int									_write_failure_count	{ 0 };
	std::string							_input_buffer;
	std::string							_output_buffer;
	std::string							_last_at_command;
	xf::Smoother<si::Power>				_rssi_smoother			{ 200_ms };
	si::Time							_last_rssi_time;
	uint8_t								_at_frame_id			{ 0x00 };
	xf::SocketChanged					_send_changed			{ _io.send };
};


inline bool
XBee::configured() const noexcept
{
	return _configuration_step == ConfigurationStep::Configured;
}


inline xf::LogBlock
XBee::debug() const
{
	return _logger << "DEBUG ";
}

#endif
