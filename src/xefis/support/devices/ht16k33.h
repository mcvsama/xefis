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

#ifndef XEFIS__SUPPORT__DEVICES__HT16K33_H__INCLUDED
#define XEFIS__SUPPORT__DEVICES__HT16K33_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>
#include <memory>
#include <functional>

// Qt:
#include <QtCore/QObject>
#include <QtCore/QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/v2/property.h>
#include <xefis/support/bus/i2c.h>
#include <xefis/utility/numeric.h>


namespace xf {

/**
 * This module interfaces Holtek's HT16K33 chip, for controlling LED displays and scanning keys/switches.
 */
class HT16K33: public QObject
{
	Q_OBJECT

	// I²C communication constants:
	static constexpr uint8_t kSetupRegister			= 0x20;
	static constexpr uint8_t kSetupOff				= 0x00;
	static constexpr uint8_t kSetupOn				= 0x01;
	static constexpr uint8_t kRowIntRegister		= 0xa0;
	static constexpr uint8_t kRowIntRow				= 0x00;
	static constexpr uint8_t kRowIntActiveL			= 0x01;
	static constexpr uint8_t kRowIntActiveH			= 0x03;
	static constexpr uint8_t kDisplayRegister		= 0x80;
	static constexpr uint8_t kDisplayOff			= 0x00;
	static constexpr uint8_t kDisplayOn				= 0x01;
	static constexpr uint8_t kDisplayBlinkOff		= 0x00;
	static constexpr uint8_t kDisplayBlinkSlow		= 0x07;
	static constexpr uint8_t kDisplayBlinkMedium	= 0x05;
	static constexpr uint8_t kDisplayBlinkFast		= 0x03;
	// Lower 4 bits indicate brightness (16 steps):
	static constexpr uint8_t kBrightnessRegister	= 0xe0;
	static constexpr uint8_t kLEDMatrixRegister		= 0x00;
	static constexpr uint8_t kInterruptRegister		= 0x60;
	static constexpr uint8_t kKeyMatrixRegister		= 0x40;

  public:
	typedef uint8_t	Row;
	typedef uint8_t	Column;

	static constexpr uint8_t kMaxBrightness = 15;

	enum BlinkingMode
	{
		Fast	= 0,
		Medium	= 1,
		Slow	= 2,
	};

	/**
	 * Manages display RAM of HT16K33 chip.
	 */
	class LEDMatrix
	{
	  public:
		// First byte is a command byte (display address pointer):
		typedef std::array<uint8_t, 16> DataArray;

		static constexpr Row	kMinRow		= 0;
		static constexpr Row	kMaxRow		= 15;
		static constexpr Column	kMinColumn	= 0;
		static constexpr Column	kMaxColumn	= 7;

	  public:
		// Ctor
		LEDMatrix();

		/**
		 * Reset all bits to 0.
		 */
		void
		clear();

		/**
		 * Turn on particular LED.
		 * \param	row Row (ROW) number [kMinRow..kMaxRow].
		 * \param	column Column (COM) number [kMinColumn..kMaxColumn].
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
		DataArray const&
		array() const noexcept;

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

		static constexpr Row	kMinRow		= 3;
		static constexpr Row	kMaxRow		= 15;
		static constexpr Column	kMinColumn	= 1;
		static constexpr Column	kMaxColumn	= 3;

	  public:
		// Ctor
		KeyMatrix();

		/**
		 * Reset all bits to 0.
		 */
		void
		clear();

		/**
		 * Read particular key state.
		 * \param	row Row (ROW) number [kMinRow..kMaxRow].
		 * \param	column Column (COM) number [kMinColumn..kMaxColumn].
		 *			Also names KS0..KS2.
		 */
		bool
		get (Row row, Column column) const noexcept;

		/**
		 * Return key RAM array, to be used
		 * to read data over I²C.
		 */
		DataArray&
		array() noexcept;

	  private:
		DataArray _data_array;
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
	 * Base class for LED output managers.
	 */
	class Display
	{
	  public:
		/**
		 * Set LEDMatrix bits according to configured
		 * digits and the value read from properties.
		 */
		virtual void
		update_led_matrix (LEDMatrix&) const = 0;
	};

