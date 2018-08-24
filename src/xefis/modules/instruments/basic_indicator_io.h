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

#ifndef XEFIS__MODULES__INSTRUMENTS__BASIC_INDICATOR_IO_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__BASIC_INDICATOR_IO_H__INCLUDED

// Standard:
#include <cstddef>
#include <optional>

// Boost:
#include <boost/format.hpp>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument.h>
#include <xefis/core/property.h>
#include <xefis/core/setting.h>


template<class Value>
	class BasicIndicatorIO: public xf::ModuleIO
	{
	  public:
		/**
		 * How the value should be printed as text.
		 */
		xf::Setting<boost::format>	format					{ this, "format", boost::format ("%1%") };

		/**
		 * Set precision. If provided, value will be converted to int, divided
		 * by n and the multipled by n again.
		 */
		xf::Setting<int32_t>		precision				{ this, "precision", xf::BasicSetting::Optional };

		xf::Setting<Value>			value_minimum			{ this, "value_minimum" };
		xf::Setting<Value>			value_minimum_critical	{ this, "value_minimum_critical", xf::BasicSetting::Optional };
		xf::Setting<Value>			value_minimum_warning	{ this, "value_minimum_warning", xf::BasicSetting::Optional };
		xf::Setting<Value>			value_maximum_warning	{ this, "value_maximum_warning", xf::BasicSetting::Optional };
		xf::Setting<Value>			value_maximum_critical	{ this, "value_maximum_critical", xf::BasicSetting::Optional };
		xf::Setting<Value>			value_maximum			{ this, "value_maximum" };
	};

#endif

