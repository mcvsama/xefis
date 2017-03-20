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

#ifndef XEFIS__MODULES__IO__PCA9685_H__INCLUDED
#define XEFIS__MODULES__IO__PCA9685_H__INCLUDED

// Standard:
#include <cstddef>
#include <array>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/module.h>
#include <xefis/core/property.h>
#include <xefis/support/bus/i2c.h>
#include <xefis/utility/smoother.h>


/**
 * Warning: this module uses I2C I/O in main thread, which may block.
 *
 * Handles PCA9685-based Adafruit's 16-channel 12-bit PWM controller.
 */
class PCA9685:
	public QObject,
	public xf::Module
{
	Q_OBJECT

	static constexpr Time			InitializationDelay	= 0.1_s;
	static constexpr unsigned int	Channels			= 16;
	static constexpr Frequency		InternalFrequency	= 25_MHz;

	enum Register: uint8_t
	{
		Register_Mode1			= 0x00,
		Register_Mode2			= 0x01,
		Register_SubAddress1	= 0x02,
		Register_SubAddress2	= 0x03,
		Register_SubAddress3	= 0x04,
		Register_AllCallAddr	= 0x05,
		Register_PWM0OnL		= 0x06,
		Register_PWM0OnH		= 0x07,
		Register_PWM0OffL		= 0x08,
		Register_PWM0OffH		= 0x09,
		Register_Prescale		= 0xfe,
	};

	enum PWMRegister: uint8_t
	{
		PWMRegister_First		= 0x00,
		PWMRegister_OnL			= 0x00,
		PWMRegister_OnH			= 0x01,
		PWMRegister_OffL		= 0x02,
		PWMRegister_OffH		= 0x03,
	};

	enum Mode1: uint8_t
	{
		Mode1_AllCallEnabled	= 1u << 0,
		Mode1_Sub3AddrEnabled	= 1u << 1,
		Mode1_Sub2AddrEnabled	= 1u << 2,
		Mode1_Sub1AddrEnabled	= 1u << 3,
		Mode1_Sleep				= 1u << 4,
		Mode1_AutoIncrement		= 1u << 5,
		Mode1_UseExtClock		= 1u << 6,
		Mode1_RestartEnabled	= 1u << 7,
	};

	enum Mode2: uint8_t
	{
		Mode2_OutNegated		= 1u << 0,
		Mode2_OutTotemPole		= 1u << 2,
		Mode2_UpdateOnAck		= 1u << 3,
		Mode2_Invert			= 1u << 4,
	};

	struct Channel
	{
		xf::PropertyFloat		input;
		xf::PropertyFloat::Type	input_default			= 0.0;
		xf::PropertyFloat::Type	input_minimum			= 0.0;
		xf::PropertyFloat::Type	input_maximum			= 1.0;
		xf::PropertyFloat::Type	last_value				= input_default;
		Time					output_minimum			= 1_ms;
		Time					output_maximum			= 2_ms;
		bool					fallback_to_last_valid	= false;
		Time					smoothing_time			= 1_ms;
		xf::Smoother<double>	smoother				{ smoothing_time };

		Time
		compute_duty_cycle();

	  private:
		Time						_last_computation_time	= 0_s;
	};

  public:
	// Ctor:
	PCA9685 (xf::ModuleManager*, QDomElement const& config);

	// Module API
	void
	data_updated() override;

  private slots:
	/**
	 * Initialize module.
	 */
	void
	initialize();

  private:
	/**
	 * Reinitialize after a failure.
	 */
	void
	reinitialize();

	/**
	 * Read values from properties and setup output.
	 */
	void
	set_pwm_values();

	/**
	 * Get register number for given channel and offset.
	 */
	Register
	get_pwm_register (unsigned int channel, PWMRegister pwm_register);

	/**
	 * Get array of bytes that should be written to given PWM registers
	 * for given duty cycle.
	 */
	std::array<uint8_t, 4>
	get_config_for_pwm (Time duty_cycle);

	/**
	 * Compute value to be put to the pre-scale register of the chip.
	 */
	static uint8_t
	calculate_pre_scale_register (Frequency frequency);

	/**
	 * Guard and reinitialize on I2C error.
	 */
	void
	guard (std::function<void()> guarded_code);

  private:
	xf::PropertyBoolean				_serviceable;
	std::array<Channel, Channels>	_channels;
	QTimer*							_initialization_timer;
	xf::i2c::Device					_i2c_device;
	Time							_output_period		= 20_ms;
};

#endif

