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

#ifndef XEFIS__UTILITY__SERIAL_PORT_H__INCLUDED
#define XEFIS__UTILITY__SERIAL_PORT_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <functional>

// Qt:
#include <QtCore/QSocketNotifier>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/noncopyable.h>
#include <xefis/utility/logger.h>


namespace Xefis {

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

	typedef std::function<void()>	DataReadyCallback;
	typedef std::function<void()>	FailureCallback;

  public:
	/**
	 * Ctor
	 * @data_ready is called when there's something to read.
	 * @failure is called when failure is detected.
	 */
	SerialPort (DataReadyCallback data_ready, FailureCallback failure);

	// Dtor
	~SerialPort();

  public:
	/**
	 * Set logger.
	 */
	void
	set_logger (Logger const& logger);

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
	 * Set device options.
	 * Baud rate is parsed from baud_rate.
	 */
	void
	set_baud_rate (std::string const& baud_rate);

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
	set_hardware_control_flow (bool enabled);

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
	std::string&
	input_buffer() noexcept;

	/**
	 * Write data to the device. Data is written asynchronously,
	 * and it's possible that not all requested data is written
	 * when function returns.
	 */
	void
	write (std::string const& data);

	/**
	 * Request writing output-buffered data to the device.
	 * Same as write ("");
	 */
	void
	write();

	/**
	 * Return true if output buffer is not empty.
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
	log() const
	{
		return *_logger;
	}

  private:
	Logger const*		_logger						= nullptr;
	Logger				_internal_logger;
	DataReadyCallback	_data_ready;
	FailureCallback		_failure;
	QSocketNotifier*	_notifier					= nullptr;
	std::string			_device_path;
	std::string			_baud_rate;
	unsigned int		_data_bits					= 8;
	Parity				_parity						= Parity::None;
	unsigned int		_stop_bits					= 1;
	bool				_rtscts						= false;
	int					_device;
	bool				_good;
	std::string			_error;
	unsigned int		_read_failure_count			= 0;
	unsigned int		_max_read_failure_count		= 0;
	unsigned int		_write_failure_count		= 0;
	unsigned int		_max_write_failure_count	= 0;
	std::string			_input_buffer;				// Data from the device.
	std::string			_output_buffer;				// Data to to sent to the device.
};


inline void
SerialPort::set_logger (Logger const& logger)
{
	_logger = &logger;
}


inline std::string
SerialPort::device_path() const noexcept
{
	return _device_path;
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


inline std::string&
SerialPort::input_buffer() noexcept
{
	return _input_buffer;
}


inline void
SerialPort::write()
{
	write ("");
}


inline bool
SerialPort::flushed() const noexcept
{
	return _output_buffer.empty();
}

} // namespace Xefis

#endif

