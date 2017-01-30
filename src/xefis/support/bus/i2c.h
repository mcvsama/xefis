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

#ifndef XEFIS__SUPPORT__BUS__I2C_H__INCLUDED
#define XEFIS__SUPPORT__BUS__I2C_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>
#include <array>

// System:
#include <linux/i2c-dev.h>
#include <linux/i2c.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/noncopyable.h>


namespace xf::i2c {

class Bus;
class Address;
class Message;
class Device;

typedef std::vector<Message> Transaction;


enum Operation {
	Write,	// Message will be sent to slave
	Read	// Message will be read from slave
};


class Bus: public Noncopyable
{
  public:
	typedef uint8_t ID;

  public:
	/**
	 * Create not opened bus.
	 */
	Bus() noexcept;

	/**
	 * Open Linux I2C device.
	 * \param	bus_number I2C bus number.
	 */
	explicit
	Bus (ID bus_number) noexcept;

	// Move ctor
	Bus (Bus&&);

	// Dtor
	~Bus();

	/**
	 * Return bus number.
	 */
	uint8_t
	bus_number() const noexcept;

	/**
	 * Set bus number.
	 */
	void
	set_bus_number (ID bus_number) noexcept;

	/**
	 * Reopen bus.
	 */
	void
	open();

	/**
	 * Open bus.
	 */
	void
	open (ID bus_number);

	/**
	 * Return true if bus was correctly open
	 * and is in good state.
	 */
	bool
	good() const;

	/**
	 * Close bus.
	 */
	void
	close();

	/**
	 * Execute I2C transaction.
	 */
	void
	execute (Transaction const&);

  private:
	bool	_open			= false;
	ID		_bus_number		= 0;
	int		_device			= 0;
};


class Address
{
  public:
	typedef uint16_t ID;

  public:
	/**
	 * Create address 0x00.
	 */
	Address() noexcept;

	/**
	 * Create I2C address.
	 * \param	address 7- or 10-bit I2C address.
	 * \param	ten_bit Tells whether address is 10-bit.
	 */
	explicit
	Address (ID address, bool ten_bit = false) noexcept;

	/**
	 * Return address.
	 */
	ID
	address() const noexcept;

	/**
	 * Return true if address is 10-bit.
	 */
	bool
	is_ten_bit() const noexcept;

  private:
	ID		_address;
	bool	_ten_bit;
};


class Message
{
  public:
	/**
	 * Create I2C Message.
	 * Data is represented by range [data, data + size). Message does not
	 * make a copy of the data.
	 */
	Message (Operation, Address const&, uint8_t* data, std::size_t size);

	/**
	 * Create I2C Message.
	 * Data is read from given vector.
	 */
	Message (Operation, Address const&, std::vector<uint8_t>& vector);

	/**
	 * Create I2C Message.
	 * Data is read from given array.
	 */
	template<std::size_t SequenceSize>
		Message (Operation, Address const&, std::array<uint8_t, SequenceSize>& array);

	/**
	 * Generate struct for use with Linux I2C API.
	 */
	struct ::i2c_msg
	generate_i2c_msg() const noexcept;

  private:
	Operation	_operation;
	Address		_address;
	uint8_t*	_data	= nullptr;
	std::size_t	_size	= 0;
};


class Device
{
  public:
	typedef uint8_t Register;

  public:
	// Ctor
	Device() = default;

	// Ctor
	Device (Bus::ID bus_id, Address const&);

	/**
	 * Open the device.
	 */
	void
	open();

	/**
	 * Close the device.
	 */
	void
	close();

	/**
	 * Return bus.
	 */
	Bus&
	bus();

	/**
	 * Return bus.
	 */
	Bus const&
	bus() const;

	/**
	 * Return used address.
	 */
	Address const&
	address() const;

	/**
	 * Set address.
	 */
	void
	set_address (Address const&);

	/**
	 * Read value from the device.
	 */
	template<class Type = uint8_t>
		Type
		read();

	/**
	 * Read register value from the device.
	 */
	template<class Type = uint8_t>
		Type
		read_register (Register reg);

	/**
	 * Read register value from the device.
	 */
	void
	read_register (Register reg, uint8_t* data, std::size_t size);

