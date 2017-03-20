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

#ifndef XEFIS__SUPPORT__DEVICES__CHR_UM6_H__INCLUDED
#define XEFIS__SUPPORT__DEVICES__CHR_UM6_H__INCLUDED

// Standard:
#include <cstddef>
#include <functional>
#include <queue>

// Qt:
#include <QtCore/QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/bus/serial_port.h>
#include <xefis/utility/packet_reader.h>
#include <xefis/utility/logger.h>
#include <xefis/utility/time_helper.h>


namespace xf {

/**
 * Encapsulates protocol used by CHR-UM6 sensor.
 * Uses provided SerialPort to communicate with UM6.
 * The port must be opened before using this API.
 */
class CHRUM6
{
  public:
	// Fwd
	class Request;
	class Command;
	class Read;
	class Write;

  public:
	// UM6 registers that can be read or written:
	enum class ConfigurationAddress
	{
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
	};

	// UM6 registers that are read only:
	enum class DataAddress
	{
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
	};

	// UM6 command registers:
	enum class CommandAddress
	{
		GetFWVersion		= 0xaa,
		FlashCommit			= 0xab,
		ZeroGyros			= 0xac,
		ResetEKF			= 0xad,
		GetData				= 0xae,
		SetAccelRef			= 0xaf,
		SetMagRef			= 0xb0,
		ResetToFactory		= 0xb1,
		GPSSetHomePosition	= 0xb3,
	};

	// UM6 special IDs
	enum class ProtocolError
	{
		None				= 0x00,		// Non-UM6
		Timeout				= 0x01,		// Non-UM6
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

	typedef std::function<void (Command)>	CommandCallback;
	typedef std::function<void (Read)>		ReadCallback;
	typedef std::function<void (Write)>		WriteCallback;

  private:
	/**
	 * Common data object.
	 */
	class CommandData
	{
		friend class CHRUM6;
		friend class Request;
		friend class Command;
		friend class Read;
		friend class Write;

		uint32_t		address			= -1u;
		Time			start_timestamp;
		Time			finish_timestamp;
		bool			finished		= false;
		bool			success			= false;
		ProtocolError	protocol_error	= ProtocolError::None;
		Blob			packet_data;
		uint32_t		value			= 0;
		CommandCallback	command_callback;
		uint32_t		retries			= 0;
	};

	/**
	 * Data object used by read operations.
	 */
	class ReadData: public CommandData
	{
		friend class Read;

		ReadCallback	read_callback;
	};

	/**
	 * Data object used by write operations.
	 */
	class WriteData: public CommandData
	{
		friend class Write;

		WriteCallback	write_callback;
	};

  public:
	/**
	 * Abstract command operation.
	 */
	class Request
	{
		friend class CHRUM6;

	  protected:
		/**
		 * Setup some internal data.
		 */
		void
		setup (uint32_t generic_address, bool write_operation, uint32_t data);

	  public:
		/**
		 * Return register address.
		 */
		uint32_t
		address() const noexcept;

		/**
		 * Return point in time at which command
		 * was executed.
		 */
		Time
		timestamp() const noexcept;

		/**
		 * Return command's duration until it has
		 * finished. If not finished yet, return
		 * now - timestamp().
		 */
		Time
		duration() const noexcept;

		/**
		 * Return true after command finishes,
		 * regardless of success or failure.
		 */
		bool
		finished() const noexcept;

		/**
		 * Return true if command succeeded.
		 */
		bool
		success() const noexcept;

		/**
		 * Return protocol error indicator if !success().
		 * This might be None, meaning there was no protocol error,
		 * but the command simply failed to execute.
		 */
		ProtocolError
		protocol_error() const noexcept;

		/**
		 * Return string information about error.
		 */
		std::string
		protocol_error_description() const;

		/**
		 * Return packet data to send to UM6.
		 */
		Blob const&
		packet_data() const noexcept;

		/**
		 * Return packet name based on address.
		 */
		const char*
		name() const noexcept;

		/**
		 * How many times this command was repeated (due to ProtocolError).
		 */
		uint32_t
		retries() const noexcept;

	  protected:
		/**
		 * Return the shared data object.
		 */
		virtual CommandData*
		data() const noexcept = 0;

		/**
		 * Call provided callback.
		 */
		virtual void
		make_callback() = 0;
	};

