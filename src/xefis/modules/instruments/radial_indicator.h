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

#ifndef XEFIS__MODULES__INSTRUMENTS__RADIAL_INDICATOR_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__RADIAL_INDICATOR_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <optional>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/instrument.h>
#include <xefis/core/property.h>
#include <xefis/core/property_observer.h>
#include <xefis/support/instrument/instrument_support.h>

// Local:
#include "basic_indicator_io.h"


template<class Value>
	class RadialIndicatorIO: public BasicIndicatorIO<Value>
	{
	  public:
		/*
		 * Settings
		 */

		xf::Setting<float>		dial_scale	{ this, "dial_scale", 1.0f };

		/*
		 * Input
		 */

		xf::PropertyIn<Value>	value		{ this, "value" };
		xf::PropertyIn<Value>	target		{ this, "target" };
		xf::PropertyIn<Value>	reference	{ this, "reference" };
		xf::PropertyIn<Value>	automatic	{ this, "automatic" };
	};


class BasicRadialIndicator: private xf::InstrumentSupport
{
  private:
	struct PointInfo
	{
		enum Zone {
			Minimums,
			Normal,
			Maximums,
		};

		Zone		zone;
		si::Angle	angle;
		QPen		pen;
		float		tick_len;
	};

  protected:
	struct IndicatorValues
	{
		boost::format				format;
		std::optional<std::string>	value_str;
		std::optional<std::string>	reference_str;
		std::optional<double>		normalized_value;
		std::optional<double>		normalized_reference;
		std::optional<double>		normalized_target;
		std::optional<double>		normalized_automatic;
		std::optional<double>		normalized_minimum_critical;
		std::optional<double>		normalized_minimum_warning;
		std::optional<double>		normalized_maximum_warning;
		std::optional<double>		normalized_maximum_critical;
		bool						critical_condition		{ false };
		bool						warning_condition		{ false };
		float						dial_scale				{ 1.f };

		// Cached struff, to prevent allocation on heap:
		std::vector<PointInfo>		point_infos;
	};

  protected:
	void
	paint (xf::PaintRequest&, IndicatorValues& values) const;

  private:
	void
	paint_text (IndicatorValues& values, xf::PaintRequest const&, xf::InstrumentAids&, xf::InstrumentPainter&, float q) const;

	void
	paint_indicator (IndicatorValues& values, xf::InstrumentAids&, xf::InstrumentPainter&, float q, float r) const;

  private:
	std::optional<float> _box_text_width;
};


template<class Value>
	class RadialIndicator:
		public xf::Instrument<RadialIndicatorIO<Value>>,
		private BasicRadialIndicator
	{
	  public:
		using Converter = std::function<float128_t (xf::Property<Value> const&)>;

	  public:
		// Ctor
		explicit
		RadialIndicator (std::unique_ptr<RadialIndicatorIO<Value>>, Converter = nullptr, std::string const& instance = {});

		// Module API
		void
		process (xf::Cycle const&) override;

		// Instrument API
		void
		paint (xf::PaintRequest&) const override;

	  private:
		xf::PropertyObserver	_inputs_observer;
		Converter				_converter;
	};


template<class Value>
	inline
	RadialIndicator<Value>::RadialIndicator (std::unique_ptr<RadialIndicatorIO<Value>> module_io, Converter converter, std::string const& instance):
		xf::Instrument<RadialIndicatorIO<Value>> (std::move (module_io), instance),
		_converter (converter)
	{
		_inputs_observer.set_callback ([&]{
			this->mark_dirty();
		});
		_inputs_observer.observe ({
			&this->io.value,
			&this->io.target,
			&this->io.reference,
			&this->io.automatic,
		});
	}


template<class Value>
	inline void
	RadialIndicator<Value>::process (xf::Cycle const& cycle)
	{
		_inputs_observer.process (cycle.update_time());
	}


template<class Value>
	inline void
	RadialIndicator<Value>::paint (xf::PaintRequest& paint_request) const
	{
		auto& io = this->io;
		xf::Range<Value> const range { *io.value_minimum, *io.value_maximum };
		constexpr xf::Range<double> kNormalizedRange { 0.0, 1.0 };
		IndicatorValues values;

		auto stringify = [&] (auto value) -> std::string {
			if (value)
			{
				if (this->io.precision)
				{
					auto const prec = *this->io.precision;
					value = std::llround (*value + 0.5f * prec) / prec * prec;
				}

				return (boost::format (*this->io.format) % *value).str();
			}
			else
				return "";
		};

		values.format = *this->io.format;
		values.dial_scale = *io.dial_scale;

		if (io.value)
		{
			values.value_str = stringify (_converter ? _converter (io.value) : io.value.to_floating_point());
			values.normalized_value = xf::renormalize (xf::clamped (*io.value, range), range, kNormalizedRange);
		}

		if (io.reference)
		{
			values.reference_str = stringify (_converter ? _converter (io.reference) : io.reference.to_floating_point());
			values.normalized_reference = xf::renormalize (xf::clamped (*io.reference, range), range, kNormalizedRange);
		}

		if (io.target)
			values.normalized_target = xf::renormalize (xf::clamped (*io.target, range), range, kNormalizedRange);

		if (io.automatic)
			values.normalized_automatic = xf::renormalize (xf::clamped (*io.automatic, range), range, kNormalizedRange);

		if (io.value_minimum_critical)
			values.normalized_minimum_critical = xf::renormalize (*io.value_minimum_critical, range, kNormalizedRange);

		if (io.value_minimum_warning)
			values.normalized_minimum_warning = xf::renormalize (*io.value_minimum_warning, range, kNormalizedRange);

		if (io.value_maximum_warning)
			values.normalized_maximum_warning = xf::renormalize (*io.value_maximum_warning, range, kNormalizedRange);

		if (io.value_maximum_critical)
			values.normalized_maximum_critical = xf::renormalize (*io.value_maximum_critical, range, kNormalizedRange);

		if (values.normalized_value)
		{
			values.critical_condition =
				(values.normalized_maximum_critical && *values.normalized_value >= *values.normalized_maximum_critical) ||
				(values.normalized_minimum_critical && *values.normalized_value <= *values.normalized_minimum_critical);
			values.warning_condition =
				(values.normalized_maximum_warning && *values.normalized_value >= *values.normalized_maximum_warning) ||
				(values.normalized_minimum_warning && *values.normalized_value <= *values.normalized_minimum_warning);
		}

		BasicRadialIndicator::paint (paint_request, values);
	}

#endif

