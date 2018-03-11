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

#ifndef XEFIS__SUPPORT__INSTRUMENT__PAINTER_SHADOW_H__INCLUDED
#define XEFIS__SUPPORT__INSTRUMENT__PAINTER_SHADOW_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QPainter>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/resource.h>
#include <xefis/utility/temporary_change.h>


namespace xf {

class PainterShadow
{
	constexpr static float kDefaultShadowWidth = 1.2f;

  public:
	using PaintFunction = std::function<void (bool painting_shadow)>;

  public:
	QColor
	shadow_color() const;

	void
	set_shadow_color (QColor color);

	float
	shadow_width() const;

	void
	set_shadow_width (float width);

	void
	reset_shadow_width();

	void
	add_shadow (QPainter&, PaintFunction);

	void
	add_shadow (QPainter&, float width, PaintFunction);

	void
	add_shadow (QPainter&, QColor color, PaintFunction);

  private:
	float		_shadow_width	= kDefaultShadowWidth;
	QColor		_shadow_color	= { 0x10, 0x20, 0x30, 127 };
};


inline QColor
PainterShadow::shadow_color() const
{
	return _shadow_color;
}


inline void
PainterShadow::set_shadow_color (QColor color)
{
	_shadow_color = color;
}


inline float
PainterShadow::shadow_width() const
{
	return _shadow_width;
}


inline void
PainterShadow::set_shadow_width (float width)
{
	_shadow_width = width;
}


inline void
PainterShadow::reset_shadow_width()
{
	_shadow_width = kDefaultShadowWidth;
}


inline void
PainterShadow::add_shadow (QPainter& painter, PaintFunction paint_function)
{
	{
		auto saved_pen = painter.pen();
		Resource pen_restore ([&] { painter.setPen (saved_pen); });

		QPen new_pen = saved_pen;
		new_pen.setColor (_shadow_color);
		new_pen.setWidthF (new_pen.width() + _shadow_width);
		painter.setPen (new_pen);

		paint_function (true);
	}

	paint_function (false);
}


inline void
PainterShadow::add_shadow (QPainter& painter, float width, PaintFunction paint_function)
{
	TemporaryChange width_change (_shadow_width, width);
	add_shadow (painter, paint_function);
}


inline void
PainterShadow::add_shadow (QPainter& painter, QColor color, PaintFunction paint_function)
{
	TemporaryChange color_change (_shadow_color, color);
	add_shadow (painter, paint_function);
}

} // namespace xf

#endif