	/**
	 * Represents command send to the sensor.
	 * Result is provided asynchronously.
	 */
	class Command:
		public Request,
		public Shared<CommandData>
	{
		friend class CHRUM6;

	  private:
		// Ctor
		Command (CommandAddress, CommandCallback = nullptr);

	  public:
		/**
		 * Return value returned by the command.
		 */
		uint32_t
		value() const noexcept;

		/**
		 * Get the firmware version. Applies to GetFWVersion command.
		 */
		std::string
		firmware_version() const;

	  protected:
		CommandData*
		data() const noexcept;

		void
		make_callback();
	};

	/**
	 * Asynchronous read operation.
	 * When it's ready, result() will contain value of given
	 * register.
	 */
	class Read:
		public Request,
		public Shared<ReadData>
	{
		friend class CHRUM6;

	  private:
		// Ctor
		Read (ConfigurationAddress, ReadCallback = nullptr);

		// Ctor
		Read (DataAddress, ReadCallback = nullptr);

	  public:
		/**
		 * Return raw 32-bit register value.
		 */
		uint32_t
		value() const noexcept;

		/**
		 * Get upper 16-bit signed integer from UM6 register.
		 * Convert to native order.
		 */
		int16_t
		value_upper16() const noexcept;

		/**
		 * Get lower 16-bit signed integer from UM6 register.
		 * Convert to native order.
		 */
		int16_t
		value_lower16() const noexcept;

		/**
		 * Convert register value to float.
		 */
		float
		value_as_float() const noexcept;

	  protected:
		CommandData*
		data() const noexcept;

		void
		make_callback();
	};

	/**
	 * Asynchronous write operation.
	 */
	class Write:
		public Request,
		public Shared<WriteData>
	{
		friend class CHRUM6;

	  private:
		// Ctor
		Write (ConfigurationAddress, uint32_t value, WriteCallback = nullptr);

	  protected:
		CommandData*
		data() const noexcept;

		void
		make_callback();
	};

  public:
	// Ctor
	explicit
	CHRUM6 (SerialPort* serial_port);

	/**
	 * Set logger.
	 */
	void
	set_logger (Logger const& logger);

	/**
	 * Set callback indicating serial port failure.
	 */
	void
	set_communication_failure_callback (std::function<void()>);

	/**
	 * Set callback indicating that serial port responds (alive-check).
	 */
	void
	set_alive_check_callback (std::function<void()>);

	/**
	 * Set callback for not-requested incoming asynchronous messages.
	 */
	void
	set_incoming_messages_callback (std::function<void (Read)>);

	/**
	 * Set automatic command retry when BadChecksum is received.
	 */
	void
	set_auto_retry (bool enable);

	/**
	 * Read from a config register.
	 */
	Read
	read (ConfigurationAddress, ReadCallback = nullptr);

	/**
	 * Read from a data register.
	 */
	Read
	read (DataAddress, ReadCallback = nullptr);

	/**
	 * Write uint32_t to a config register.
	 */
	Write
	write (ConfigurationAddress, uint32_t value, WriteCallback = nullptr);

	/**
	 * Write float to a config register.
	 */
	Write
	write (ConfigurationAddress, float value, WriteCallback = nullptr);

	/**
	 * Issue a command.
	 */
	Command
	command (CommandAddress, CommandCallback = nullptr);

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
	bits_for_baud_rate (unsigned int baud_rate);

  private:
	// Serial port callback
	void
	serial_ready();

	// Serial port callback
	void
	serial_failure();

	/**
	 * Process next request from queue if possible.
	 */
	void
	process_queue();

	/**
	 * Create packet for UM6 for single (non-batch) operation.
	 */
	static Blob
	make_packet (uint32_t address, bool write, uint32_t data = 0);

	/**
	 * Parse incoming packet from _packet_reader.
	 * Call various processing functions.
	 */
	std::size_t
	parse_packet();

	/**
	 * Sends packet through serial port.
	 */
	void
	send_packet (Blob const&);

	/**
	 * Called when COMMAND_COMPLETE is sent from the UM6.
	 */
	void
	process_packet (uint32_t address, bool failed, bool has_data, uint32_t data);

	/**
	 * Return packet name.
	 */
	static const char*
	packet_name (uint32_t address) noexcept;

