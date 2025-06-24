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

#ifndef XEFIS__MODULES__HELPERS__MIXER_H__INCLUDED
#define XEFIS__MODULES__HELPERS__MIXER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/logger.h>
#include <xefis/core/module.h>
#include <xefis/core/setting.h>
#include <xefis/core/socket.h>
#include <xefis/support/sockets/socket_value_changed.h>

// Standard:
#include <cstddef>


template<class Value>
	class MixerIO: public xf::Module
	{
	  public:
		/*
		 * Settings
		 */

		xf::Setting<double>		input_a_factor	{ this, "input_a_factor", 1.0 };
		xf::Setting<double>		input_b_factor	{ this, "input_b_factor", 1.0 };
		xf::Setting<Value>		output_minimum	{ this, "output_minimum", xf::BasicSetting::Optional };
		xf::Setting<Value>		output_maximum	{ this, "output_maximum", xf::BasicSetting::Optional };

		/*
		 * Input
		 */

		xf::ModuleIn<Value>		input_a_value	{ this, "input.a" };
		xf::ModuleIn<Value>		input_b_value	{ this, "input.b" };

		/*
		 * Output
		 */

		xf::ModuleOut<Value>	output_value	{ this, "value" };

	  public:
		using xf::Module::Module;
	};


template<class pValue>
	class Mixer: public MixerIO<pValue>
	{
	  private:
		static constexpr char kLoggerScope[] = "mod::Mixer";

	  public:
		using Value = pValue;

	  public:
		// Ctor
		explicit
		Mixer (nu::Logger const&, std::string_view const instance = {});

	  protected:
		// Module API
		void
		initialize() override;

		// Module API
		void
		process (xf::Cycle const&) override;

	  private:
		MixerIO&						_io					{ *this };
		nu::Logger						_logger;
		xf::SocketValueChanged<Value>	_input_a_changed	{ _io.input_a_value };
		xf::SocketValueChanged<Value>	_input_b_changed	{ _io.input_b_value };
	};


/*
 * Implementation
 */


template<class V>
	Mixer<V>::Mixer (nu::Logger const& logger, std::string const& instance):
		MixerIO<V> (instance),
		_logger (logger.with_context (std::string (kLoggerScope) + "#" + instance))
	{ }


template<class V>
	void
	Mixer<V>::initialize()
	{
		if (_io.setting_output_minimum && _io.setting_output_maximum && *_io.setting_output_minimum > *_io.setting_output_maximum)
			_logger << "Settings error: maximum value is less than the minimum value." << std::endl;
	}


template<class V>
	void
	Mixer<V>::process (xf::Cycle const&)
	{
		if (_input_a_changed.value_changed() || _input_b_changed.value_changed())
		{
			auto& a = input_a_value;
			auto& b = input_b_value;

			if (a || b)
			{
				Value sum{};

				if (a)
					sum += *_io.setting_input_a_factor * *a;
				if (b)
					sum += *_io.setting_input_b_factor * *b;

				if (_io.setting_output_minimum && sum < *_io.setting_output_minimum)
					sum = *setting_output_minimum;

				if (_io.setting_output_maximum && sum > *_io.setting_output_maximum)
					sum = *setting_output_maximum;

				output_value = sum;
			}
			else
				output_value = xf::nil;
		}
	}

#endif
