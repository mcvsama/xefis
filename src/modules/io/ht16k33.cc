/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
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

// Lib:
#include <boost/lexical_cast.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "ht16k33.h"


XEFIS_REGISTER_MODULE_CLASS ("io/ht16k33", HT16K33);


std::array<uint8_t, 11> HT16K33::_digit_symbols = { {
	0x3f, // 0 abcdef .
	0x06, // 1  bc    .
	0x5b, // 2 ab de g.
	0x4f, // 3 abcd  g.
	0x66, // 4  bc  fg.
	0x6d, // 5 a cd fg.
	0x7d, // 6 a cdefg.
	0x07, // 7 abc    .
	0x7f, // 8 abcdefg.
	0x6f, // 9 abcd fg.
	0x40, // -       g.
} };


HT16K33::LedMatrix::LedMatrix()
{
	clear();
}


inline void
HT16K33::LedMatrix::clear()
{
	_data_array.fill (0);
}


inline void
HT16K33::LedMatrix::set (Row row, Column column, bool value) noexcept
{
	uint8_t byte = 2 * column + (row < 8 ? 0 : 1);
	uint8_t bit = row % 8;
	if (value)
		_data_array[byte] |= 1 << bit;
	else
		_data_array[byte] &= ~(1 << bit);
}


inline void
HT16K33::LedMatrix::set_column (Row row, uint8_t column_bits) noexcept
{
	for (unsigned int i = 0; i < 8; ++i)
		set (row, i, (column_bits >> i) & 1);
}


inline HT16K33::LedMatrix::DataArray&
HT16K33::LedMatrix::array()
{
	return _data_array;
}


HT16K33::KeyMatrix::KeyMatrix()
{
	clear();
}


inline void
HT16K33::KeyMatrix::clear()
{
	_data_array.fill (0);
}


inline bool
HT16K33::KeyMatrix::get (Row row, Column column) const noexcept
{
	column -= 1;
	row -= 3;
	uint8_t byte = 2 * column + (row < 8 ? 0 : 1);
	uint8_t bit = row % 8;
	return !!(_data_array[byte] & (1 << bit));
}


inline HT16K33::KeyMatrix::DataArray&
HT16K33::KeyMatrix::array()
{
	return _data_array;
}


HT16K33::SingleLed::SingleLed (QDomElement const& element)
{
	if (!element.hasAttribute ("row"))
		throw Xefis::MissingDomAttribute (element, "row");
	if (!element.hasAttribute ("column"))
		throw Xefis::MissingDomAttribute (element, "column");
	if (!element.hasAttribute ("path"))
		throw Xefis::MissingDomAttribute (element, "path");

	_row = Xefis::limit<int> (element.attribute ("row").toInt(), 0, 15);
	_column = Xefis::limit<int> (element.attribute ("column").toInt(), 0, 7);
	_property_boolean.set_path (element.attribute ("path").toStdString());
}


inline void
HT16K33::SingleLed::update_led_matrix (LedMatrix& led_matrix) const
{
	led_matrix.set (_row, _column, _property_boolean.read (false));
}


HT16K33::NumericDisplay::NumericDisplay (QDomElement const& element)
{
	_rounding = element.hasAttribute ("rounding") && element.attribute ("rounding") == "true";

	if (!element.hasAttribute ("path"))
		throw Xefis::MissingDomAttribute (element, "path");
	_property.set_path (element.attribute ("path").toStdString());

	for (QDomElement& e: element)
	{
		if (e == "digit")
		{
			if (!e.hasAttribute ("row"))
				throw Xefis::MissingDomAttribute (e, "row");
			int row = Xefis::limit (e.attribute ("row").toInt(), 0, 15);
			_digit_rows.push_back (row);
		}
	}
}