	/**
	 * Read series of bytes from the device.
	 */
	void
	read_register (Register reg, std::vector<uint8_t> const& data);

	/**
	 * Read series of bytes from the device.
	 */
	template<std::size_t Size>
		void
		read_register (Register reg, std::array<uint8_t, Size> const& data);

	/**
	 * Write value to the device.
	 */
	template<class Type>
		void
		write (Type value);

	/**
	 * Write given value to a register.
	 */
	template<class Type>
		void
		write_register (Register reg, Type value);

	/**
	 * Write series of bytes to the device.
	 */
	void
	write_register (Register reg, uint8_t* data, std::size_t size);

	/**
	 * Write series of bytes to the device.
	 */
	void
	write_register (Register reg, std::vector<uint8_t> const& data);

	/**
	 * Write series of bytes to the device.
	 */
	template<std::size_t Size>
		void
		write_register (Register reg, std::array<uint8_t, Size> const& data);

  private:
	/**
	 * Try to open bus if not open yet.
	 */
	void
	ensure_open();

  private:
	Bus		_bus;
	Address	_address;
};


template<std::size_t SequenceSize>
	Message::Message (Operation operation, Address const& address, std::array<uint8_t, SequenceSize>& sequence):
		_operation (operation),
		_address (address),
		_data (sequence.data()),
		_size (sequence.size())
	{ }


inline uint8_t
Bus::bus_number() const noexcept
{
	return _bus_number;
}


inline void
Bus::set_bus_number (uint8_t bus_number) noexcept
{
	_bus_number = bus_number;
}


inline
Address::Address() noexcept:
	_address (0x00),
	_ten_bit (0)
{ }


inline
Address::Address (ID address, bool ten_bit) noexcept:
	_address (address),
	_ten_bit (ten_bit)
{ }


inline Address::ID
Address::address() const noexcept
{
	return _address;
}


inline bool
Address::is_ten_bit() const noexcept
{
	return _ten_bit;
}


inline Bus&
Device::bus()
{
	return _bus;
}


inline Bus const&
Device::bus() const
{
	return _bus;
}


inline Address const&
Device::address() const
{
	return _address;
}


inline void
Device::set_address (Address const& address)
{
	_address = address;
}


template<class Type>
	Type
	Device::read()
	{
		ensure_open();
		Type result;
		_bus.execute ({ Message (Read, _address, reinterpret_cast<uint8_t*> (&result), sizeof (result)) });
		return result;
	}


template<class Type>
	Type
	Device::read_register (Register reg)
	{
		ensure_open();
		Type result;
		_bus.execute ({ Message (Write, _address, &reg, sizeof (reg)),
						Message (Read, _address, reinterpret_cast<uint8_t*> (&result), sizeof (result)) });
		return result;
	}


template<std::size_t Size>
	void
	Device::read_register (Register reg, std::array<uint8_t, Size> const& data)
	{
		ensure_open();
		_bus.execute ({ Message (Write, _address, &reg, sizeof (reg)),
						Message (Read, _address, const_cast<uint8_t*> (data.data()), data.size()) });
	}


template<class Type>
	void
	Device::write (Type value)
	{
		ensure_open();
		_bus.execute ({ Message (Write, _address, reinterpret_cast<uint8_t*> (&value), sizeof (value)) });
	}


template<class Type>
	void
	Device::write_register (Register reg, Type value)
	{
		ensure_open();
		std::array<uint8_t, sizeof (value) + 1> data;
		data[0] = reg;
		for (std::size_t b = 0; b < sizeof (value); ++b)
			data[b + 1] = (value >> (8 * (sizeof (value) - b - 1))) & 0xff;
		_bus.execute ({ Message (Write, _address, data) });
	}


template<std::size_t Size>
	void
	Device::write_register (Register reg, std::array<uint8_t, Size> const& data)
	{
		ensure_open();
		std::array<uint8_t, Size + 1> data_to_write;
		std::copy (data.begin(), data.end(), data_to_write.begin() + 1);
		data_to_write[0] = reg;
		_bus.execute ({ Message (Write, _address, data_to_write) });
	}

} // namespace xf::i2c

#endif

