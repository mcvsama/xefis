/* vim:ts=4
 *
 * Copyleft 2012…2013  Michał Gawron
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
#include <tuple>
#include <map>

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
	/**
	 * Stores drawn images.
	 */
	class Cache
	{
		struct Key
		{
			QRect	size;
			QColor	color;
			QString	text;
			int		flags;

			bool
			operator< (Key const& other) const;
		};

		typedef std::map<Key, QImage> CacheMap;

	  public:
		QImage*
		load_image (QRect const& size, QColor const& color, QString const& text, int flags);

		void
		store_image (QRect const& size, QColor const& color, QString const& text, int flags, QImage& image);

	  private:
		CacheMap _cache;
	};

  public:
	TextPainter (QPainter& painter, Cache* cache = nullptr, float oversampling_factor = 2.0f);

	void
	drawText (QRectF const& target, int flags, QString const& text, bool dont_cache = false);

  private:
	Cache*		_cache;
	QPainter&	_painter;
	QImage		_buffer;
	float		_oversampling_factor;
};


inline bool
TextPainter::Cache::Key::operator< (Key const& other) const
{
	Key const& a = *this;
	Key const& b = other;

	return
		std::make_tuple (a.size.top(), a.size.left(), a.size.bottom(), a.size.right(), a.color.rgba(), a.text, a.flags) <
		std::make_tuple (b.size.top(), b.size.left(), b.size.bottom(), b.size.right(), b.color.rgba(), b.text, b.flags);
}

#endif

