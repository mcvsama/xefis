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

// Standard:
#include <cstddef>
#include <memory>
#include <iostream>

// Lib:
#include <boost/endian/conversion.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/time.h>

// Local:
#include "bmp085.h"


BMP085::BMP085 (std::string const& instance):
	Module (instance)
{
	_reinitialize_timer = std::make_unique<QTimer> (this);
	_reinitialize_timer->setInterval (250);
	_reinitialize_timer->setSingleShot (true);
	QObject::connect (_reinitialize_timer.get(), SIGNAL (timeout()), this, SLOT (initialize()));

	_serviceable = false;
}


void
BMP085::initialize()
{
	_i2c_device.bus().set_bus_number (setting_i2c_bus);
	_i2c_device.set_address (xf::i2c::Address (setting_i2c_address));

	guard ([&] {
		hw_initialize();
	});
}


void
BMP085::hw_initialize()
{
	guard ([&] {
		_i2c_device.open();

		_ac1 = read_s16 (AC1_REG);
		_ac2 = read_s16 (AC2_REG);
		_ac3 = read_s16 (AC3_REG);
		_ac4 = read_u16 (AC4_REG);
		_ac5 = read_u16 (AC5_REG);
		_ac6 = read_u16 (AC6_REG);
		_b1 = read_s16 (B1_REG);
		_b2 = read_s16 (B2_REG);
		_mb = read_s16 (MB_REG);
		_mc = read_s16 (MC_REG);
		_md = read_s16 (MD_REG);

		_temperature_timer = std::make_unique<QTimer> (this);
		_temperature_timer->setInterval (setting_temperature_update_interval->quantity<Millisecond>());
		_temperature_timer->setSingleShot (false);
		QObject::connect (_temperature_timer.get(), SIGNAL (timeout()), this, SLOT (request_temperature()));

		_temperature_ready_timer = std::make_unique<QTimer> (this);
		_temperature_ready_timer->setInterval (5);
		_temperature_ready_timer->setSingleShot (true);
		QObject::connect (_temperature_ready_timer.get(), SIGNAL (timeout()), this, SLOT (read_temperature()));

		_pressure_timer = std::make_unique<QTimer> (this);
		_pressure_timer->setInterval (setting_pressure_update_interval->quantity<Millisecond>());
		_pressure_timer->setSingleShot (false);
		QObject::connect (_pressure_timer.get(), SIGNAL (timeout()), this, SLOT (request_pressure()));

		_pressure_ready_timer = std::make_unique<QTimer> (this);
		_pressure_ready_timer->setInterval (_pressure_waiting_times[_oversampling].quantity<Millisecond>());
		_pressure_ready_timer->setSingleShot (true);
		QObject::connect (_pressure_ready_timer.get(), SIGNAL (timeout()), this, SLOT (read_pressure()));

		_temperature_timer->start();
		_pressure_timer->start();
	});
}


void
BMP085::hw_reinitialize()
{
	_serviceable = false;
	_temperature.set_nil();
	_pressure.set_nil();

	_middle_of_request = false;
	_request_other = false;

	_temperature_timer.reset();
	_temperature_ready_timer.reset();
	_pressure_timer.reset();
	_pressure_ready_timer.reset();

	_reinitialize_timer->start();
}


void
BMP085::request_temperature()
{
	if (_middle_of_request)
		_request_other = true;
	else
	{
		guard ([&] {
			_middle_of_request = true;
			write (0xf4, 0x2e);
			_temperature_ready_timer->start();
		});
	}
}


void
BMP085::request_pressure()
{
	if (_middle_of_request)
		_request_other = true;
	else
	{
		guard ([&] {
			_middle_of_request = true;
			int os = static_cast<int> (_oversampling);
			write (0xf4, 0x34 + (os << 6));
			_pressure_ready_timer->start();
		});
	}
}


void
BMP085::read_temperature()
{
	_middle_of_request = false;

	guard ([&] {
		_ut = read_u16 (0xf6);
		int32_t x1 = ((_ut - _ac6) * _ac5) >> 15;
		int32_t x2 = (_mc << 11) / (x1 + _md);
		_b5 = x1 + x2;
		_ct = (_b5 + 8) >> 4;
		_temperature = Quantity<Celsius> (_ct / 10.0);

		handle_other (&BMP085::request_pressure);
	});
}


void
BMP085::read_pressure()
{
	_middle_of_request = false;

	guard ([&] {
		int os = static_cast<int> (_oversampling);
		_up = read_u24 (0xf6) >> (8 - os);
		_b6 = _b5 - 4000;
		int32_t x1 = (_b2 * (_b6 * _b6 >> 12)) >> 11;
		int32_t x2 = _ac2 * _b6 >> 11;
		int32_t x3 = x1 + x2;
		_b3 = (((_ac1 * 4 + x3) << os) + 2) >> 2;
		x1 = _ac3 * _b6 >> 13;
		x2 = (_b1 * (_b6 * _b6 >> 12)) >> 16;
		x3 = ((x1 + x2) + 2) >> 2;
		_b4 = (_ac4 * static_cast<uint32_t> (x3 + 32768)) >> 15;
		_b7 = static_cast<uint32_t> (_up - _b3) * (50000 >> os);
		if (_b7 < 0x80000000)
			_cp = (_b7 * 2) / _b4;
		else
			_cp = (_b7 / _b4) * 2;
		x1 = (_cp >> 8) * (_cp >> 8);
		x1 = (x1 * 3038) >> 16;
		x2 = (-7357 * _cp) >> 16;
		_cp = _cp + ((x1 + x2 + 3791) >> 4);
		_pressure = 0.01_hPa * _cp;

		handle_other (&BMP085::request_temperature);

		_serviceable = true;
	});
}


void
BMP085::guard (std::function<void()> guarded_code)
{
	try {
		guarded_code();
	}
	catch (xf::IOError& e)
	{
		log() << "I/O error: " << e.message() << std::endl;
		hw_reinitialize();
	}
	catch (...)
	{
		hw_reinitialize();
	}
}


void
BMP085::handle_other (void (BMP085::*request_function)())
{
	if (_request_other)
	{
		_request_other = false;
		(this->*request_function)();
	}
	else
		_request_other = false;
}


int32_t
BMP085::read_s16 (uint8_t base_register)
{
	int16_t value = _i2c_device.read_register<int16_t> (base_register);
	boost::endian::big_to_native (value);
	return value;
}


uint32_t
BMP085::read_u16 (uint8_t base_register)
{
	uint16_t value = _i2c_device.read_register<uint16_t> (base_register);
	boost::endian::big_to_native (value);
	return value;
}


uint32_t
BMP085::read_u24 (uint8_t base_register)
{
	uint32_t value = 0;
	uint8_t* value_8 = reinterpret_cast<uint8_t*> (&value);
	_i2c_device.read_register (base_register, value_8 + 1, 3);
	boost::endian::big_to_native (value);
	return value;
}


void
BMP085::write (uint8_t base_register, uint8_t value)
{
	_i2c_device.write_register (base_register, value);
}