	/**
	 * Single on/off switch that manages a boolean property.
	 */
	class SingleSwitch: public Switch
	{
	  public:
		// Ctor
		SingleSwitch (v2::Property<bool>&, Row, Column);

		bool
		key_matrix_updated (KeyMatrix const&) override;

		void
		invalidate() override;

	  private:
		v2::Property<bool>&	_property;
		Row					_row;
		Column				_column;
	};

	/**
	 * Handles single LEDs.
	 * Reads input from boolean properties.
	 */
	class SingleLED: public Display
	{
	  public:
		// Ctor
		SingleLED (v2::Property<bool>&, Row row, Column);

		void
		update_led_matrix (LEDMatrix&) const override;

	  private:
		v2::Property<bool>&	_property;
		Row					_row;
		Column				_column;
	};

	/**
	 * Handles array of 7-segment displays.
	 * Reads input from float or int property.
	 *
	 * \param	pValue
	 *			Value held by a Property.
	 * \param	pUnit
	 *			SI unit from si::units:: used when reading si Value.
	 */
	template<class pValue, class pUnit>
		class NumericDisplay: public Display
		{
		  public:
			typedef pValue	Value;
			typedef pUnit	Unit;

			// First element is least significant digit:
			typedef std::vector<Row> DigitRows;

		  public:
			// Ctor
			NumericDisplay (v2::Property<Value>&, DigitRows, bool rounding = false);

			void
			update_led_matrix (LEDMatrix&) const override;

		  private:
			// TODO handle floats
			int64_t
			get_integer_value() const;

		  private:
			v2::Property<Value>&	_property;
			DigitRows				_digit_rows;
			bool					_rounding;
		};

  private:
	typedef std::vector<Unique<Display>>	Displays;
	typedef std::vector<Unique<Switch>>		Switches;

  public:
	// Ctor
	explicit
	HT16K33 (i2c::Device&&, xf::Logger* = nullptr);

	/**
	 * Turn on/off displays and LEDs.
	 */
	void
	set_displays_enabled (bool enabled);

	/**
	 * Set displays brightness. Valid values are 0..kMaxBrightness, inclusive.
	 */
	void
	set_brightness (uint8_t brightness);

	/**
	 * Set displays brightness. Value range is 0.0…1.0, inclusive.
	 */
	void
	set_brightness (float brightness);

	/**
	 * Enable/disable blinking.
	 */
	void
	set_blinking (bool blinking);

	/**
	 * Set blinking mode of displays and LEDs.
	 */
	void
	set_blinking_mode (BlinkingMode);

	/**
	 * Set key-scanning frequency. Limited to 0..50 Hz, or 0..25 Hz in reliable-mode.
	 */
	void
	set_keyscan_frequency (si::Frequency);

	/**
	 * Enable reliable-mode.
	 * It gives better reliablility when scanning keys, but the scan frequency is automatically limited to max 25 Hz.
	 */
	void
	set_reliable_mode (bool enabled);

	/**
	 * Add a switch handler.
	 */
	template<class ...Arg>
		void
		add_single_switch (Arg&& ...args);

	/**
	 * Add a single LED handler.
	 */
	template<class ...Arg>
		void
		add_single_led (Arg&& ...args);

	/**
	 * Add a 7-segment LED display handler.
	 */
	template<class Value, class Unit, class ...Arg>
		void
		add_numeric_display (Arg&& ...args);

	// TODO options for API user: manual synchronization or automatic synchronization
	/**
	 * Synchronize with the chip - send new values to the chip and read key states from it.
	 */
	void
	update();

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
	update_timers();

  private:
	/**
	 * Digit symbols.
	 * Symbol number 10 is minus sign, 11 is dot.
	 * LSB is segment "a", MSB is dot:
	 */
	static constexpr std::array<uint8_t, 12> _digit_symbols {
		0x3f, // 0 |abcdef  |
		0x06, // 1 | bc     |
		0x5b, // 2 |ab de g |
		0x4f, // 3 |abcd  g |
		0x66, // 4 | bc  fg |
		0x6d, // 5 |a cd fg |
		0x7d, // 6 |a cdefg |
		0x07, // 7 |abc     |
		0x7f, // 8 |abcdefg |
		0x6f, // 9 |abcd fg |
		0x40, // - |      g |
		0x80, // . |       h|
	};

