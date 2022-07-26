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

#ifndef XEFIS__MODULES__INSTRUMENTS__LINEAR_GAUGE_H__INCLUDED
#define XEFIS__MODULES__INSTRUMENTS__LINEAR_GAUGE_H__INCLUDED

// Local:
#include "gauge.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/graphics.h>
#include <xefis/core/instrument.h>
#include <xefis/core/setting.h>
#include <xefis/core/sockets/socket.h>
#include <xefis/support/instrument/instrument_support.h>
#include <xefis/support/sockets/socket_observer.h>

// Neutrino:
#include <neutrino/synchronized.h>

// Standard:
#include <cstddef>
#include <optional>


// TODO handle nans
template<class Value>
	class LinearGauge:
		public Gauge<Value>,
		private xf::InstrumentSupport
	{
	  public:
		/*
		 * Settings
		 */

		xf::Setting<bool>			mirrored_style	{ this, "mirrored_style", false };
		xf::Setting<bool>			line_hidden		{ this, "line_hidden", false };
		xf::Setting<float>			font_scale		{ this, "font_scale", 1.0f };
		xf::Setting<std::string>	note			{ this, "note", "" };

		/*
		 * Input
		 */

		xf::ModuleIn<Value>			value			{ this, "value" };

	  public:
		using Converter = std::function<float128_t (Value const&)>;

	  private:
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

			Zone	zone;
			float	length;
			QPen	pen;
			float	tick_len;
			bool	critical;
		};

		struct GaugeValues: public Gauge<Value>::GaugeValues
		{
			bool					mirrored_style;
			bool					line_hidden;
			float					font_scale;
			std::string				note_str;
			bool					inbound;
		};

	  public:
		// Ctor
		explicit
		LinearGauge (xf::Graphics const&, Converter = nullptr, std::string_view const& instance = {});

		// Module API
		void
		process (xf::Cycle const&) override;

		// Instrument API
		std::packaged_task<void()>
		paint (xf::PaintRequest) const override;

	  private:
		void
		async_paint (xf::PaintRequest const&, GaugeValues const&) const;

		void
		paint_indicator (GaugeValues const&, xf::InstrumentAids&, xf::InstrumentPainter&, float q, QPointF p0, QPointF p1) const;

		void
		paint_text (GaugeValues const&, xf::InstrumentAids&, xf::InstrumentPainter&, float q, QPointF p0) const;

	  private:
		LinearGauge<Value>&									_io { *this };
		xf::SocketObserver									_inputs_observer;
		Converter											_converter;
		// Cached stuff:
		xf::Synchronized<std::vector<PointInfo>> mutable	_point_infos;
	};


template<class Value>
	inline
	LinearGauge<Value>::LinearGauge (xf::Graphics const& graphics, Converter converter, std::string_view const& instance):
		Gauge<Value> (instance),
		xf::InstrumentSupport (graphics),
		_converter (converter)
	{
		_inputs_observer.set_callback ([&]{
			this->mark_dirty();
		});
		_inputs_observer.observe ({
			&_io.value,
		});
	}


template<class Value>
	inline void
	LinearGauge<Value>::process (xf::Cycle const& cycle)
	{
		_inputs_observer.process (cycle.update_time());
	}


template<class Value>
	inline std::packaged_task<void()>
	LinearGauge<Value>::paint (xf::PaintRequest paint_request) const
	{
		xf::Range const range { *_io.value_minimum, *_io.value_maximum };

		GaugeValues values;
		values.get_from (*this, range, (_converter && _io.value) ? _converter (*_io.value) : _io.value.to_floating_point());
		values.mirrored_style = *_io.mirrored_style;
		values.line_hidden = *_io.line_hidden;
		values.font_scale = *_io.font_scale;
		values.note_str = *_io.note;

		if (_io.value)
			values.inbound = xf::Range { 0.0f, 1.0f }.includes (xf::renormalize (*_io.value, range, BasicGauge::kNormalizedRange));

		return std::packaged_task<void()> ([this, pr = std::move (paint_request), gv = std::move (values)] {
			async_paint (pr, gv);
		});
	}


