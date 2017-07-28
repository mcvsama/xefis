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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "ht16k33.h"


namespace xf {

constexpr std::array<uint8_t, 12> HT16K33::_digit_symbols;


HT16K33::LEDMatrix::LEDMatrix()
{
	clear();
}


void
HT16K33::LEDMatrix::clear()
{
	_data_array.fill (0);
}


void
HT16K33::LEDMatrix::set (Row row, Column column, bool value) noexcept
{
	uint8_t byte = 2 * column + (row < 8 ? 0 : 1);
	uint8_t bit = row % 8;
	if (value)
		_data_array[byte] |= 1 << bit;
	else
		_data_array[byte] &= ~(1 << bit);
}


void
HT16K33::LEDMatrix::set_column (Row row, uint8_t column_bits) noexcept
{
	for (unsigned int i = 0; i < 8; ++i)
		set (row, i, (column_bits >> i) & 1);
}


HT16K33::LEDMatrix::DataArray const&
HT16K33::LEDMatrix::array() const noexcept
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
HT16K33::KeyMatrix::array() noexcept
{
	return _data_array;
}


HT16K33::SingleLED::SingleLED (v2::Property<bool>& property, Row row, Column column):
	_property (property),
	_row (row),
	_column (column)
{
	xf::clamp (_row, LEDMatrix::kMinRow, LEDMatrix::kMaxRow);
	xf::clamp (_column, LEDMatrix::kMinColumn, LEDMatrix::kMaxColumn);
}


inline void
HT16K33::SingleLED::update_led_matrix (LEDMatrix& led_matrix) const
{
	led_matrix.set (_row, _column, _property.value_or (false));
}


HT16K33::SingleSwitch::SingleSwitch (v2::Property<bool>& property, Row row, Column column):
	_property (property),
	_row (row),
	_column (column)
{
	xf::clamp (_row, KeyMatrix::kMinRow, KeyMatrix::kMaxRow);
	xf::clamp (_column, KeyMatrix::kMinColumn, KeyMatrix::kMaxColumn);
}


bool
HT16K33::SingleSwitch::key_matrix_updated (KeyMatrix const& key_matrix)
{
	bool prev_value = _property.value_or (false);
	bool next_value = key_matrix.get (_row, _column);
	_property = next_value;

	return prev_value != next_value;
}


void
HT16K33::SingleSwitch::invalidate()
{
	_property.set_nil();
}


HT16K33::HT16K33 (i2c::Device&& i2c_device, Logger* logger):
	_i2c_device (std::move (i2c_device)),
	_logger (logger)
{
	_reinitialize_timer = new QTimer (this);
	_reinitialize_timer->setInterval (250);
	_reinitialize_timer->setSingleShot (true);
	QObject::connect (_reinitialize_timer, SIGNAL (timeout()), this, SLOT (initialize()));

	_scan_timer = new QTimer (this);
	_scan_timer->setSingleShot (false);
	QObject::connect (_scan_timer, SIGNAL (timeout()), this, SLOT (pool_keys()));

	update_timers();

	guard ([&] {
		initialize();
	});
}


void
HT16K33::update()
{
	guard ([&] {
		uint8_t display_bits = 0;

		if (_displays_enabled)
			display_bits |= kDisplayOn;
		else
			display_bits |= kDisplayOff;

		if (_blinking_enabled)
		{
			switch (_blinking_mode)
			{
				case BlinkingMode::Fast:
					display_bits |= kDisplayBlinkFast;
					break;

				case BlinkingMode::Medium:
					display_bits |= kDisplayBlinkMedium;
					break;

				case BlinkingMode::Slow:
					display_bits |= kDisplayBlinkSlow;
					break;
			}
		}
		else
			display_bits |= kDisplayBlinkOff;

		_i2c_device.write (kDisplayRegister | display_bits);

		uint8_t brightness = xf::clamped<uint8_t> (_brightness, 0, kMaxBrightness);

		_i2c_device.write (kBrightnessRegister | brightness);

		_led_matrix.clear();
		for (auto& display: _displays)
			display->update_led_matrix (_led_matrix);

		_i2c_device.write_register (kLEDMatrixRegister, _led_matrix.array());
	});
}


void
HT16K33::initialize()
{
	guard ([&] {
		_i2c_device.write (kSetupRegister | kSetupOn);
		_i2c_device.write (kRowIntRegister | kRowIntRow);
	});
}


void
HT16K33::reinitialize()
{
	for (auto& sw: _switches)
		sw->invalidate();

	_reinitialize_timer->start();
}


void
HT16K33::pool_keys()
{
	guard ([&] {
		// Check for interrupt flag:
		uint8_t interrupt_flag = _i2c_device.read_register (kInterruptRegister);

		if (_reliable_mode && !interrupt_flag)
		{
			// In reliable-mode we expect at least one key to be hardwired to be pressed,
			// and therefore interrupt flag should always be != 0. If it's not,
			// then we should skip this reading, since it's invalid.
			return;
		}

		// Read key RAM:
		_i2c_device.read_register (kKeyMatrixRegister, _key_matrix.array());

		for (auto& sw: _switches)
			sw->key_matrix_updated (_key_matrix);
	});
}


void
HT16K33::guard (std::function<void()> guarded_code)
{
	try {
		guarded_code();
	}
	catch (xf::IOError& e)
	{
		if (_logger)
			(*_logger) << "I/O error: " << e.message() << std::endl;
		reinitialize();
	}
	catch (...)
	{
		reinitialize();
	}
}


void
HT16K33::update_timers()
{
	// According to docs, each scan takes 20 ms, so limit sampling rate to 50 Hz:
	auto scan_frequency = xf::clamped (_scan_frequency, 0_Hz, _reliable_mode ? 25_Hz : 50_Hz);
	_scan_timer->setInterval ((1.0 / scan_frequency).quantity<Millisecond>());
	_scan_timer->start();
}

} // namespace xf

