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

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/numeric.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/time_helper.h>

// Local:
#include "pca9685.h"


XEFIS_REGISTER_MODULE_CLASS ("io/pca9685", PCA9685)


constexpr Time			PCA9685::InitializationDelay;
constexpr unsigned int	PCA9685::Channels;
constexpr Frequency		PCA9685::InternalFrequency;


Time
PCA9685::Channel::compute_duty_cycle()
{
	xf::PropertyFloat::Type value = fallback_to_last_valid ? last_value : input_default;
	if (input.valid())
		value = *input;
	value = xf::limit (value, input_minimum, input_maximum);

	last_value = value;

	Time now = xf::TimeHelper::now();
	value = smoother.process (value, now - _last_computation_time);
	_last_computation_time = now;

	return 1_s * xf::renormalize (value, input_minimum, input_maximum, output_minimum.quantity<Second>(), output_maximum.quantity<Second>());
}


PCA9685::PCA9685 (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	xf::I2C::Bus::ID i2c_bus;
	xf::I2C::Address::ID i2c_address;

	// Settings:

	typedef xf::ConfigReader::SettingsParser::SettingsList SettingsList;

	SettingsList settings_list = {
		{ "i2c.bus", i2c_bus, true },
		{ "i2c.address", i2c_address, true },
		{ "output-period", _output_period, false },
	};

	for (std::size_t i = 0; i < Channels; ++i)
	{
		QString i_str = QString ("%1").arg (i);

		SettingsList channel_settings = {
			{ "channel." + i_str + ".input.default", _channels[i].input_default, false },
			{ "channel." + i_str + ".input.minimum", _channels[i].input_minimum, false },
			{ "channel." + i_str + ".input.maximum", _channels[i].input_maximum, false },
			{ "channel." + i_str + ".output.minimum", _channels[i].output_minimum, false },
			{ "channel." + i_str + ".output.maximum", _channels[i].output_maximum, false },
			{ "channel." + i_str + ".fallback-to-last-valid", _channels[i].fallback_to_last_valid, false },
			{ "channel." + i_str + ".smoothing", _channels[i].smoothing_time, false },
		};
		settings_list.insert (settings_list.end(), channel_settings.begin(), channel_settings.end());
	}

	parse_settings (config, settings_list);

	// Properties:

	xf::ConfigReader::PropertiesParser::PropertiesList properties_list;

	for (std::size_t i = 0; i < Channels; ++i)
	{
		QString i_str = QString ("%1").arg (i);
		properties_list.emplace_back ("channel." + i_str, _channels[i].input, false);
	}

	parse_properties (config, properties_list);

	_i2c_device.bus().set_bus_number (i2c_bus);
	_i2c_device.set_address (xf::I2C::Address (i2c_address));

	_initialization_timer = new QTimer (this);
	_initialization_timer->setInterval (InitializationDelay.quantity<Millisecond>());
	_initialization_timer->setSingleShot (true);
	QObject::connect (_initialization_timer, SIGNAL (timeout()), this, SLOT (initialize()));
	_initialization_timer->start();

	for (Channel& channel: _channels)
	{
		channel.last_value = channel.input_default;
		channel.smoother.set_smoothing_time (channel.smoothing_time);
		channel.smoother.reset (channel.last_value);
	}

	_serviceable.set_default (false);
}


void
PCA9685::data_updated()
{
	set_pwm_values();
}


void
PCA9685::initialize()
{
	guard ([&] {
		_i2c_device.open();

		log() << "Resetting PCA9685." << std::endl;
		_i2c_device.write_register (Register_Mode1, 0x00);
		_i2c_device.write_register (Register_Mode2, Mode2_OutTotemPole | Mode2_UpdateOnAck);

		// Set pre-scale value and thus set period time.
		// Need to go to sleep to change prescale value.
		uint8_t mode1_orig = _i2c_device.read_register (Register_Mode1) & ~Mode1_RestartEnabled;
		_i2c_device.write_register (Register_Mode1, mode1_orig | Mode1_Sleep);
		_i2c_device.write_register (Register_Prescale, calculate_pre_scale_register (1.0 / _output_period));
		_i2c_device.write_register (Register_Mode1, mode1_orig & ~Mode1_Sleep);
		// Need to sleep for max 500 µs while waiting for osc to restart.
		usleep (500);
		_i2c_device.write_register (Register_Mode1, mode1_orig | Mode1_RestartEnabled);

		_serviceable.write (true);

		set_pwm_values();
	});
}


void
PCA9685::reinitialize()
{
	_serviceable.write (false);
	_i2c_device.close();
	_initialization_timer->start();
}


void
PCA9685::set_pwm_values()
{
	guard ([&] {
		for (std::size_t ch = 0; ch < _channels.size(); ++ch)
		{
			Channel& channel = _channels[ch];
			if (channel.input.valid() && channel.input.fresh())
			{
				// Fill in all 4 PWM registers:
				auto ary = get_config_for_pwm (channel.compute_duty_cycle());
				for (uint8_t i = 0; i < ary.size(); ++i)
					_i2c_device.write_register (get_pwm_register (ch, static_cast<PWMRegister> (PWMRegister_First + i)), ary[i]);
			}
		}
	});
}


PCA9685::Register
PCA9685::get_pwm_register (unsigned int channel, PWMRegister pwm_register)
{
	return static_cast<Register> (Register_PWM0OnL + 4 * channel + pwm_register);
}


std::array<uint8_t, 4>
PCA9685::get_config_for_pwm (Time duty_cycle)
{
	float y_corr = 0.955;
	uint16_t on_time = 0;
	uint16_t off_time = 4095 * (duty_cycle / _output_period) / y_corr;

	on_time = xf::limit<uint16_t> (on_time, 0, 4095);
	off_time = xf::limit<uint16_t> (off_time, 0, 4095);

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
PCA9685::calculate_pre_scale_register (Frequency frequency)
{
	// Spec says: refresh_rate = EXTCLK / (4096 * (prescale + 1))
	return std::round (InternalFrequency / (4096.0 * frequency) - 1);
}


void
PCA9685::guard (std::function<void()> guarded_code)
{
	try {
		guarded_code();
	}
	catch (xf::IOError& e)
	{
		log() << "I/O error: " << e.message() << std::endl;
		reinitialize();
	}
	catch (...)
	{
		reinitialize();
	}
}

