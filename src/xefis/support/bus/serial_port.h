/* vim:ts=4
 *
 * Copyleft 2008…2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__BUS__SERIAL_PORT_H__INCLUDED
#define XEFIS__SUPPORT__BUS__SERIAL_PORT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <vector>
#include <functional>

// Qt:
#include <QtCore/QSocketNotifier>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/noncopyable.h>
#include <xefis/utility/logger.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/owner_token.h>


namespace xf {

class SerialPort:
	public QObject,
	private Noncopyable
{
	Q_OBJECT

  public:
	// Parity bit:
	enum class Parity
	{
		None,
		Odd,
		Even,
	};

	/**
	 * Thrown when write operation can't be performed
	 * (eg. on closed device).
	 */
	class WriteException: public Exception
	{
	  public:
		using Exception::Exception;
	};

	/**
	 * Represents serial port configuration.
	 */
	class Configuration
	{
		friend class SerialPort;

	  public:
		/**
		 * Get path to the device.
		 */
		std::string
		device_path() const noexcept;

		/**
		 * Set path to the device.
		 */
		void
		set_device_path (std::string const& device_path);

		/**
		 * Return current baud rate.
		 */
		unsigned int
		baud_rate() const noexcept;

		/**
		 * Set device options.
		 * Baud rate is parsed from baud_rate.
		 */
		void
		set_baud_rate (unsigned int baud_rate);

		/**
		 * Set data bits. Possible values are: 5, 6, 7, 8.
		 * Default: 8.
		 */
		void
		set_data_bits (unsigned int data_bits);

		/**
		 * Set parity bit: even or odd.
		 */
		void
		set_parity_bit (Parity);

		/**
		 * Set stop bits: possible values are: 1, 2.
		 * Default: 1
		 */
		void
		set_stop_bits (unsigned int stop_bits);

		/**
		 * Enable hardware flow control.
		 */
		void
		set_hardware_flow_control (bool enabled);

	  private:
		std::string		_device_path;
		unsigned int	_baud_rate;
		unsigned int	_data_bits	= 8;
		Parity			_parity		= Parity::None;
		unsigned int	_stop_bits	= 1;
		bool			_rtscts		= false;
	};

	typedef std::function<void()>	DataReadyCallback;
	typedef std::function<void()>	FailureCallback;

  public:
	/**
	 * Ctor
	 * @data_ready is called when there's something to read.
	 * @failure is called when failure is detected.
	 */
	explicit
	SerialPort (DataReadyCallback data_ready = nullptr, FailureCallback failure = nullptr);

	// Move ctor
	SerialPort (SerialPort&&);

	// Dtor
	~SerialPort();

	// Move operator
	SerialPort&
	operator= (SerialPort&&);

	/**
	 * Set data ready callback.
	 */
	void
	set_data_ready_callback (DataReadyCallback);

	/**
	 * Set failure callback.
	 */
	void
	set_failure_callback (FailureCallback);

	/**
	 * Set serial port configuration.
	 */
	void
	set_configuration (Configuration const&);

	/**
	 * Return configuration.
	 */
	Configuration const&
	configuration() const noexcept;

	/**
	 * Set logger.
	 */
	void
	set_logger (Logger const& logger);

	/**
	 * Set number of read failures at which
	 * connection will be closed. Default: 0.
	 */
	void
	set_max_read_failures (unsigned int number);

	/**
	 * Set number of write failures at which
	 * connection will be closed. Default: 0.
	 */
	void
	set_max_write_failures (unsigned int number);

	/**
	 * Return true if last open() succeeded.
	 */
	bool
	good() const noexcept;

	/**
	 * Return last error message.
	 */
	std::string const&
	error() const noexcept;

	/**
	 * Notify about a failure. Set error() to message.
	 * Call failure() signal.
	 */
	void
	notify_failure (std::string const& message);

	/**
	 * Access the input buffer. You should remove the data
	 * that have been processed from the beginning of the buffer.
	 */
	Blob&
	input_buffer() noexcept;

	/**
	 * Write data to the device. Data is written asynchronously,
	 * and it's possible that not all requested data is written
	 * when function returns.
	 * \throws	WriteException
	 *			When device is closed.
	 */
	void
	write (Blob const& data);

	/**
	 * Write string to the device. Same conditions apply as for
	 * write (Blob).
	 * \throws	WriteException
	 *			When device is closed.
	 */
	void
	write (std::string const& data);

	/**
	 * Request writing output-buffered data to the device (asynchronously).
	 */
	void
	flush_async();

	/**
	 * Flush the output buffer to the device - synchronously.
	 */
	void
	flush();

	/**
	 * Return true if output buffer is empty
	 * (all data have been written to the device).
	 */
	bool
	flushed() const noexcept;

  public slots:
	/**
	 * Try to open device. Return true if opening succeeded.
	 * If false is returned, error message can be retrieved
	 * using error() method.
	 */
	bool
	open();

	/**
	 * Close device.
	 */
	void
	close();

  public:
	/**
	 * Return termios baudrate constant from given baudrate integer.
	 */
	static int
	termios_baud_rate (unsigned int baud_rate);

	/**
	 * Return termios baudrate constant from given baudrate string.
	 */
	static int
	termios_baud_rate (std::string const& baud_rate);

  private slots:
	/**
	 * Read data from a device to buffer.
	 */
	void
	read();

  private:
	/**
	 * Set baud rate and other parameters of the serial port.
	 */
	bool
	set_device_options();

	/**
	 * Return logger object.
	 */
	Logger const&
	log() const;

	/**
	 * Return prefix for logger messages.
	 */
	std::string
	log_prefix() const;

  private:
	xf::OwnerToken			_owned;
	Logger const*			_logger						= nullptr;
	Logger					_internal_logger;
	Configuration			_configuration;
	DataReadyCallback		_data_ready;
	FailureCallback			_failure;
	Unique<QSocketNotifier>	_notifier;
	int						_device;
	bool					_good;
	std::string				_error;
	unsigned int			_read_failure_count			= 0;
	unsigned int			_max_read_failure_count		= 0;
	unsigned int			_write_failure_count		= 0;
	unsigned int			_max_write_failure_count	= 0;
	Blob					_input_buffer;				// Data from the device.
	Blob					_output_buffer;				// Data to to sent to the device.
};


