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

#ifndef XEFIS__MODULES__IO__HT16K33_H__INCLUDED
#define XEFIS__MODULES__IO__HT16K33_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>
#include <memory>
#include <functional>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/support/bus/i2c.h>


/**
 * This module interfaces Holtek's HT16K33 chip, for controlling
 * LED displays and scanning keys/switches.
 */
class HT16K33:
	public QObject,
	public xf::Module
{
	Q_OBJECT

	class Display;
	class Switch;

	typedef std::vector<Shared<Display>>	Displays;
	typedef std::vector<Shared<Switch>>		Switches;
	typedef uint8_t							Row;
	typedef uint8_t							Column;

	enum Blinking {
		Fast		= 0,
		Medium		= 1,
		Slow		= 2
	};

	// I²C communication constants.
	static constexpr uint8_t SetupRegister		= 0x20;
	static constexpr uint8_t SetupOff			= 0x00;
	static constexpr uint8_t SetupOn			= 0x01;
	static constexpr uint8_t RowIntRegister		= 0xa0;
	static constexpr uint8_t RowIntRow			= 0x00;
	static constexpr uint8_t RowIntActiveL		= 0x01;
	static constexpr uint8_t RowIntActiveH		= 0x03;
	static constexpr uint8_t DisplayRegister	= 0x80;
	static constexpr uint8_t DisplayOff			= 0x00;
	static constexpr uint8_t DisplayOn			= 0x01;
	static constexpr uint8_t DisplayBlinkOff	= 0x00;
	static constexpr uint8_t DisplayBlinkSlow	= 0x07;
	static constexpr uint8_t DisplayBlinkMedium	= 0x05;
	static constexpr uint8_t DisplayBlinkFast	= 0x03;
	// Lower 4 bits indicate brightness (16 steps):
	static constexpr uint8_t BrightnessRegister	= 0xe0;
	static constexpr uint8_t LedMatrixRegister	= 0x00;
	static constexpr uint8_t InterruptRegister	= 0x60;
	static constexpr uint8_t KeyMatrixRegister	= 0x40;

	/**
	 * Manages display RAM of HT16K33 chip.
	 */
	class LedMatrix
	{
	  public:
		// First byte is a command byte (display address pointer):
		typedef std::array<uint8_t, 16> DataArray;

	  public:
		// Ctor:
		LedMatrix();

		/**
		 * Reset all bits to 0.
		 */
		void
		clear();

		/**
		 * Turn on particular LED.
		 * \param	row Row (ROW) number [0..15].
		 * \param	column Column (COM) number [0..7].
		 * \param	value Bit value to set.
		 */
		void
		set (Row row, Column column, bool value) noexcept;

		/**
		 * Set whole column (COM outputs) at once.
		 */
		void
		set_column (Row row, uint8_t column_bits) noexcept;

		/**
		 * Return data to be sent over I²C.
		 */
		DataArray&
		array();

	  private:
		DataArray _data_array;
	};

	/**
	 * Manages key-scan RAM of HT16K33 chip.
	 */
	class KeyMatrix
	{
	  public:
		typedef std::array<uint8_t, 6> DataArray;

	  public:
		// Ctor:
		KeyMatrix();

		/**
		 * Reset all bits to 0.
		 */
		void
		clear();

		/**
		 * Read particular key state.
		 * \param	row Row (ROW) number [3..15].
		 * \param	column Column (COM) number [1..3].
		 *			Also names KS0..KS2.
		 */
		bool
		get (Row row, Column column) const noexcept;

		/**
		 * Return key RAM array, to be used
		 * to read data over I²C.
		 */
		DataArray&
		array();

	  private:
		DataArray _data_array;
	};

	/**
	 * Base class for LED output managers.
	 */
	class Display
	{
	  public:
		/**
		 * Set LedMatrix bits according to configured
		 * digits and the value read from properties.
		 */
		virtual void
		update_led_matrix (LedMatrix&) const = 0;
	};

	/**
	 * Handles single LEDs.
	 * Reads input from boolean properties.
	 */
	class SingleLed: public Display
	{
	  public:
		// Ctor:
		SingleLed (QDomElement const& element);

		void
		update_led_matrix (LedMatrix&) const override;

	  private:
		Row						_row = 0;
		Column					_column = 0;
		xf::PropertyBoolean	_property_boolean;
	};

	/**
	 * Handles array of 7-segment displays.
	 * Reads input from float or int property.
	 */
	class NumericDisplay: public Display
	{
		// First element is least significant digit:
		typedef std::vector<Row> DigitRows;

	  public:
		// Ctor:
		NumericDisplay (QDomElement const& element);

		void
		update_led_matrix (LedMatrix&) const override;

	  private:
		xf::PropertyInteger::Type
		get_integer_value() const;

	  private:
		bool				_rounding = false;
		std::string			_unit;
		DigitRows			_digit_rows;
		xf::GenericProperty	_property;
	};

	/**
	 * Base class for key reading interfaces.
	 */
	class Switch
	{
	  public:
		/**
		 * Read key values from the key memory
		 * and do appropriate configured actions.
		 * Return true if any property has been updated.
		 */
		virtual bool
		key_matrix_updated (KeyMatrix const&) = 0;

		/**
		 * Signal input failure to reset the property
		 * to nil-value.
		 */
		virtual void
		invalidate() = 0;
	};

	/**
	 * Single on/off switch that manages
	 * a boolean property.
	 */
	class SingleSwitch: public Switch
	{
	  public:
		SingleSwitch (QDomElement const& element);

		bool
		key_matrix_updated (KeyMatrix const&) override;

		void
		invalidate() override;

	  private:
		Row					_row = 0;
		Column				_column = 0;
		xf::PropertyBoolean	_property_boolean;
	};

  public:
	// Ctor:
	HT16K33 (xf::ModuleManager*, QDomElement const& config);

  private slots:
	void
	initialize();

	void
	reinitialize();

	void
	pool_keys();

  private:
	void
	guard (std::function<void()> guarded_code);

	void
	data_updated() override;

  private:
	/**
	 * Digit symbols.
	 * Symbol number 10 is "minus".
	 * LSB is segment "a", MSB is dot:
	 */
	static std::array<uint8_t, 11> _digit_symbols;

	xf::PropertyBoolean	_enabled;
	xf::PropertyInteger	_brightness;
	xf::PropertyBoolean	_blinking;
	xf::PropertyInteger	_blinking_mode;
	xf::I2C::Device		_i2c_device;
	LedMatrix			_led_matrix;
	KeyMatrix			_key_matrix;
	Displays			_displays;
	Switches			_switches;
	QTimer*				_reinitialize_timer	= nullptr;
	QTimer*				_scan_timer			= nullptr;
	Frequency			_scan_frequency		= 25_Hz;
	bool				_reliable_mode		= false;
};

#endif

