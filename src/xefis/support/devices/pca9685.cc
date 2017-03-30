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

// System:
#include <unistd.h>

// Lib:
#include <boost/endian/conversion.hpp>

// Qt:
#include <QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/time_helper.h>

// Local:
#include "pca9685.h"


namespace xf {

constexpr si::Time		PCA9685::kInitializationDelay;
constexpr unsigned int	PCA9685::kChannels;
constexpr si::Frequency	PCA9685::kInternalFrequency;


PCA9685::PCA9685 (i2c::Device&& device, si::Time output_period, xf::Logger* logger):
	_i2c_device (std::move (device)),
	_output_period (output_period),
	_logger (logger)
{
	std::fill (_duty_cycles.begin(), _duty_cycles.end(), 0_ms);

	_initialization_timer = new QTimer (this);
	_initialization_timer->setInterval (kInitializationDelay.quantity<Millisecond>());
	_initialization_timer->setSingleShot (true);
	QObject::connect (_initialization_timer, SIGNAL (timeout()), this, SLOT (initialize()));
	_initialization_timer->start();
}


void
PCA9685::set_duty_cycle (std::size_t channel_id, si::Time duty_cycle)
{
	if (channel_id >= kChannels)
		throw std::out_of_range ("channel_id should be between 0 and kChannels");

	_duty_cycles[channel_id] = duty_cycle;
	update_chip();
}


void
PCA9685::initialize()
{
	guard ([&] {
		_i2c_device.open();

		if (_logger)
			*_logger << "Resetting PCA9685." << std::endl;

		_i2c_device.write_register (Register_Mode1, 0x00);
		_i2c_device.write_register (Register_Mode2, Mode2_OutTotemPole | Mode2_UpdateOnAck);

		// Set pre-scale value and thus set period time.
		// Need to go to sleep to change prescale value.
		uint8_t mode1_orig = _i2c_device.read_register (Register_Mode1) & ~Mode1_RestartEnabled;
		_i2c_device.write_register (Register_Mode1, mode1_orig | Mode1_Sleep);
		_i2c_device.write_register (Register_Prescale, calculate_prescale_register (1.0 / _output_period));
		_i2c_device.write_register (Register_Mode1, mode1_orig & ~Mode1_Sleep);
		// Need to sleep for max 500 µs while waiting for osc to restart.
		usleep (500);
		_i2c_device.write_register (Register_Mode1, mode1_orig | Mode1_RestartEnabled);

		_serviceable = true;

		update_chip();
	});
}


void
PCA9685::reinitialize()
{
	_serviceable = false;
	_i2c_device.close();
	_initialization_timer->start();
}


void
PCA9685::update_chip()
{
	guard ([&] {
		for (std::size_t ch = 0; ch < _duty_cycles.size(); ++ch)
		{
			// Fill in all 4 PWM registers:
			auto ary = get_config_for_pwm (_duty_cycles[ch]);

			for (uint8_t i = 0; i < ary.size(); ++i)
				_i2c_device.write_register (get_pwm_register (ch, static_cast<PWMRegister> (PWMRegister_First + i)), ary[i]);
		}
	});
}


PCA9685::Register
PCA9685::get_pwm_register (unsigned int channel, PWMRegister pwm_register)
{
	return static_cast<Register> (Register_PWM0OnL + 4 * channel + pwm_register);
}


std::array<uint8_t, 4>
PCA9685::get_config_for_pwm (si::Time duty_cycle)
{
	float y_corr = 0.955;
	uint16_t on_time = 0;
	uint16_t off_time = 4095 * (duty_cycle / _output_period) / y_corr;

	xf::clamp<uint16_t> (on_time, 0, 4095);
	xf::clamp<uint16_t> (off_time, 0, 4095);

	boost::endian::native_to_little (on_time);
	boost::endian::native_to_little (off_time);

	std::array<uint8_t, 4> result;
	result[0] = (on_time >> 0) & 0xff;
	result[1] = (on_time >> 8) & 0x0f;
	result[2] = (off_time >> 0) & 0xff;
	result[3] = (off_time >> 8) & 0x0f;

	return result;
}


uint8_t
PCA9685::calculate_prescale_register (si::Frequency frequency)
{
	// Spec says: refresh_rate = EXTCLK / (4096 * (prescale + 1))
	return std::round (kInternalFrequency / (4096.0 * frequency) - 1);
}


void
PCA9685::guard (std::function<void()> guarded_code)
{
	try {
		guarded_code();
	}
	catch (xf::IOError& e)
	{
		if (_logger)
			*_logger << "I/O error: " << e.message() << std::endl;

		reinitialize();
	}
	catch (...)
	{
		reinitialize();
	}
}

} // namespace xf