	static constexpr size_t kMinusSignIndex	= 10;
	static constexpr size_t kDotIndex		= 11;

	i2c::Device						_i2c_device;
	xf::Logger*						_logger;
	bool							_displays_enabled	= true;
	uint8_t							_brightness			= 16;
	bool							_blinking_enabled	= false;
	BlinkingMode					_blinking_mode		= BlinkingMode::Slow;
	si::Frequency					_scan_frequency		= 25_Hz;
	bool							_reliable_mode		= false;
	LEDMatrix						_led_matrix;
	KeyMatrix						_key_matrix;
	Displays						_displays;
	Switches						_switches;
	QTimer*							_reinitialize_timer	= nullptr;
	QTimer*							_scan_timer			= nullptr;
};


/*
 * Implementation
 */


template<class V, class U>
	HT16K33::NumericDisplay<V, U>::NumericDisplay (v2::Property<Value>& property, DigitRows digit_rows, bool rounding):
		_property (property),
		_digit_rows (digit_rows),
		_rounding (rounding)
	{
		for (auto row: _digit_rows)
			if (row < 0 || row > 15)
				throw Exception ("NumericDisplay: incorrect digit-rows configuration; max row index is 15");
	}


template<class V, class U>
	void
	HT16K33::NumericDisplay<V, U>::update_led_matrix (LEDMatrix& led_matrix) const
	{
		if (_digit_rows.empty())
			return;

		std::vector<uint8_t> digits;
		digits.reserve (16);

		int64_t integer = get_integer_value();

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
				led_matrix.set_column (_digit_rows.back(), _digit_symbols[kMinusSignIndex]);
			else if (digits.size() > _digit_rows.size())
			{
				set_all_digits_9();
				// Minus sign:
				led_matrix.set_column (_digit_rows.back(), _digit_symbols[kMinusSignIndex]);
			}
			else
			{
				clear_all_digits();
				normally_display();
				// Minus sign:
				led_matrix.set_column (_digit_rows[digits.size()-1], _digit_symbols[kMinusSignIndex]);
			}
		}
	}


template<class V, class U>
	int64_t
	HT16K33::NumericDisplay<V, U>::get_integer_value() const
	{
		auto value = si::quantity_in_units<Unit> (_property.value_or (Value{}));

		if (_rounding)
			return value + 0.5;
		else
			return value;
	}


inline void
HT16K33::set_displays_enabled (bool enabled)
{
	_displays_enabled = enabled;
	update();
}


inline void
HT16K33::set_brightness (uint8_t brightness)
{
	_brightness = brightness;
	update();
}


inline void
HT16K33::set_brightness (float brightness)
{
	set_brightness (static_cast<uint8_t> (16 * xf::clamped (brightness, 0.0f, 1.0f)));
}


inline void
HT16K33::set_blinking (bool enabled)
{
	_blinking_enabled = enabled;
	update();
}


inline void
HT16K33::set_blinking_mode (BlinkingMode blinking_mode)
{
	_blinking_mode = blinking_mode;
	update();
}


inline void
HT16K33::set_keyscan_frequency (si::Frequency frequency)
{
	_scan_frequency = frequency;
	update_timers();
}


inline void
HT16K33::set_reliable_mode (bool enabled)
{
	_reliable_mode = enabled;
	update_timers();
}


template<class ...Arg>
	inline void
	HT16K33::add_single_switch (Arg&& ...args)
	{
		_switches.push_back (std::move (std::make_unique<SingleSwitch> (std::forward<Arg> (args)...)));
	}


template<class ...Arg>
	inline void
	HT16K33::add_single_led (Arg&& ...args)
	{
		_displays.push_back (std::move (std::make_unique<SingleLED> (std::forward<Arg> (args)...)));
	}


template<class Value, class Unit, class ...Arg>
	inline void
	HT16K33::add_numeric_display (Arg&& ...args)
	{
		_displays.push_back (std::move (std::make_unique<NumericDisplay<Value, Unit>> (std::forward<Arg> (args)...)));
	}

} // namespace xf

#endif

