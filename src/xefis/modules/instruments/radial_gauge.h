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

#ifndef XEFIS__MODULES__INSTRUMENTS__RADIAL_GAUGE_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__RADIAL_GAUGE_H__INCLUDED

// Standard:
#include <cstddef>
#include <string>
#include <optional>

// Neutrino:
#include <neutrino/synchronized.h>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/graphics.h>
#include <xefis/core/instrument.h>
#include <xefis/core/sockets/socket.h>
#include <xefis/support/instrument/instrument_support.h>
#include <xefis/support/sockets/socket_observer.h>

// Local:
#include "basic_gauge.h"


namespace si = neutrino::si;


// TODO handle nans
template<class Value>
	class RadialGaugeIO: public BasicGaugeIO<Value>
	{
	  public:
		/*
		 * Settings
		 */

		xf::Setting<float>	dial_scale	{ this, "dial_scale", 1.0f };

		/*
		 * Input
		 */

		xf::ModuleIn<Value>	value		{ this, "value" };
		xf::ModuleIn<Value>	target		{ this, "target" };
		xf::ModuleIn<Value>	reference	{ this, "reference" };
		xf::ModuleIn<Value>	automatic	{ this, "automatic" };
	};


class BasicRadialGauge:
	protected BasicGauge,
	protected xf::InstrumentSupport
{
  protected:
	/**
	 * Critical points (ie. minimum/maximum warning/critical values.
	 */
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

	struct GaugeValues: public BasicGauge::GaugeValues
	{
		std::optional<std::string>	reference_str;
		std::optional<float>		normalized_reference;
		std::optional<float>		normalized_target;
		std::optional<float>		normalized_automatic;
		float						dial_scale				{ 1.f };
	};

  protected:
	// Ctor
	explicit
	BasicRadialGauge (xf::Graphics const&);

	void
	async_paint (xf::PaintRequest const&, GaugeValues const&) const;

  private:
	void
	paint_text (GaugeValues const&, xf::PaintRequest const&, xf::InstrumentAids&, xf::InstrumentPainter&, float q) const;

	void
	paint_indicator (GaugeValues const&, xf::InstrumentAids&, xf::InstrumentPainter&, float r) const;

  private:
	// Cached struff, to prevent allocation on heap on every repaint:
	xf::Synchronized<std::optional<float>> mutable		_box_text_width;
	xf::Synchronized<std::vector<PointInfo>> mutable	_point_infos;
};


template<class Value>
	class RadialGauge:
		public xf::Instrument<RadialGaugeIO<Value>>,
		private BasicRadialGauge
	{
	  public:
		using Converter = std::function<float128_t (Value const&)>;

	  public:
		// Ctor
		explicit
		RadialGauge (std::unique_ptr<RadialGaugeIO<Value>>, xf::Graphics const&, Converter = nullptr, std::string_view const& instance = {});

		// Module API
		void
		process (xf::Cycle const&) override;

		// Instrument API
		std::packaged_task<void()>
		paint (xf::PaintRequest) const override;

	  private:
		xf::SocketObserver	_inputs_observer;
		Converter			_converter;
	};


template<class Value>
	inline RadialGauge<Value>::RadialGauge (std::unique_ptr<RadialGaugeIO<Value>> module_io, xf::Graphics const& graphics, Converter converter, std::string_view const& instance):
		xf::Instrument<RadialGaugeIO<Value>> (std::move (module_io), instance),
		BasicRadialGauge (graphics),
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
	RadialGauge<Value>::process (xf::Cycle const& cycle)
	{
		_inputs_observer.process (cycle.update_time());
	}


template<class Value>
	inline std::packaged_task<void()>
	RadialGauge<Value>::paint (xf::PaintRequest paint_request) const
	{
		auto& io = this->io;
		xf::Range const range { *io.value_minimum, *io.value_maximum };

		GaugeValues values;
		values.get_from (io, range, (_converter && io.value) ? _converter (*io.value) : io.value.to_floating_point());
		values.dial_scale = *io.dial_scale;

		if (io.reference)
		{
			auto v = (_converter && io.reference) ? _converter (*io.reference) : io.reference.to_floating_point();
			values.reference_str = BasicGauge::stringify (v, *io.format, io.precision);
			values.normalized_reference = xf::renormalize (xf::clamped (*io.reference, range), range, kNormalizedRange);
		}

		if (io.target)
			values.normalized_target = xf::renormalize (xf::clamped (*io.target, range), range, kNormalizedRange);

		if (io.automatic)
			values.normalized_automatic = xf::renormalize (xf::clamped (*io.automatic, range), range, kNormalizedRange);

		return std::packaged_task<void()> ([this, pr = std::move (paint_request), gv = std::move (values)] {
			async_paint (pr, gv);
		});
	}

#endif

