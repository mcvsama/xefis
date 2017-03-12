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

// Standard:
#include <cstddef>
#include <map>

// System:
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

// Lib:
#include <boost/lexical_cast.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/mutex.h>

// Local:
#include "serial_port.h"


namespace xf {

static Mutex termios_baud_rate_entry_mutex;


SerialPort::SerialPort (DataReadyCallback data_ready, FailureCallback failure):
	_data_ready (data_ready),
	_failure (failure)
{
	_internal_logger.set_prefix ("[serial port]");
	_logger = &_internal_logger;
}


SerialPort::SerialPort (SerialPort&& other)
{
	*this = std::move (other);
}


SerialPort::~SerialPort()
{
	close();
}


SerialPort&
SerialPort::operator= (SerialPort&& other)
{
	_owned = std::move (other._owned);
	_logger = other._logger;
	_internal_logger = other._internal_logger;
	_configuration = other._configuration;
	_data_ready = other._data_ready;
	_failure = other._failure;
	_notifier = std::move (other._notifier);
	_device = other._device;
	_good = other._good;
	_error = other._error;
	_read_failure_count = other._read_failure_count;
	_max_read_failure_count = other._max_read_failure_count;
	_write_failure_count = other._write_failure_count;
	_max_write_failure_count = other._max_write_failure_count;
	_input_buffer = other._input_buffer;
	_output_buffer = other._output_buffer;
	return *this;
}


void
SerialPort::set_max_read_failures (unsigned int number)
{
	_max_read_failure_count = number;
}


void
SerialPort::set_max_write_failures (unsigned int number)
{
	_max_write_failure_count = number;
}


void
SerialPort::write (Blob const& data)
{
	if (!_good)
		throw WriteException ("can't write to serial port - device is closed");

	_output_buffer.insert (_output_buffer.end(), data.begin(), data.end());

	int written = ::write (_device, _output_buffer.data(), _output_buffer.size());

	if (written == -1)
	{
		log() << log_prefix() << "Write error " << strerror (errno) << std::endl;

		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			log() << log_prefix() << "Write failure (could not write " << _output_buffer.size() << " bytes)." << std::endl;
			_write_failure_count++;
			if (_write_failure_count > _max_write_failure_count)
				notify_failure ("multiple write failures");
		}
		else
			log() << log_prefix() << "Write failure: would block." << std::endl;
	}
	else if (written < static_cast<int> (_output_buffer.size()))
	{
		log() << log_prefix() << "Write buffer overrun." << std::endl;

		_output_buffer.erase (_output_buffer.begin(), _output_buffer.begin() + written);
	}
	else
	{
		_output_buffer.clear();
		_write_failure_count = 0;
	}
}


void
SerialPort::write (std::string const& data)
{
	_output_buffer.insert (_output_buffer.end(), data.begin(), data.end());
	flush_async();
}


void
SerialPort::flush_async()
{
	write (Blob());
}


void
SerialPort::flush()
{
	::fsync (_device);
}