inline std::string
SerialPort::Configuration::device_path() const noexcept
{
	return _device_path;
}


inline void
SerialPort::Configuration::set_device_path (std::string const& device_path)
{
	_device_path = device_path;
}


inline unsigned int
SerialPort::Configuration::baud_rate() const noexcept
{
	return _baud_rate;
}


inline void
SerialPort::Configuration::set_baud_rate (unsigned int baud_rate)
{
	_baud_rate = baud_rate;
}


inline void
SerialPort::Configuration::set_data_bits (unsigned int data_bits)
{
	_data_bits = xf::clamped (data_bits, 5u, 8u);
}


inline void
SerialPort::Configuration::set_parity_bit (Parity parity)
{
	_parity = parity;
}


inline void
SerialPort::Configuration::set_stop_bits (unsigned int stop_bits)
{
	_stop_bits = xf::clamped (stop_bits, 1u, 2u);
}


inline void
SerialPort::Configuration::set_hardware_flow_control (bool enabled)
{
	_rtscts = enabled;
}


inline void
SerialPort::set_data_ready_callback (DataReadyCallback callback)
{
	_data_ready = callback;
}


inline void
SerialPort::set_failure_callback (FailureCallback callback)
{
	_failure = callback;
}


inline void
SerialPort::set_configuration (Configuration const& conf)
{
	_configuration = conf;
}


inline SerialPort::Configuration const&
SerialPort::configuration() const noexcept
{
	return _configuration;
}


inline void
SerialPort::set_logger (Logger const& logger)
{
	_logger = &logger;
}


inline bool
SerialPort::good() const noexcept
{
	return _good;
}


inline std::string const&
SerialPort::error() const noexcept
{
	return _error;
}


inline Blob&
SerialPort::input_buffer() noexcept
{
	return _input_buffer;
}


inline bool
SerialPort::flushed() const noexcept
{
	return _output_buffer.empty();
}


inline Logger const&
SerialPort::log() const
{
	return *_logger;
}

} // namespace xf

#endif