template<class Value>
	inline void
	LinearGauge<Value>::async_paint (xf::PaintRequest const& paint_request, GaugeValues const& values) const
	{
		auto aids = get_aids (paint_request);
		auto painter = get_painter (paint_request);
		float const w = aids->width();
		float const h = aids->height();

		if (values.mirrored_style)
		{
			painter.translate (w, 0.f);
			painter.scale (-1.f, 1.f);
		}

		float const q = 0.05f * w;
		float const m = 0.7f * q;
		QRectF area (m, m, w - 2.f * m, h - 2.f * m);
		QPointF p0 (area.right() - 3.f * q, area.bottom());
		QPointF p1 (area.right() - 3.f * q, area.top());

		if (!values.line_hidden)
			paint_indicator (values, *aids, painter, q, p0, p1);

		paint_text (values, *aids, painter, q, p0);
	}


template<class Value>
	inline void
	LinearGauge<Value>::paint_indicator (GaugeValues const& values, xf::InstrumentAids& aids, xf::InstrumentPainter& painter, float q, QPointF p0, QPointF p1) const
	{
		float const r = 6.5f * q;
		float const kValueSpanLength = (p1 - p0).y();

		auto get_length = [kValueSpanLength](std::optional<float> const& normalized) -> std::optional<float> {
			if (normalized)
				return kValueSpanLength * *normalized;
			else
				return std::nullopt;
		};

		QPen const silver_pen = aids.get_pen (BasicGauge::kSilver, 1.0f);
		QPen const warning_pen = aids.get_pen (BasicGauge::kWarningColor, 1.1f);
		QPen const critical_pen = aids.get_pen (BasicGauge::kCriticalColor, 1.1f);

		// Gauge line:
		painter.save_context ([&] {
			float const length_gap = aids.pen_width (2.0);

			std::optional<float> const minimum_critical_length = get_length (values.normalized_minimum_critical);
			std::optional<float> const minimum_warning_length = get_length (values.normalized_minimum_warning);
			std::optional<float> const maximum_warning_length = get_length (values.normalized_maximum_warning);
			std::optional<float> const maximum_critical_length = get_length (values.normalized_maximum_critical);

			QPen const no_pen;
			QColor const no_color;
			float const no_tick_len {};
			float const tick_dir = 1.0f;
			auto point_infos = _point_infos.lock();

			point_infos->clear();
			point_infos->push_back ({ PointInfo::Minimums, 0.0f, no_pen, no_tick_len, false });

			if (minimum_critical_length)
				point_infos->push_back ({ PointInfo::Minimums, *minimum_critical_length, critical_pen, tick_dir * 0.2f * r, true });

			if (minimum_warning_length)
				point_infos->push_back ({ PointInfo::Minimums, *minimum_warning_length, warning_pen, tick_dir * (minimum_critical_length ? 0.1f : 0.2f) * r, false });

			if (maximum_warning_length)
				point_infos->push_back ({ PointInfo::Maximums, *maximum_warning_length, warning_pen, tick_dir * (maximum_critical_length ? 0.1f : 0.2f) * r, false });

			if (maximum_critical_length)
				point_infos->push_back ({ PointInfo::Maximums, *maximum_critical_length, critical_pen, tick_dir * 0.2f * r, true });

			point_infos->push_back ({ PointInfo::Maximums, kValueSpanLength, no_pen, no_tick_len, false });

			// Actual painting:
			for (size_t i = 0u; i < point_infos->size() - 1; ++i)
			{
				PointInfo const prev = (*point_infos)[i];
				PointInfo const next = (*point_infos)[i + 1];
				bool const add_min_gap = (prev.zone == PointInfo::Minimums) && (i > 0);
				bool const add_max_gap = (next.zone == PointInfo::Maximums) && (i < point_infos->size() - 2);

				painter.save_context ([&] {
					auto const length_0 = prev.length - (add_min_gap ? length_gap : 0.0f);
					auto const length_1 = next.length + (add_max_gap ? length_gap : 0.0f);
					QPointF const uy (0.0f, 1.0f);
					QPointF const ux (1.0f, 0.0f);
					QPointF const std_umx (-0.25f, 0.0f);

					if (next.zone == PointInfo::Minimums)
					{
						QPointF const umx = next.critical ? std_umx : QPointF (0.0f, 0.0f);

						painter.setPen (next.pen);
						// Vertical:
						painter.drawLine (p0 + length_0 * uy, p0 + length_1 * uy);
						// Horizontal:
						painter.drawLine (p0 + length_1 * uy + next.tick_len * umx,
										  p0 + length_1 * uy + next.tick_len * ux);
					}
					else if (next.zone != PointInfo::Minimums && prev.zone != PointInfo::Maximums)
					{
						if (values.critical_condition)
							painter.setPen (critical_pen);
						else if (values.warning_condition)
							painter.setPen (warning_pen);
						else
							painter.setPen (silver_pen);

						painter.drawLine (p0 + length_0 * uy, p0 + length_1 * uy);
					}
					else if (prev.zone == PointInfo::Maximums)
					{
						QPointF const umx = prev.critical ? std_umx : QPointF (0.0f, 0.0f);

						painter.setPen (prev.pen);
						// Vertical:
						painter.drawLine (p0 + length_0 * uy, p0 + length_1 * uy);
						// Horizontal:
						painter.drawLine (p0 + length_0 * uy + prev.tick_len * umx,
										  p0 + length_0 * uy + prev.tick_len * ux);
					}
				});
			}
		});

		// Triangular indicator:
		if (values.normalized_value)
		{
			if (values.critical_condition)
			{
				painter.setBrush (BasicGauge::kCriticalColor);
				painter.setPen (critical_pen);
			}
			else if (values.warning_condition)
			{
				painter.setBrush (BasicGauge::kWarningColor);
				painter.setPen (warning_pen);
			}
			else
			{
				if (values.inbound)
					painter.setBrush (Qt::white);
				else
					painter.setBrush (Qt::NoBrush);

				painter.setPen (aids.get_pen (Qt::white, 1.0f));
			}

			QPolygonF triangle = QPolygonF()
				<< QPointF (0.f, 0.f)
				<< QPointF (1.5f * q, -0.5f * q)
				<< QPointF (1.5f * q, +0.5f * q);
			triangle.translate (p1.x() + 0.25f * q, xf::renormalize<qreal> (*values.normalized_value, 0.0f, 1.0f, p0.y(), p1.y()));
			painter.paint (aids.default_shadow(), [&] {
				painter.drawPolygon (triangle);
			});
		}
	}


