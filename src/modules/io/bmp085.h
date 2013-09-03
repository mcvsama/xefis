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

#ifndef XEFIS__MODULES__IO__BMP085_H__INCLUDED
#define XEFIS__MODULES__IO__BMP085_H__INCLUDED

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/utility/i2c.h>


/**
 * This module interfaces Bochs' BMP085 presure and temperature sensor.
 */
class BMP085:
	public QObject,
	public Xefis::Module
{
	Q_OBJECT

  public:
	enum Oversampling {
		Oversampling0	= 0,
		Oversampling1	= 1,
		Oversampling2	= 2,
		Oversampling3	= 3
	};

  public:
	// Ctor
	BMP085 (Xefis::ModuleManager*, QDomElement const& config);

  private slots:
	void
	initialize();

	void
	reinitialize();

	void
	request_temperature();

	void
	request_pressure();

	void
	read_temperature();

	void
	read_pressure();

  private:
	void
	guard (std::function<void()> guarded_code);

	void
	handle_other (void (BMP085::*request_function)());

	int32_t
	read_s16 (uint8_t base_register);

	uint32_t
	read_u16 (uint8_t base_register);

	uint32_t
	read_u24 (uint8_t base_register);

	void
	write (uint8_t base_register, uint8_t value);

  private:
	// Register addresses:
	static constexpr uint8_t	AC1_REG	= 0xaa;
	static constexpr uint8_t	AC2_REG	= 0xac;
	static constexpr uint8_t	AC3_REG	= 0xae;
	static constexpr uint8_t	AC4_REG	= 0xb0;
	static constexpr uint8_t	AC5_REG	= 0xb2;
	static constexpr uint8_t	AC6_REG	= 0xb4;
	static constexpr uint8_t	B1_REG	= 0xb6;
	static constexpr uint8_t	B2_REG	= 0xb8;
	static constexpr uint8_t	MB_REG	= 0xba;
	static constexpr uint8_t	MC_REG	= 0xbc;
	static constexpr uint8_t	MD_REG	= 0xbe;
	// Data:
	Xefis::PropertyFloat		_temperature;
	Time						_temperature_interval		= 500_ms;
	Xefis::PropertyPressure		_pressure;
	Time						_pressure_interval			= 50_ms;
	uint8_t						_i2c_bus_number;
	Xefis::I2C::Bus				_i2c_bus;
	Xefis::I2C::Address			_i2c_address;
	Oversampling				_oversampling				= Oversampling3;
	Time						_pressure_waiting_times[4]	= { 4.5_ms, 7.5_ms, 13.5_ms, 25.5_ms };
	QTimer*						_reinitialize_timer			= nullptr;
	QTimer*						_temperature_timer			= nullptr;
	QTimer*						_temperature_ready_timer	= nullptr;
	QTimer*						_pressure_timer				= nullptr;
	QTimer*						_pressure_ready_timer		= nullptr;
	// Set to true, between request_ and read_ functions.
	bool						_middle_of_request			= false;
	bool						_request_other				= false;
	// Calibration coeffs:
	int32_t						_ac1;
	int32_t						_ac2;
	int32_t						_ac3;
	int32_t						_ac4;
	int32_t						_ac5;
	int32_t						_ac6;
	int32_t						_b1;
	int32_t						_b2;
	int32_t						_b3;
	uint32_t					_b4;
	int32_t						_b5;
	int32_t						_b6;
	uint32_t					_b7;
	int32_t						_mb;
	int32_t						_mc;
	int32_t						_md;
	int32_t						_ut;
	int32_t						_up;
	int32_t						_ct;
	int32_t						_cp;
};

#endif
