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

// Lib:
#include <boost/endian/conversion.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/stdexcept.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "ets_airspeed.h"


XEFIS_REGISTER_MODULE_CLASS ("io/ets-airspeed", ETSAirspeed);


constexpr uint8_t		ETSAirspeed::ValueRegister;
constexpr float			ETSAirspeed::ValueScale;
constexpr Time			ETSAirspeed::InitializationDelay;
constexpr unsigned int	ETSAirspeed::OffsetCalculationSamples;
constexpr uint16_t		ETSAirspeed::RawValueMinimum;
constexpr uint16_t		ETSAirspeed::RawValueMaximum;


ETSAirspeed::ETSAirspeed (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	xf::I2C::Bus::ID i2c_bus;
	xf::I2C::Address::ID i2c_address;

	parse_settings (config, {
		{ "i2c.bus", i2c_bus, true },
		{ "i2c.address", i2c_address, true },
		{ "airspeed.read-interval", _airspeed_read_interval, true },
		{ "airspeed.smoothing-time", _airspeed_smoothing_time, true },
	});

	parse_properties (config, {
		{ "serviceable", _serviceable, true },
		{ "airspeed", _airspeed, true },
		{ "airspeed.minimum", _airspeed_minimum, false },
		{ "airspeed.maximum", _airspeed_maximum, false },
	});

	_i2c_device.bus().set_bus_number (i2c_bus);
	_i2c_device.set_address (xf::I2C::Address (i2c_address));

	if (_airspeed_read_interval < 100_ms)
	{
		log() << "The setting airspeed.read-invterval is too low, setting it to 100 ms." << std::endl;
		_airspeed_read_interval = 100_ms;
	}

	_calibration_data.reserve (OffsetCalculationSamples);
	_airspeed_smoother.set_smoothing_time (_airspeed_smoothing_time);

	_initialization_timer = new QTimer (this);
	_initialization_timer->setInterval (InitializationDelay.ms());
	_initialization_timer->setSingleShot (true);
	QObject::connect (_initialization_timer, SIGNAL (timeout()), this, SLOT (initialize()));
	_initialization_timer->start();

	_periodic_read_timer = new QTimer (this);
	_periodic_read_timer->setInterval (_airspeed_read_interval.ms());
	_periodic_read_timer->setSingleShot (false);
	QObject::connect (_periodic_read_timer, SIGNAL (timeout()), this, SLOT (read()));

	_serviceable.set_default (false);
	_airspeed_minimum.set_default (10_kt);
	_airspeed_maximum.set_default (290_kt);
}


void
ETSAirspeed::initialize()
{
	guard ([&] {
		_i2c_device.open();
		// Start gathering samples for computation of an offset:
		_periodic_read_timer->start();
	});
}


void
ETSAirspeed::reinitialize()
{
	_serviceable.write (false);
	_airspeed.set_nil();
	_i2c_device.close();
	// Wait for module hardware initialization and try to read values again.
	// There's nothing else we can do.
	_initialization_timer->start();
}


void
ETSAirspeed::read()
{
	guard ([&] {
		uint16_t raw_value = _i2c_device.read_register<uint16_t> (ValueRegister);
		boost::endian::little_to_native (raw_value);

		if (!_serviceable.read (false))
			_serviceable.write (true);

		switch (_stage)
		{
			case Stage::Calibrating:
				if (_calibration_data.size() < OffsetCalculationSamples)
					_calibration_data.push_back (raw_value);
				else
				{
					offset_collected();
					_stage = Stage::Running;
				}
				break;

			case Stage::Running:
				// Convert raw_value to m/s:
				Speed speed = 0_kt;
				if (raw_value >= _offset)
					speed = 1_mps * (ValueScale * std::sqrt (1.0f * (raw_value - _offset)));
				_airspeed.write (1_kt * _airspeed_smoother.process (speed.kt(), _airspeed_read_interval));
				break;
		}
	});
}


void
ETSAirspeed::offset_collected()
{
	// Drop 25% lowest and 25% highest samples:
	std::sort (_calibration_data.begin(), _calibration_data.end());
	if (_calibration_data.size() > 10)
	{
		int drop_samples = 0.25 * _calibration_data.size();
		_calibration_data.erase (_calibration_data.begin(), _calibration_data.begin() + drop_samples);
		_calibration_data.erase (_calibration_data.end() - drop_samples, _calibration_data.end());
	}

	// Average:
	uint32_t sum = 0;
	for (auto s: _calibration_data)
		sum += s;
	_offset = std::round (sum / _calibration_data.size());
	log() << "Calculated raw offset: " << _offset << std::endl;

	// Limit offset:
	uint16_t saved_offset = _offset;
	xf::limit (_offset, RawValueMinimum, RawValueMaximum);
	if (saved_offset != _offset)
		log() << "Offset clipped to: " << _offset << std::endl;
}


void
ETSAirspeed::guard (std::function<void()> guarded_code)
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