void
HT16K33::NumericDisplay::update_led_matrix (LedMatrix& led_matrix) const
{
	if (_digit_rows.empty())
		return;

	std::vector<uint8_t> digits;
	digits.reserve (16);

	Xefis::PropertyInteger::Type integer = get_integer_value();

	digits.clear();
	for (auto c: QString ("%1").arg (integer))
		digits.push_back (c.digitValue());

	auto clear_all_digits = [&] {
		for (Row row: _digit_rows)
			led_matrix.set_column (row, 0);
	};

	auto set_all_digits_9 = [&] {
		for (Row row: _digit_rows)
			led_matrix.set_column (row, _digit_symbols[9]);
	};

	auto normally_display = [&] {
		std::size_t ds = digits.size();
		for (unsigned int i = 0; i < ds; ++i)
			led_matrix.set_column (_digit_rows[ds - i - 1], _digit_symbols[digits[i]]);
	};

	if (integer >= 0)
	{
		if (digits.size() > _digit_rows.size())
			set_all_digits_9();
		else
		{
			clear_all_digits();
			normally_display();
		}
	}
	else
	{
		if (_digit_rows.size() == 1)
			// Only the '-' sign:
			led_matrix.set_column (_digit_rows.back(), _digit_symbols[10]);
		else if (digits.size() > _digit_rows.size())
		{
			set_all_digits_9();
			// Minus sign:
			led_matrix.set_column (_digit_rows.back(), _digit_symbols[10]);
		}
		else
		{
			clear_all_digits();
			normally_display();
			// Minus sign:
			led_matrix.set_column (_digit_rows[digits.size()-1], _digit_symbols[10]);
		}
	}
}


Xefis::PropertyInteger::Type
HT16K33::NumericDisplay::get_integer_value() const
{
	try {
		Xefis::PropertyFloat::Type value = _property.floatize (_unit);
		Xefis::PropertyInteger::Type int_value = value;

		if (_rounding)
			int_value = value + 0.5;

		return int_value;
	}
	catch (UnsupportedUnit const&)
	{
		return 0;
	}
}


HT16K33::SingleSwitch::SingleSwitch (QDomElement const& element)
{
	if (!element.hasAttribute ("path"))
		throw Xefis::MissingDomAttribute (element, "path");
	_property_boolean.set_path (element.attribute ("path").toStdString());

	_row = Xefis::limit<int> (element.attribute ("row").toInt(), 3, 15);
	_column = Xefis::limit<int> (element.attribute ("column").toInt(), 1, 3);
}


bool
HT16K33::SingleSwitch::key_matrix_updated (KeyMatrix const& key_matrix)
{
	bool prev_value = *_property_boolean;
	bool next_value = key_matrix.get (_row, _column);
	_property_boolean.write (next_value);
	return prev_value != next_value;
}


void
HT16K33::SingleSwitch::invalidate()
{
	_property_boolean.set_nil();
}


