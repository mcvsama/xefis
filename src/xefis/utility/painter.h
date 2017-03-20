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

#ifndef XEFIS__UTILITY__PAINTER_H__INCLUDED
#define XEFIS__UTILITY__PAINTER_H__INCLUDED

// Standard:
#include <cstddef>
#include <tuple>
#include <map>
#include <memory>

// Qt:
#include <QtGui/QPainter>
#include <QtGui/QImage>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/text_painter.h>


namespace xf {

class Painter: public TextPainter
{
	constexpr static float DefaultShadowWidth = 1.2f;

  public:
	// Ctor
	explicit
	Painter (TextPainter::Cache* cache);

	// Ctor
	explicit
	Painter (QPaintDevice* device, TextPainter::Cache* cache);

	void
	draw_outlined_line (QPointF const& from, QPointF const& to);

	void
	draw_outlined_polygon (QPolygonF const& polygon);

	bool
	painting_shadow() const;

	QColor
	shadow_color() const;

	void
	set_shadow_color (QColor color);

	void
	set_shadow_width (float width);

	void
	reset_shadow_width();

	void
	configure_for_shadow();

	void
	configure_normal();

	void
	add_shadow (std::function<void()> paint_function);

	void
	add_shadow (float width, std::function<void()> paint_function);

	void
	add_shadow (QColor color, std::function<void()> paint_function);

  private:
	float	_shadow_width			= DefaultShadowWidth;
	QColor	_shadow_color			= { 0x10, 0x20, 0x30, 127 };
	QPen	_saved_pen;
	bool	_painting_shadow		= false;
};


inline bool
Painter::painting_shadow() const
{
	return _painting_shadow;
}


inline QColor
Painter::shadow_color() const
{
	return _shadow_color;
}


inline void
Painter::set_shadow_color (QColor color)
{
	_shadow_color = color;
}


inline void
Painter::set_shadow_width (float width)
{
	_shadow_width = width;
}


inline void
Painter::reset_shadow_width()
{
	_shadow_width = DefaultShadowWidth;
}


inline void
Painter::configure_for_shadow()
{
	_painting_shadow = true;
	_saved_pen = pen();
	QPen s = _saved_pen;
	s.setColor (_shadow_color);
	s.setWidthF (s.width() + _shadow_width);
	setPen (s);
}


inline void
Painter::configure_normal()
{
	_painting_shadow = false;
	setPen (_saved_pen);
}


inline void
Painter::add_shadow (std::function<void()> paint_function)
{
	add_shadow (_shadow_width, paint_function);
}


inline void
Painter::add_shadow (float width, std::function<void()> paint_function)
{
	float prev = _shadow_width;
	_shadow_width = width;

	configure_for_shadow();
	paint_function();
	configure_normal();
	paint_function();

	_shadow_width = prev;
}


inline void
Painter::add_shadow (QColor color, std::function<void()> paint_function)
{
	QColor s = _shadow_color;
	_shadow_color = color;
	add_shadow (paint_function);
	_shadow_color = s;
}

} // namespace xf

#endif

