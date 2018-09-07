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

#ifndef XEFIS__SUPPORT__INSTRUMENT__DEFAULT_INSTRUMENT_H__INCLUDED
#define XEFIS__SUPPORT__INSTRUMENT__DEFAULT_INSTRUMENT_H__INCLUDED

// Standard:
#include <cstddef>
#include <memory>
#include <optional>

// Qt:
#include <QtGui/QPainter>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/screen.h>
#include <xefis/support/instrument/instrument_aids.h>
#include <xefis/support/instrument/instrument_painter.h>
#include <xefis/support/instrument/text_painter.h>


namespace xf {

class InstrumentSupport
{
  public:
	std::shared_ptr<InstrumentAids>
	get_aids (PaintRequest const&) const;

	/**
	 * Return an instrument painter with pointers to local caches, etc.
	 * Use it in every paint() call, it will ensure that caches are reset
	 * when instrument size changes.
	 * Not thread safe!
	 */
	InstrumentPainter
	get_painter (PaintRequest&) const;

  private:
	void
	update_cache (PaintRequest const&) const;

  private:
	std::optional<PaintRequest::Metric> mutable		_cached_canvas_metric;
	std::shared_ptr<InstrumentAids> mutable			_cached_aids;
	TextPainter::Cache mutable						_text_painter_cache; // FIXME this implies thread-safety:
};


inline std::shared_ptr<InstrumentAids>
InstrumentSupport::get_aids (PaintRequest const& paint_request) const
{
	if (!_cached_aids || !_cached_canvas_metric || *_cached_canvas_metric != paint_request.metric())
		update_cache (paint_request);
		// TODO ↑ screen ref may change between get_aids() calls

	return _cached_aids;
}


inline InstrumentPainter
InstrumentSupport::get_painter (PaintRequest& paint_request) const
{
	if (!_cached_canvas_metric || *_cached_canvas_metric != paint_request.metric())
		update_cache (paint_request);

	return InstrumentPainter (paint_request.canvas(), _text_painter_cache);
}


inline void
InstrumentSupport::update_cache (PaintRequest const& paint_request) const
{
	_cached_aids = std::make_shared<InstrumentAids> (paint_request.metric());
	_text_painter_cache = TextPainter::Cache();
	_cached_canvas_metric = paint_request.metric();
}

} // namespace xf

#endif