HT16K33::HT16K33 (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	Xefis::I2C::Bus::ID i2c_bus;
	Xefis::I2C::Address::ID i2c_address;

	parse_settings (config, {
		{ "i2c.bus", i2c_bus, true },
		{ "i2c.address", i2c_address, true },
	});

	parse_properties (config, {
		{ "enabled", _enabled, false },
		{ "brightness", _brightness, false },
		{ "blinking", _blinking, false },
		{ "blinking.mode", _blinking_mode, false },
	});

	for (QDomElement& e: config)
	{
		if (e == "input")
		{
			_reliable_mode = e.attribute ("reliable-mode") == "true";

			if (!e.hasAttribute ("scan-frequency"))
				throw Xefis::MissingDomAttribute (e, "scan-frequency");
			_scan_frequency.parse (e.attribute ("scan-frequency").toStdString());

			if (_scan_frequency > 25_Hz && !_reliable_mode)
				throw Xefis::BadDomAttribute (e, "scan-frequency", "if greater than 25 Hz, 'reliable-mode' must be 'true'");

			// According to docs, each scan takes 20 ms, so limit sampling rate to 50 Hz:
			_scan_frequency = Xefis::limit (_scan_frequency, 0_Hz, 50_Hz);

			for (QDomElement& e2: e)
			{
				if (e2 == "single-switch")
					_switches.push_back (std::make_shared<SingleSwitch> (e2));
				else
					throw Xefis::BadDomElement (e2);
			}
		}
		else if (e == "output")
		{
			for (QDomElement& e2: e)
			{
				if (e2 == "numeric-display")
					_displays.push_back (std::make_shared<NumericDisplay> (e2));
				else if (e2 == "single-led")
					_displays.push_back (std::make_shared<SingleLed> (e2));
				else
					throw Xefis::BadDomElement (e2);
			}
		}
	}

	_i2c_device.bus().set_bus_number (i2c_bus);
	_i2c_device.set_address (Xefis::I2C::Address (i2c_address));

	_reinitialize_timer = new QTimer (this);
	_reinitialize_timer->setInterval (250);
	_reinitialize_timer->setSingleShot (true);
	QObject::connect (_reinitialize_timer, SIGNAL (timeout()), this, SLOT (initialize()));

	_scan_timer = new QTimer (this);
	_scan_timer->setInterval ((1.0 / _scan_frequency).ms());
	_scan_timer->setSingleShot (false);
	QObject::connect (_scan_timer, SIGNAL (timeout()), this, SLOT (pool_keys()));
	_scan_timer->start();

	guard ([&] {
		initialize();
	});
}


void
HT16K33::initialize()
{
	guard ([&] {
		_i2c_device.write (SetupRegister | SetupOn);
		_i2c_device.write (RowIntRegister | RowIntRow);
	});
}


void
HT16K33::reinitialize()
{
	for (auto sw: _switches)
		sw->invalidate();

	_reinitialize_timer->start();
}


void
HT16K33::pool_keys()
{
	guard ([&] {
		// Check for interrupt flag:
		uint8_t interrupt_flag = _i2c_device.read_register (InterruptRegister);

		if (_reliable_mode && !interrupt_flag)
		{
			// In reliable-mode we expect at least one key to be hardwired to be pressed,
			// and therefore interrupt flag should always be != 0. If it's not,
			// then we should skip this reading, since it's invalid.
			return;
		}

		// Read key RAM:
		KeyMatrix::DataArray& key_array = _key_matrix.array();
		_i2c_device.read_register (KeyMatrixRegister, key_array);

		for (auto sw: _switches)
			sw->key_matrix_updated (_key_matrix);
	});
}


inline void
HT16K33::guard (std::function<void()> guarded_code)
{
	try {
		guarded_code();
	}
	catch (Xefis::IOError& e)
	{
		log() << "I/O error: " << e.message() << std::endl;
		reinitialize();
	}
	catch (...)
	{
		reinitialize();
	}
}


void
HT16K33::data_updated()
{
	guard ([&] {
		uint8_t display = 0;
		uint8_t brightness = 0;

		if (_enabled.read (true) == true)
			display |= DisplayOn;
		else
			display |= DisplayOff;

		if (_blinking.read (false) == false)
			display |= DisplayBlinkOff;
		else
		{
			int m = _blinking_mode.read (static_cast<int> (Blinking::Fast));
			if (m == Blinking::Fast)
				display |= DisplayBlinkFast;
			else if (m == Blinking::Medium)
				display |= DisplayBlinkMedium;
			else
				display |= DisplayBlinkSlow;
		}

		brightness = Xefis::limit<int> (_brightness.read (15), 0, 15) & 0xf;

		_i2c_device.write (DisplayRegister | display);
		_i2c_device.write (BrightnessRegister | brightness);

		_led_matrix.clear();
		for (auto display: _displays)
			display->update_led_matrix (_led_matrix);

		// Write LED configuration:
		LedMatrix::DataArray& led_array = _led_matrix.array();
		_i2c_device.write_register (LedMatrixRegister, led_array);
	});
}

