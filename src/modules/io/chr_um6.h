/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__IO__CHR_UM6_H__INCLUDED
#define XEFIS__MODULES__IO__CHR_UM6_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <memory>

// Qt:
#include <QtCore/QSocketNotifier>

// Lib:
#include <boost/endian/conversion.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <xefis/core/module.h>
#include <xefis/utility/serial_port.h>
#include <xefis/utility/packet_reader.h>
#include <xefis/utility/smoother.h>


/**
 * CH-Robotics UM6 sensor driver.
 * Uses UART for communication.
 */
class CHRUM6:
	public QObject,
	public Xefis::Module
{
	Q_OBJECT

	static constexpr Time RestartDelay			= 200_ms;
	static constexpr Time AliveCheckInterval	= 500_ms;
	static constexpr Time StatusCheckInterval	= 200_ms;
	static constexpr Time InitializationDelay	= 3_s;

	// Register write or read?
	enum class Operation
	{
		Write,
		Read,
		Command,
	};

	// UM6 registers that can be read or written:
	enum class Address
	{
		// Configuration registers:
		Communication		= 0x00,
		MiscConfig			= 0x01,
		MagRefX				= 0x02,
		MagRefY				= 0x03,
		MagRefZ				= 0x04,
		AccelRefX			= 0x05,
		AccelRefY			= 0x06,
		AccelRefZ			= 0x07,
		EKFMagVariance		= 0x08,
		EKFAccelVariance	= 0x09,
		EKFProcessVariance	= 0x0a,
		GyroBiasXY			= 0x0b,
		GyroBiasZ			= 0x0c,
		AccelBiasXY			= 0x0d,
		AccelBiasZ			= 0x0e,
		MagBiasXY			= 0x0f,
		MagBiasZ			= 0x10,
		AccelCal00			= 0x11,
		AccelCal01			= 0x12,
		AccelCal02			= 0x13,
		AccelCal10			= 0x14,
		AccelCal11			= 0x15,
		AccelCal12			= 0x16,
		AccelCal20			= 0x17,
		AccelCal21			= 0x18,
		AccelCal22			= 0x19,
		GyroCal00			= 0x1a,
		GyroCal01			= 0x1b,
		GyroCal02			= 0x1c,
		GyroCal10			= 0x1d,
		GyroCal11			= 0x1e,
		GyroCal12			= 0x1f,
		GyroCal20			= 0x20,
		GyroCal21			= 0x21,
		GyroCal22			= 0x22,
		MagCal00			= 0x23,
		MagCal01			= 0x24,
		MagCal02			= 0x25,
		MagCal10			= 0x26,
		MagCal11			= 0x27,
		MagCal12			= 0x28,
		MagCal20			= 0x29,
		MagCal21			= 0x2a,
		MagCal22			= 0x2b,
		GyroXBias0			= 0x2c,
		GyroXBias1			= 0x2d,
		GyroXBias2			= 0x2e,
		GyroXBias3			= 0x2f,
		GyroYBias0			= 0x30,
		GyroYBias1			= 0x31,
		GyroYBias2			= 0x32,
		GyroYBias3			= 0x33,
		GyroZBias0			= 0x34,
		GyroZBias1			= 0x35,
		GyroZBias2			= 0x36,
		GyroZBias3			= 0x37,
		GPSHomeLat			= 0x38,
		GPSHomeLon			= 0x39,
		GPSHomeAltitude		= 0x40,
		// Data registers:
		Status				= 0x55,
		GyroRawXY			= 0x56,
		GyroRawZ			= 0x57,
		AccelRawXY			= 0x58,
		AccelRawZ			= 0x59,
		MagRawXY			= 0x5a,
		MagRawZ				= 0x5b,
		GyroProcXY			= 0x5c,
		GyroProcZ			= 0x5d,
		AccelProcXY			= 0x5e,
		AccelProcZ			= 0x5f,
		MagProcXY			= 0x60,
		MagProcZ			= 0x61,
		EulerPhiTheta		= 0x62,
		EulerPsi			= 0x63,
		QuatAB				= 0x64,
		QuatCD				= 0x65,
		ErrorCov00			= 0x66,
		ErrorCov01			= 0x67,
		ErrorCov02			= 0x68,
		ErrorCov03			= 0x69,
		ErrorCov10			= 0x6a,
		ErrorCov11			= 0x6b,
		ErrorCov12			= 0x6c,
		ErrorCov13			= 0x6d,
		ErrorCov20			= 0x6e,
		ErrorCov21			= 0x6f,
		ErrorCov22			= 0x70,
		ErrorCov23			= 0x71,
		ErrorCov30			= 0x72,
		ErrorCov31			= 0x73,
		ErrorCov32			= 0x74,
		ErrorCov33			= 0x75,
		Temperature			= 0x76,
		GPSLongitude		= 0x77,
		GPSLatitude			= 0x78,
		GPSAltitude			= 0x79,
		GPSPositionN		= 0x7a,
		GPSPositionE		= 0x7b,
		GPSPositionH		= 0x7c,
		GPSCourseSpeed		= 0x7d,
		GPSSatSummary		= 0x7e,
		GPSSat12			= 0x7f,
		GPSSat34			= 0x80,
		GPSSat56			= 0x81,
		GPSSat78			= 0x82,
		GPSSat9A			= 0x83,
		GPSSatBC			= 0x84,
		// Command registers:
		GetFWVersion		= 0xaa,
		FlashCommit			= 0xab,
		ZeroGyros			= 0xac,
		ResetEKF			= 0xad,
		GetData				= 0xae,
		SetAccelRef			= 0xaf,
		SetMagRef			= 0xb0,
		ResetToFactory		= 0xb1,
		GPSSetHomePosition	= 0xb3,
		BadChecksum			= 0xfd,
		UnknownAddress		= 0xfe,
		InvalidBatchSize	= 0xff,
	};

	// Communication register bits:
	enum class CommunicationRegister: uint32_t
	{
		BroadcastRateLSB	= 1u << 0,	// 8-bit wide
		BaudRateLSB			= 1u << 8,	// 3-bit wide
		GPSBaudRateLSB		= 1u << 11,	// 3-bit wide
		SAT					= 1u << 15,
		SUM					= 1u << 16,
		VEL					= 1u << 17,
		REL					= 1u << 18,
		POS					= 1u << 19,
		TMP					= 1u << 20,
		COV					= 1u << 21,
		EU					= 1u << 22,
		QT					= 1u << 23,
		MP					= 1u << 24,
		AP					= 1u << 25,
		GP					= 1u << 26,
		MR					= 1u << 27,
		AR					= 1u << 28,
		GR					= 1u << 29,
		BEN					= 1u << 30,
	};

	// MiscConfig register bits:
	enum class MiscConfigRegister: uint32_t
	{
		MUE					= 1u << 31,
		AUE					= 1u << 30,
		CAL					= 1u << 29,
		QUAT				= 1u << 28,
		PPS					= 1u << 27,
	};

	// Status register bits:
	enum class StatusRegister: uint32_t
	{
		SelfTested			= 1u << 0,
		MagDel				= 1u << 13,
		AccelDel			= 1u << 14,
		GyroDel				= 1u << 15,
		EKFDivergent		= 1u << 16,
		BusMagError			= 1u << 17,
		BusAccelError		= 1u << 18,
		BusGyroError		= 1u << 19,
		SelfTestMagZFail	= 1u << 20,
		SelfTestMagYFail	= 1u << 21,
		SelfTestMagXFail	= 1u << 22,
		SelfTestAccelZFail	= 1u << 23,
		SelfTestAccelYFail	= 1u << 24,
		SelfTestAccelXFail	= 1u << 25,
		SelfTestGyroZFail	= 1u << 26,
		SelfTestGyroYFail	= 1u << 27,
		SelfTestGyroXFail	= 1u << 28,
		GyroInitFail		= 1u << 29,
		AccelInitFail		= 1u << 30,
		MagInitFail			= 1u << 31,
	};

	enum class Stage
	{
		Initialize,
		Run,
	};

  public:
	// Ctor
	CHRUM6 (Xefis::ModuleManager* module_manager, QDomElement const& config);

  private slots:
	/**
	 * Open device and start processing data.
	 */
	void
	open_device();

	/**
	 * Called when device doesn't respond for a while.
	 */
	void
	alive_check_failed();

	/**
	 * Called when initialization takes too long to complete.
	 */
	void
	initialization_timeout();

	/**
	 * Try to restart operation after failure is detected.
	 */
	void
	restart();

	/**
	 * Check device status: read fail bits, check temperature, etc.
	 */
	void
	status_check();

  private:
	/**
	 * Start setting up the device. It's asynchronous, and will
	 * issue several commands. When it's finished, initialization_complete()
	 * will be called.
	 */
	void
	initialize();

	/**
	 * Goes to next initialization step, after 'command complete' packet
	 * is received from the device.
	 */
	void
	next_initialization_step();

	/**
	 * Repeat last initialization step.
	 */
	void
	repeat_initialization_step();

	/**
	 * Called when initialization is complete.
	 */
	void
	initialization_complete();

	/**
	 * Reset buffer and state. A must after a failure of some sort.
	 */
	void
	reset();

	/**
	 * Indicate failure. Try to reopen device, perhaps
	 * with other baud-rate setting.
	 */
	void
	failure (std::string const& message);

	/**
	 * Called when data is ready to read from serial port.
	 */
	void
	serial_ready();

	/**
	 * Called when failure is detected on serial port.
	 */
	void
	serial_failure();

	/**
	 * Issue command to UM6.
	 */
	void
	issue_command (Address);

	/**
	 * Write to UM6 register.
	 */
	void
	write_register (Address, uint32_t data);

	/**
	 * Write to UM6 32-bit float.
	 */
	void
	write_register (Address, float data);

	/**
	 * Read from UM6 32-bit register.
	 * process_data() will be called with appropriate address.
	 */
	void
	read_register (Address);

	/**
	 * Sends packet through serial port.
	 */
	void
	send_packet (Blob const&);

	/**
	 * Create packet for UM6 for single (non-batch) operation.
	 */
	Blob
	make_packet (Address, Operation, uint32_t data = 0);

	/**
	 * Parse incoming packet from _packet_reader.
	 * Call various processing functions.
	 */
	std::size_t
	parse_packet();

	/**
	 * Called when COMMAND_COMPLETE is sent from the UM6.
	 */
	void
	process_packet (Address, bool failed, bool has_data, uint32_t data);

	/**
	 * Get upper 16-bit signed integer from UM6 register.
	 * Convert to native order.
	 */
	static int16_t
	upper16 (uint32_t data) noexcept;

	/**
	 * Get lower 16-bit signed integer from UM6 register.
	 * Convert to native order.
	 */
	static int16_t
	lower16 (uint32_t data) noexcept;

	/**
	 * Convert uint32_t to float.
	 */
	static float
	to_float (uint32_t data) noexcept;

	/**
	 * For given sampling rate return UM6 setting ready
	 * to be written to the Communication register.
	 */
	static uint32_t
	sample_rate_setting (Frequency) noexcept;

	/**
	 * Return bit value used in communication register
	 * for baud rate. Bits are not shifted to the right position.
	 */
	static uint32_t
	bits_for_baud_rate (std::string const& baud_rate);

	/**
	 * Checks status bits and sets status/serviceable properties.
	 */
	void
	status_verify (uint32_t data);

  private:
	Unique<QTimer>				_restart_timer;
	Unique<QTimer>				_alive_check_timer;
	Unique<QTimer>				_status_check_timer;
	Unique<QTimer>				_initialization_timer;
	Xefis::SerialPort			_serial_port;
	Xefis::PacketReader			_packet_reader;
	int							_failure_count			= 0;
	float						_ekf_process_variance	= 0.5f;
	Frequency					_sample_rate			= 20_Hz;
	std::string					_baud_rate				= "115200";
	bool						_signal_data_updated	= false;
	Stage						_stage					= Stage::Initialize;
	int							_initialization_step	= 0;
	// Backup gyro bias values:
	Optional<uint32_t>			_gyro_bias_xy;
	Optional<uint32_t>			_gyro_bias_z;

	Xefis::PropertyBoolean		_serviceable;
	Xefis::PropertyBoolean		_caution;
	Xefis::PropertyInteger		_failures;
	Xefis::PropertyTemperature	_internal_temperature;
	Xefis::PropertyAngle		_orientation_pitch;
	Xefis::PropertyAngle		_orientation_roll;
	Xefis::PropertyAngle		_orientation_magnetic_heading;
	// In gravities:
	Xefis::PropertyFloat		_acceleration_x;
	Xefis::PropertyFloat		_acceleration_y;
	Xefis::PropertyFloat		_acceleration_z;
	// In deg/s:
	Xefis::PropertyAngle		_rotation_x;
	Xefis::PropertyAngle		_rotation_y;
	Xefis::PropertyAngle		_rotation_z;
	// In... what?
	Xefis::PropertyFloat		_magnetic_x;
	Xefis::PropertyFloat		_magnetic_y;
	Xefis::PropertyFloat		_magnetic_z;
};


inline int16_t
CHRUM6::upper16 (uint32_t data) noexcept
{
	int16_t d = (data >> 16) & 0xffff;
	boost::endian::little_to_native (d);
	return d;
}


inline int16_t
CHRUM6::lower16 (uint32_t data) noexcept
{
	int16_t d = (data >> 0) & 0xffff;
	boost::endian::little_to_native (d);
	return d;
}


inline float
CHRUM6::to_float (uint32_t data) noexcept
{
	union { uint32_t i; float f; } u = { data };
	return u.f;
}

#endif

