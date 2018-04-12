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

#ifndef XEFIS__SUPPORT__INSTRUMENT__INSTRUMENT_AIDS_H__INCLUDED
#define XEFIS__SUPPORT__INSTRUMENT__INSTRUMENT_AIDS_H__INCLUDED

// Standard:
#include <cstddef>
#include <optional>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/instrument/instrument_painter.h>
#include <xefis/support/instrument/text_painter.h>
#include <xefis/utility/strong_type.h>


namespace xf {

using AspectRatio = StrongType<float, struct AspectRatioType>;

class InstrumentAids
{
  public:
	/**
	 * Return an instrument painter with pointers to local caches, etc.
	 * Not thread safe!
	 */
	InstrumentPainter
	get_painter (QPaintDevice&) const;

  protected:
	// TODO initialize these:
	QFont	font_0;
	QFont	font_1;
	QFont	font_2;
	QFont	font_3;

	static QRectF
	centered_rect (QRectF whole_area, AspectRatio);

  private:
	// FIXME this implies thread-safety:
	TextPainter::Cache mutable	_text_painter_cache;
	std::optional<AspectRatio>	_aspect_ratio;
};


inline QRectF
InstrumentAids::centered_rect (QRectF whole_area, AspectRatio)
{
	// TODO
	return { 0, 0, 1, 1 };
}

} // namespace xf

#endif