template<class Value>
	inline void
	LinearGauge<Value>::paint_text (GaugeValues const& values, xf::InstrumentAids& aids, xf::InstrumentPainter& painter, float q, QPointF const p0) const
	{
		QFont font (aids.font_5.font);
		font.setPixelSize (font.pixelSize() * values.font_scale);
		QFontMetricsF const metrics (font);
		float const char_width = metrics.width ("0");
		float const hcorr = 0.025f * metrics.height();

		QPen text_pen = aids.get_pen (Qt::white, 0.8f);
		QPen box_pen = text_pen;

		if (values.critical_condition)
		{
			text_pen = aids.get_pen (BasicGauge::kCriticalColor, 1.0f);
			box_pen = text_pen;
		}
		else if (values.warning_condition)
		{
			text_pen = aids.get_pen (BasicGauge::kWarningColor, 1.0f);
			box_pen.setColor (BasicGauge::kCriticalColor);
		}

		// Box:
		auto const value_box_inner_margin = 0.33f * char_width;
		auto const note_distance = 0.5f * char_width;
		QPointF text_position;
		QPointF note_position;
		QString const sample_text = QString::fromStdString ((boost::format (values.format) % 0.0).str());
		painter.setFont (font);
		QRectF text_rect = painter.get_text_box (QPointF (p0.x() - 1.25 * q, aids.height() / 2.f), Qt::AlignRight | Qt::AlignVCenter, sample_text);
		text_rect.adjust (-2 * value_box_inner_margin, 0, 0.f, -2.f * hcorr);
		painter.setPen (box_pen);
		painter.setBrush (Qt::NoBrush);
		painter.drawRect (text_rect);

		if (values.mirrored_style)
		{
			text_position = QPointF (text_rect.left() + value_box_inner_margin, text_rect.center().y());
			text_position = painter.transform().map (text_position);
			note_position = QPointF (text_rect.left() - note_distance, text_rect.center().y());
			note_position = painter.transform().map (note_position);
		}
		else
		{
			text_position = QPointF (text_rect.right() - value_box_inner_margin, text_rect.center().y());
			note_position = QPointF (text_rect.left() - note_distance, text_rect.center().y());
		}

		painter.resetTransform();

		// Text:
		if (values.value_str)
		{
			auto const value_qstr = QString::fromStdString (*values.value_str);
			painter.setPen (text_pen);
			painter.fast_draw_text (text_position, Qt::AlignVCenter | Qt::AlignRight, value_qstr);
		}

		// Note:
		if (!values.note_str.empty())
		{
			auto const align_lr = values.mirrored_style ? Qt::AlignLeft : Qt::AlignRight;
			auto const value_qstr = QString::fromStdString (values.note_str);
			painter.setPen (text_pen);
			painter.fast_draw_text (note_position, Qt::AlignVCenter | align_lr, value_qstr);
		}
	}

#endif

