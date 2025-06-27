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

// Local:
#include "ets_airspeed.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/numeric.h>
#include <neutrino/qt/qdom.h>
#include <neutrino/stdexcept.h>

// Lib:
#include <boost/endian/conversion.hpp>

// Standard:
#include <cstddef>


ETSAirspeed::ETSAirspeed (xf::ProcessingLoop& loop, nu::i2c::Device&& device, nu::Logger const& logger, std::string_view const instance):
	ETSAirspeedIO (loop, instance),
	_logger (logger.with_context (std::string (kLoggerScope) + "#" + instance)),
	_device (std::move (device))
{
	_calibration_data.reserve (kOffsetCalculationSamples);

	_io.serviceable = false;
	_io.airspeed_minimum = 10_kt;
	_io.airspeed_maximum = 290_kt;
}


void
ETSAirspeed::initialize()
{
	if (*_io.read_interval < 100_ms)
	{
		_logger << "The setting airspeed.read-invterval is too low, setting it to 100 ms." << std::endl;
		_io.read_interval = 100_ms;
	}

	_airspeed_smoother.set_smoothing_time (*_io.smoothing_time);

	_device_initialization_timer = new QTimer (this);
	_device_initialization_timer->setInterval (kInitializationDelay.in<si::Millisecond>());
	_device_initialization_timer->setSingleShot (true);
	QObject::connect (_device_initialization_timer, &QTimer::timeout, this, &ETSAirspeed::device_initialize);
	_device_initialization_timer->start();

	_periodic_read_timer = new QTimer (this);
	_periodic_read_timer->setTimerType (Qt::PreciseTimer);
	_periodic_read_timer->setInterval (_io.read_interval->in<si::Millisecond>());
	_periodic_read_timer->setSingleShot (false);
	QObject::connect (_periodic_read_timer, &QTimer::timeout, this, &ETSAirspeed::read);
}


void
ETSAirspeed::device_initialize()
{
	guard ([&]{
		_device.open();
		// Start gathering samples for computation of an offset:
		_periodic_read_timer->start();
	});
}


void
ETSAirspeed::reinitialize()
{
	_io.serviceable = false;
	_io.airspeed = xf::nil;
	_device.close();
	// Wait for module hardware initialization and try to read values again.
	// There's nothing else we can do.
	_device_initialization_timer->start();
}


void
ETSAirspeed::read()
{
	guard ([&]{
		uint16_t raw_value = _device.read_register<uint16_t> (kValueRegister);
		boost::endian::little_to_native (raw_value);

		if (!_io.serviceable.value_or (false))
			_io.serviceable = true;

		switch (_stage)
		{
			case Stage::Calibrating:
				if (_calibration_data.size() < kOffsetCalculationSamples)
					_calibration_data.push_back (raw_value);
				else
				{
					offset_collected();
					_stage = Stage::Running;
				}
				break;

			case Stage::Running:
				si::Velocity speed = 0_kt;
				if (raw_value >= _offset)
					speed = 1_mps * (kValueScale * std::sqrt (1.0f * (raw_value - _offset)));
				_io.airspeed = _airspeed_smoother (speed, *_io.read_interval);
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
	_logger << "Calculated raw offset: " << _offset << std::endl;

	// Limit offset:
	uint16_t saved_offset = _offset;
	nu::clamp_inplace (_offset, kRawValueMinimum, kRawValueMaximum);

	if (saved_offset != _offset)
		_logger << "Offset clipped to: " << _offset << std::endl;
}


void
ETSAirspeed::guard (std::function<void()> guarded_code)
{
	try {
		guarded_code();
	}
	catch (nu::IOError& e)
	{
		_logger << "I/O error: " << e.message() << std::endl;
		reinitialize();
	}
	catch (...)
	{
		reinitialize();
	}
}

