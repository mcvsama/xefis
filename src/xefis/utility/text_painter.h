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
#include <memory>

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
	 * Stores drawn glyphs.
	 */
	class Cache
	{
		friend class TextPainter;

		class Glyph
		{
		  public:
			static constexpr int Rank = 8;

		  private:
			struct Data
			{
				QImage positions[Rank][Rank];
			};

		  public:
			/**
			 * Generate all images for given character and font.
			 */
			Glyph (QFont const&, QColor const&, QChar const&);

			Glyph (Glyph const& other);

			Glyph&
			operator= (Glyph const& other);

		  public:
			std::shared_ptr<Data> data;
		};

		struct Font
		{
			QFont	font;
			QColor	color;

			bool
			operator< (Font const& other) const;
		};

		typedef std::map<QChar, Glyph>	Glyphs;
		typedef std::map<Font, Glyphs>	Fonts;

		Fonts fonts;
	};

  public:
	TextPainter (QPainter& painter, Cache* cache);

	void
	drawText (QPointF const& position, QString const& text);

	void
	drawText (QRectF const& target, int flags, QString const& text);

  private:
	Cache*		_cache;
	QPainter&	_painter;
};


inline
TextPainter::Cache::Glyph::Glyph (Glyph const& other):
	data (other.data)
{ }


inline TextPainter::Cache::Glyph&
TextPainter::Cache::Glyph::operator= (Glyph const& other)
{
	data = other.data;
	return *this;
}


inline bool
TextPainter::Cache::Font::operator< (Font const& other) const
{
	return std::make_pair (font, color.rgba()) < std::make_pair (other.font, other.color.rgba());
}

#endif