bool
SerialPort::open()
{
	log() << log_prefix() << "Opening device " << _configuration._device_path << " at " << configuration().baud_rate() << std::endl;
	close();
	_device = ::open (_configuration._device_path.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

	if (_device < 0)
	{
		auto es = strerror (errno);
		_error = es;
		_good = false;
		log() << log_prefix() << "Could not open device file " << _configuration._device_path << ": " << es << std::endl;
	}
	else
	{
		if (set_device_options())
		{
			log() << log_prefix() << "Open at " << configuration().baud_rate() << "." << std::endl;
			_good = true;
			_notifier = std::make_unique<QSocketNotifier> (_device, QSocketNotifier::Read, this);
			_notifier->setEnabled (true);
			QObject::connect (_notifier.get(), SIGNAL (activated (int)), this, SLOT (read()));
		}
		else
			log() << log_prefix() << "Failed to set device parameters." << std::endl;
	}

	return good();
}


void
SerialPort::close()
{
	if (_owned)
	{
		_notifier.reset();

		if (_device)
		{
			::close (_device);
			_device = 0;
			_good = false;
			_error.clear();
			_read_failure_count = 0;
			_write_failure_count = 0;
			_input_buffer.clear();
			_output_buffer.clear();
		}
	}
}


int
SerialPort::termios_baud_rate (unsigned int baud_rate)
{
	// Must acquire lock before statically- and non-statically initializing static variables:
	auto lock = termios_baud_rate_entry_mutex.acquire_lock();

	static std::map<int, int> baud_rates_map;

	if (baud_rates_map.empty())
	{
		baud_rates_map[50] = B50;
		baud_rates_map[75] = B75;
		baud_rates_map[110] = B110;
		baud_rates_map[134] = B134;
		baud_rates_map[150] = B150;
		baud_rates_map[200] = B200;
		baud_rates_map[300] = B300;
		baud_rates_map[600] = B600;
		baud_rates_map[1200] = B1200;
		baud_rates_map[1800] = B1800;
		baud_rates_map[2400] = B2400;
		baud_rates_map[4800] = B4800;
		baud_rates_map[9600] = B9600;
		baud_rates_map[19200] = B19200;
		baud_rates_map[38400] = B38400;
		baud_rates_map[57600] = B57600;
		baud_rates_map[115200] = B115200;
		baud_rates_map[230400] = B230400;
		baud_rates_map[460800] = B460800;
		baud_rates_map[500000] = B500000;
		baud_rates_map[576000] = B576000;
		baud_rates_map[921600] = B921600;
		baud_rates_map[1000000] = B1000000;
		baud_rates_map[1152000] = B1152000;
		baud_rates_map[1500000] = B1500000;
		baud_rates_map[2000000] = B2000000;
		baud_rates_map[2500000] = B2500000;
		baud_rates_map[3000000] = B3000000;
		baud_rates_map[3500000] = B3500000;
		baud_rates_map[4000000] = B4000000;
	}

	auto c = baud_rates_map.find (baud_rate);
	if (c == baud_rates_map.end())
		c = baud_rates_map.upper_bound (baud_rate);
	if (c == baud_rates_map.end())
		return 0;
	return c->second;
}


int
SerialPort::termios_baud_rate (std::string const& baud_rate)
{
	return termios_baud_rate (boost::lexical_cast<unsigned int> (baud_rate));
}


void
SerialPort::read()
{
	if (!_good)
		return;

	std::string buffer;

	bool err = false;
	bool exc = xf::Exception::guard ([&] {
		// Read as much as possible:
		for (;;)
		{
			std::string::size_type prev_size = buffer.size();
			std::string::size_type try_read = 4096;
			buffer.resize (prev_size + try_read);
			int n = ::read (_device, &buffer[prev_size], try_read);

			if (n < 0)
			{
				if (errno == EAGAIN || errno == EWOULDBLOCK)
				{
					// Nothing to read (read would block)
					buffer.resize (prev_size);
					log() << log_prefix() << "Nothing to read (read would block)." << std::endl;
					break;
				}
				else
				{
					log() << log_prefix() << "Error while reading from serial port: " << strerror (errno) << std::endl;
					err = true;
					break;
				}
			}
			else
			{
				buffer.resize (prev_size + n);
				if (n == 0)
				{
					log() << log_prefix() << "Read failure (0 bytes read by read())." << std::endl;
					_read_failure_count++;
					if (_read_failure_count > _max_read_failure_count)
						notify_failure ("multiple read failures");
				}

				if (n < static_cast<int> (try_read))
					break;
			}
		}
	});

	if (exc || err)
		notify_failure ("read()");

	if (!buffer.empty())
	{
		_input_buffer.insert (_input_buffer.end(), buffer.begin(), buffer.end());
		if (_data_ready)
			_data_ready();
	}
}


bool
SerialPort::set_device_options()
{
	std::string parity_str = "none";
	if (_configuration._parity == Parity::Odd)
		parity_str = "odd";
	else if (_configuration._parity == Parity::Even)
		parity_str = "even";
	log() << log_prefix() << "Setting baud rate: " << _configuration._baud_rate << ", data bits: " << _configuration._data_bits
		  << ", parity: " << parity_str << ", stop bits: " << _configuration._stop_bits << std::endl;

	termios options;
	bzero (&options, sizeof (options));

	// Min characters to be read:
	options.c_cc[VMIN] = 0;
	// Time to wait for data (tenths of seconds):
	options.c_cc[VTIME] = 0;
	// Set output and local modes to defaults:
	options.c_cflag = CREAD | CLOCAL;

	switch (_configuration._data_bits)
	{
		case 5: options.c_cflag |= CS5; break;
		case 6: options.c_cflag |= CS6; break;
		case 7: options.c_cflag |= CS7; break;
		case 8: options.c_cflag |= CS8; break;
	}

	if (_configuration._stop_bits == 2)
		options.c_cflag |= CSTOPB;

	switch (_configuration._parity)
	{
		case Parity::None:	break;
		case Parity::Odd:	options.c_cflag |= PARENB | PARODD; break;
		case Parity::Even:	options.c_cflag |= PARENB; break;
	}

	if (_configuration._rtscts)
		options.c_cflag |= CRTSCTS;

	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;

	int baud_rate_const = termios_baud_rate (_configuration._baud_rate);
	cfsetispeed (&options, baud_rate_const);
	cfsetospeed (&options, baud_rate_const);

	tcflush (_device, TCIOFLUSH);

	if (tcsetattr (_device, TCSAFLUSH, &options) != 0)
	{
		log() << log_prefix() << "Could not setup serial port: " << _configuration._device_path << ": " << strerror (errno) << std::endl;
		return false;
	}

	if (tcflow (_device, TCOON | TCION) != 0)
	{
		log() << log_prefix() << "Could not enable flow: tcflow(): " << _configuration._device_path << ": " << strerror (errno) << std::endl;
		return false;
	}

	return true;
}


void
SerialPort::notify_failure (std::string const& message)
{
	_error = message;
	log() << log_prefix() << "Failure detected: " << message << std::endl;
	if (_failure)
		_failure();
	close();
}


std::string
SerialPort::log_prefix() const
{
	return (boost::format ("SerialPort<%p>: ") % static_cast<void const*> (this)).str();
}

} // namespace xf