  private:
	SerialPort*					_serial_port	= nullptr;
	Unique<PacketReader>		_packet_reader;
	std::function<void()>		_communication_failure_callback;
	std::function<void()>		_alive_check_callback;
	std::function<void (Read)>	_incoming_messages_callback;
	bool						_auto_retry		= false;
	std::queue<Unique<Request>>	_requests;
	Unique<Request>				_current_req;
	Logger						_logger;
};


inline void
CHRUM6::Request::setup (uint32_t generic_address, bool write_operation, uint32_t value)
{
	data()->address = generic_address;
	data()->packet_data = CHRUM6::make_packet (generic_address, write_operation, value);
}


inline uint32_t
CHRUM6::Request::address() const noexcept
{
	return data()->address;
}


inline Time
CHRUM6::Request::timestamp() const noexcept
{
	return data()->start_timestamp;
}


inline Time
CHRUM6::Request::duration() const noexcept
{
	if (finished())
		return data()->finish_timestamp - data()->start_timestamp;
	else
		return TimeHelper::now() - data()->start_timestamp;
}


inline bool
CHRUM6::Request::finished() const noexcept
{
	return data()->finished;
}


inline bool
CHRUM6::Request::success() const noexcept
{
	return data()->success;
}


inline CHRUM6::ProtocolError
CHRUM6::Request::protocol_error() const noexcept
{
	return data()->protocol_error;
}


inline Blob const&
CHRUM6::Request::packet_data() const noexcept
{
	return data()->packet_data;
}


inline const char*
CHRUM6::Request::name() const noexcept
{
	return CHRUM6::packet_name (data()->address);
}


inline uint32_t
CHRUM6::Request::retries() const noexcept
{
	return data()->retries;
}


inline
CHRUM6::Command::Command (CommandAddress address, CommandCallback callback):
	Shared<CommandData> (new CommandData())
{
	setup (static_cast<uint32_t> (address), false, 0);
	get()->command_callback = callback;
}


inline uint32_t
CHRUM6::Command::value() const noexcept
{
	return get()->value;
}


inline CHRUM6::CommandData*
CHRUM6::Command::data() const noexcept
{
	return get();
}


inline void
CHRUM6::Command::make_callback()
{
	if (get()->command_callback)
		get()->command_callback (*this);
}


inline
CHRUM6::Read::Read (ConfigurationAddress address, ReadCallback callback):
	Shared<ReadData> (new ReadData)
{
	setup (static_cast<uint32_t> (address), false, 0);
	get()->read_callback = callback;
}


inline
CHRUM6::Read::Read (DataAddress address, ReadCallback callback):
	Shared<ReadData> (new ReadData)
{
	setup (static_cast<uint32_t> (address), false, 0);
	get()->read_callback = callback;
}


inline uint32_t
CHRUM6::Read::value() const noexcept
{
	return get()->value;
}


inline float
CHRUM6::Read::value_as_float() const noexcept
{
	union { uint32_t i; float f; } u = { value() };
	return u.f;
}


inline CHRUM6::CommandData*
CHRUM6::Read::data() const noexcept
{
	return get();
}


inline void
CHRUM6::Read::make_callback()
{
	if (get()->read_callback)
		get()->read_callback (*this);
}


inline
CHRUM6::Write::Write (ConfigurationAddress address, uint32_t value, WriteCallback callback):
	Shared<WriteData> (new WriteData())
{
	setup (static_cast<uint32_t> (address), true, value);
	get()->value = value;
	get()->write_callback = callback;
}


inline CHRUM6::CommandData*
CHRUM6::Write::data() const noexcept
{
	return get();
}


inline void
CHRUM6::Write::make_callback()
{
	if (get()->write_callback)
		get()->write_callback (*this);
}


inline void
CHRUM6::set_communication_failure_callback (std::function<void()> callback)
{
	_communication_failure_callback = callback;
}


inline void
CHRUM6::set_alive_check_callback (std::function<void()> callback)
{
	_alive_check_callback = callback;
}


inline void
CHRUM6::set_incoming_messages_callback (std::function<void (Read)> callback)
{
	_incoming_messages_callback = callback;
}


inline void
CHRUM6::set_auto_retry (bool enable)
{
	_auto_retry = enable;
}

} // namespace xf

#endif

