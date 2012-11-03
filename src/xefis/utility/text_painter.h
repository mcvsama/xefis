/* vim:ts=4
 *
 * Copyleft 2008…2012  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__UTILITY__SCALED_TEXT_H__INCLUDED
#define XEFIS__UTILITY__SCALED_TEXT_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtGui/QPainter>
#include <QtGui/QImage>

// Xefis:
#include <xefis/config/all.h>


/**
 * Draws bigger text and then scales it down to the destination area
 * for better quality fonts.
 */
class TextPainter
{
  public:
	TextPainter (QPainter& painter, float oversampling_factor = 2.0f);

	void
	drawText (QRectF const& target, int flags, QString const& text);

  private:
	QPainter&	_painter;
	QImage		_buffer;
	float		_oversampling_factor;
};

#endif

