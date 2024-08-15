/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__MODULES__INSTRUMENTS__GAUGE_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__GAUGE_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/socket.h>

// Standard:
#include <cstddef>
#include <format>
#include <cmath>


class BasicGauge: public xf::Instrument
{
  public:
	using FloatingPoint = double;

  public:
	/**
	 * How the value should be printed as text.
	 */
	xf::Setting<std::format_string<FloatingPoint>>
									format					{ this, "format", std::format_string<FloatingPoint> ("{:f}") };

	/**
	 * Set precision. If provided, value will be converted to int, divided
	 * by n and the multipled by n again.
	 */
	xf::Setting<int32_t>			precision				{ this, "precision", xf::BasicSetting::Optional };

  public:
	using Instrument::Instrument;

  protected:
	/**
	 * Normalized and preprocessed data trasferred to the painting object.
	 */
	class GaugeValues
	{
	  public:
		std::format_string<FloatingPoint>	format					{ "{:f}" };
		std::optional<std::string>			value_str;
		std::optional<FloatingPoint>		normalized_value;
		std::optional<FloatingPoint>		normalized_minimum_critical;
		std::optional<FloatingPoint>		normalized_minimum_warning;
		std::optional<FloatingPoint>		normalized_maximum_warning;
		std::optional<FloatingPoint>		normalized_maximum_critical;
		bool								critical_condition		{ false };
		bool								warning_condition		{ false };

	  public:
		void
		get_from (auto const& io, auto const& range, std::optional<float128_t> value);
	};

  protected:
	static constexpr	xf::Range<FloatingPoint>	kNormalizedRange	{ 0.0f, 1.0f };
	static inline		QColor const				kSilver				{ 0xbb, 0xbd, 0xbf };
	static inline		QColor const				kWarningColor		{ 255, 200, 0 };
	static inline		QColor const				kCriticalColor		{ 255, 35, 35 };

  protected:
	static std::string
	stringify (auto value, std::format_string<FloatingPoint> const& format, xf::Setting<int32_t> const& precision);
};


template<class Value>
	class Gauge: public BasicGauge
	{
	  public:
		xf::Setting<Value>		value_minimum			{ this, "value_minimum" };
		xf::Setting<Value>		value_minimum_critical	{ this, "value_minimum_critical", xf::BasicSetting::Optional };
		xf::Setting<Value>		value_minimum_warning	{ this, "value_minimum_warning", xf::BasicSetting::Optional };
		xf::Setting<Value>		value_maximum_warning	{ this, "value_maximum_warning", xf::BasicSetting::Optional };
		xf::Setting<Value>		value_maximum_critical	{ this, "value_maximum_critical", xf::BasicSetting::Optional };
		xf::Setting<Value>		value_maximum			{ this, "value_maximum" };

	  public:
		using BasicGauge::BasicGauge;
	};


inline void
BasicGauge::GaugeValues::get_from (auto const& module, auto const& range, std::optional<float128_t> floating_point_value)
{
	this->format = *module.format;

	if (module.value)
	{
		value_str = BasicGauge::stringify (floating_point_value, *module.format, module.precision);
		normalized_value = xf::renormalize (xf::clamped (*module.value, range), range, kNormalizedRange);
	}

	if (module.value_minimum_critical)
		normalized_minimum_critical = xf::renormalize (*module.value_minimum_critical, range, kNormalizedRange);

	if (module.value_minimum_warning)
		normalized_minimum_warning = xf::renormalize (*module.value_minimum_warning, range, kNormalizedRange);

	if (module.value_maximum_warning)
		normalized_maximum_warning = xf::renormalize (*module.value_maximum_warning, range, kNormalizedRange);

	if (module.value_maximum_critical)
		normalized_maximum_critical = xf::renormalize (*module.value_maximum_critical, range, kNormalizedRange);

	if (normalized_value)
	{
		critical_condition =
			(normalized_maximum_critical && *normalized_value >= *normalized_maximum_critical) ||
			(normalized_minimum_critical && *normalized_value <= *normalized_minimum_critical);
		warning_condition =
			(normalized_maximum_warning && *normalized_value >= *normalized_maximum_warning) ||
			(normalized_minimum_warning && *normalized_value <= *normalized_minimum_warning);
	}
}


inline std::string
BasicGauge::stringify (auto value, std::format_string<FloatingPoint> const& format, xf::Setting<int32_t> const& precision)
{
	if (value)
	{
		if (precision)
		{
			auto const prec = *precision;
			value = std::llround (*value + 0.5f * prec) / prec * prec;
		}

		return std::format (format, static_cast<FloatingPoint> (*value));
	}
	else
		return {};
}

#endif

