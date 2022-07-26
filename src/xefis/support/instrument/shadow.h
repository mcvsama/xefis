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

#ifndef XEFIS__SUPPORT__INSTRUMENT__SHADOW_H__INCLUDED
#define XEFIS__SUPPORT__INSTRUMENT__SHADOW_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QtGui/QColor>
#include <QtGui/QPen>

// Standard:
#include <cstddef>


namespace xf {

class Shadow
{
	constexpr static float kDefaultShadowWidth = 1.0f;

  public:
	QColor
	color() const;

	void
	set_color (QColor);

	float
	width() const;

	void
	set_width (float);

	float
	width_for_pen (QPen const& pen) const;

  private:
	float	_width	{ kDefaultShadowWidth };
	QColor	_color	{ 0x10, 0x20, 0x30, 127 };
};


inline QColor
Shadow::color() const
{
	return _color;
}


inline void
Shadow::set_color (QColor color)
{
	_color = color;
}


inline float
Shadow::width() const
{
	return _width;
}


inline void
Shadow::set_width (float width)
{
	_width = width;
}


inline float
Shadow::width_for_pen (QPen const& pen) const
{
	return pen.width() + 2 * width();
}

} // namespace xf

#endif

