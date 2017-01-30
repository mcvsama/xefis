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

// System:
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>

// Lib:
#include <boost/format.hpp>

// Xefis:
#include <xefis/core/stdexcept.h>

// Local:
#include "i2c.h"


namespace xf::i2c {

Message::Message (Operation operation, Address const& address, uint8_t* data, std::size_t size):
	_operation (operation),
	_address (address),
	_data (data),
	_size (size)
{ }


Message::Message (Operation operation, Address const& address, std::vector<uint8_t>& sequence):
	_operation (operation),
	_address (address),
	_data (&sequence[0]),
	_size (sequence.size())
{ }


struct ::i2c_msg
Message::generate_i2c_msg() const noexcept
{
	struct ::i2c_msg msg;
	msg.addr = _address.address();
	msg.flags = 0;
	if (_address.is_ten_bit())
		msg.flags |= I2C_M_TEN;
	if (_operation == Read)
		msg.flags |= I2C_M_RD;
	msg.buf = _data;
	msg.len = _size;
	return msg;
}


Bus::Bus() noexcept
{ }


Bus::Bus (uint8_t bus_number) noexcept
{
	open (bus_number);
}


Bus::Bus (Bus&& other):
	_open (other._open),
	_bus_number (other._bus_number),
	_device (other._device)
{
	other._open = false;
}


Bus::~Bus()
{
	close();
}


void
Bus::open()
{
	open (_bus_number);
}


void
Bus::open (uint8_t bus_number)
{
	close();
	_bus_number = bus_number;
	_device = ::open ((boost::format ("/dev/i2c-%1%") % static_cast<int> (_bus_number)).str().c_str(), O_RDWR);
	if (_device < 0)
		throw IOError ((boost::format ("could not open I²C bus %1%: %2%") % static_cast<int> (_bus_number) % strerror (errno)).str());
	_open = true;
}


bool
Bus::good() const
{
	return !!_open;
}


void
Bus::close()
{
	if (_open)
	{
		::close (_device);
		_open = false;
	}
}


void
Bus::execute (Transaction const& transaction)
{
	std::vector<struct ::i2c_msg> msgs (transaction.size());
	for (Transaction::size_type i = 0; i < transaction.size(); ++i)
		msgs[i] = transaction[i].generate_i2c_msg();

	struct ::i2c_rdwr_ioctl_data msgset;
	msgset.msgs = &msgs[0];
	msgset.nmsgs = transaction.size();

	if (ioctl (_device, I2C_RDWR, &msgset) < 0)
		throw IOError ((boost::format ("could not execute I²C transaction: %1%") % strerror (errno)).str());
}


void
Device::open()
{
	ensure_open();
}


void
Device::close()
{
	_bus.close();
}


Device::Device (Bus::ID bus_id, Address const& address):
	_bus (bus_id),
	_address (address)
{ }


void
Device::read_register (Register reg, uint8_t* data, std::size_t size)
{
	ensure_open();
	_bus.execute ({ Message (Write, _address, &reg, sizeof (reg)),
					Message (Read, _address, data, size) });
}


void
Device::read_register (Register reg, std::vector<uint8_t> const& data)
{
	ensure_open();
	_bus.execute ({ Message (Write, _address, &reg, sizeof (reg)),
					Message (Read, _address, const_cast<uint8_t*> (data.data()), data.size()) });
}


void
Device::write_register (Register reg, std::vector<uint8_t> const& data)
{
	ensure_open();
	std::vector<uint8_t> data_to_write;
	std::copy (data.begin(), data.end(), data_to_write.begin() + 1);
	data_to_write[0] = reg;
	_bus.execute ({ Message (Write, _address, data_to_write) });
}


void
Device::ensure_open()
{
	if (!_bus.good())
		_bus.open();
}

} // namespace xf::i2c

