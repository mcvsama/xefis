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

// Standard:
#include <cstddef>

// Lib:
#include <boost/endian/conversion.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "ets_airspeed.h"


XEFIS_REGISTER_MODULE_CLASS ("io/ets-airspeed", ETSAirspeed);


ETSAirspeed::ETSAirspeed (Xefis::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	for (QDomElement& e: config)
	{
		if (e == "settings")
		{
			parse_settings (e, {
				{ "ias.read-interval", _ias_read_interval, true },
				{ "ias.smoothing-time", _ias_smoothing_time, true },
			});
		}
		if (e == "properties")
		{
			parse_properties (e, {
				{ "serviceable", _serviceable, true },
				{ "ias", _ias, true },
				{ "ias.minimum", _ias_minimum, false },
				{ "ias.maximum", _ias_maximum, false },
			});
		}
		else if (e == "i2c")
			parse_i2c (e, _i2c_bus, _i2c_address);
	}

	if (_ias_read_interval < 100_ms)
	{
		log() << "The setting ias.read-invterval is too low, setting it to 100 ms." << std::endl;
		_ias_read_interval = 100_ms;
	}

	_calibration_data.reserve (OffsetCalculationSamples);
	_ias_smoother.set_smoothing_time (_ias_smoothing_time);

	_initialization_timer = new QTimer (this);
	_initialization_timer->setInterval (InitializationDelay.ms());
	_initialization_timer->setSingleShot (true);
	QObject::connect (_initialization_timer, SIGNAL (timeout()), this, SLOT (initialize()));
	_initialization_timer->start();

	_periodic_read_timer = new QTimer (this);
	_periodic_read_timer->setInterval (_ias_read_interval.ms());
	_periodic_read_timer->setSingleShot (false);
	QObject::connect (_periodic_read_timer, SIGNAL (timeout()), this, SLOT (read()));

	_serviceable.set_default (false);
	_ias_minimum.set_default (10_kt);
	_ias_maximum.set_default (290_kt);
}


void
ETSAirspeed::initialize()
{
	guard ([&]() {
		_i2c_bus.open();
		// Start gathering samples for computation of an offset:
		_periodic_read_timer->start();
	});
}


void
ETSAirspeed::reinitialize()
{
	_serviceable.write (false);
	_ias.set_nil();
	_i2c_bus.close();
	// Wait for module hardware initialization and try to read values again.
	// There's nothing else we can do.
	_initialization_timer->start();

	signal_data_updated();
}


void
ETSAirspeed::read()
{
	bool updated = false;

	guard ([&]() {
		uint8_t reg = ValueRegister;
		uint16_t raw_value;
		uint8_t* c = reinterpret_cast<uint8_t*> (&raw_value);
		_i2c_bus.execute ({ Xefis::I2C::Message (Xefis::I2C::Write, _i2c_address, &reg),
							Xefis::I2C::Message (Xefis::I2C::Read, _i2c_address, c, c + 2) });
		boost::endian::little_to_native (raw_value);

		if (!_serviceable.read (false))
		{
			updated = true;
			_serviceable.write (true);
		}

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
				_ias.write (1_kt * _ias_smoother.process (speed.kt(), update_dt()));
				updated = true;
				break;
		}
	});

	if (updated)
		signal_data_updated();
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
	Xefis::limit (_offset, RawValueMinimum, RawValueMaximum);
	if (saved_offset != _offset)
		log() << "Offset clipped to: " << _offset << std::endl;
}


void
ETSAirspeed::guard (std::function<void()> guarded_code)
{
	try {
		guarded_code();
	}
	catch (Xefis::I2C::IOError& e)
	{
		log() << "I2C error: " << e.message() << std::endl;
		reinitialize();
	}
	catch (...)
	{
		reinitialize();
	}
}

