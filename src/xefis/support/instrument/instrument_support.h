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

#ifndef XEFIS__SUPPORT__INSTRUMENT__INSTRUMENT_SUPPORT_H__INCLUDED
#define XEFIS__SUPPORT__INSTRUMENT__INSTRUMENT_SUPPORT_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/graphics.h>
#include <xefis/support/instrument/instrument_aids.h>
#include <xefis/support/instrument/instrument_painter.h>
#include <xefis/support/instrument/text_painter.h>

// Neutrino:
#include <neutrino/synchronized.h>

// Qt:
#include <QtGui/QPainter>

// Standard:
#include <cstddef>
#include <memory>
#include <optional>


namespace xf {

class InstrumentSupport
{
	struct Data
	{
		std::optional<PaintRequest::Metric>	cached_canvas_metric;
		std::shared_ptr<InstrumentAids>		cached_aids;
	};

  public:
	// Ctor
	explicit
	InstrumentSupport (Graphics const&);

	/**
	 * Return shared InstrumentAids object.
	 */
	std::shared_ptr<InstrumentAids>
	get_aids (PaintRequest const&) const;

	/**
	 * Return an instrument painter with pointers to local caches, etc.
	 * Use it in every paint() call, it will ensure that caches are reset
	 * when instrument size changes.
	 */
	InstrumentPainter
	get_painter (PaintRequest const&) const;

  private:
	void
	update_cache (PaintRequest const&, Data&) const;

  private:
	Graphics const&									_graphics;
	xf::Synchronized<Data> mutable					_data;
	static inline thread_local TextPainter::Cache	_text_painter_cache;
};


inline
InstrumentSupport::InstrumentSupport (Graphics const& graphics):
	_graphics (graphics)
{ }


inline std::shared_ptr<InstrumentAids>
InstrumentSupport::get_aids (PaintRequest const& paint_request) const
{
	auto data = _data.lock();

	if (!data->cached_aids || !data->cached_canvas_metric || *data->cached_canvas_metric != paint_request.metric())
		update_cache (paint_request, *data);

	return data->cached_aids;
}


inline InstrumentPainter
InstrumentSupport::get_painter (PaintRequest const& paint_request) const
{
	auto data = _data.lock();

	if (!data->cached_canvas_metric || *data->cached_canvas_metric != paint_request.metric())
		update_cache (paint_request, *data);

	return InstrumentPainter (paint_request.canvas(), _text_painter_cache);
}


inline void
InstrumentSupport::update_cache (PaintRequest const& paint_request, Data& data) const
{
	data.cached_aids = std::make_shared<InstrumentAids> (paint_request.metric(), _graphics);
	data.cached_canvas_metric = paint_request.metric();
}

} // namespace xf

#endif

