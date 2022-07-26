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

#ifndef XEFIS__SUPPORT__INSTRUMENT__TEXT_PAINTER_H__INCLUDED
#define XEFIS__SUPPORT__INSTRUMENT__TEXT_PAINTER_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/instrument/shadow.h>

// Qt:
#include <QtGui/QImage>
#include <QtGui/QPainter>

// Standard:
#include <cstddef>
#include <map>
#include <memory>
#include <optional>
#include <tuple>


namespace xf {

/**
 * Draws bigger text and then scales it down to the destination area
 * for better quality fonts.
 */
class TextPainter: virtual public QPainter
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
			explicit
			Glyph (QFont const&, QColor, QChar, QPointF position_correction, std::optional<Shadow> shadow);

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
			float	shadow_width;

			bool
			operator< (Font const&) const;

			bool
			operator== (Font const&) const;

			bool
			operator!= (Font const&) const;
		};

		using Glyphs	= std::map<QChar, Glyph>;
		using Fonts		= std::map<Font, Glyphs>;

		struct Last
		{
			Last (Font const&, Glyphs*);

			Font	font;
			Glyphs*	glyphs;
		};

		Fonts				fonts;
		std::optional<Last>	last;
	};

  public:
	// Ctor
	explicit
	TextPainter (Cache& cache);

	// Ctor
	explicit
	TextPainter (QPaintDevice& device, Cache& cache);

	/**
	 * Set font position correction (value is relative to font's size, it's not represented in pixels).
	 */
	void
	set_font_position_correction (QPointF correction);

	QRectF
	get_text_box (QPointF const& position, Qt::Alignment flags, QString const& text) const;

	QRectF
	get_vertical_text_box (QPointF const& position, Qt::Alignment flags, QString const& text) const;

	void
	fast_draw_text (QPointF const& position, QString const& text, std::optional<Shadow> = {});

	void
	fast_draw_text (QPointF const& position, Qt::Alignment flags, QString const& text, std::optional<Shadow> = {});

	void
	fast_draw_text (QRectF const& target, Qt::Alignment flags, QString const& text, std::optional<Shadow> = {});

	void
	fast_draw_vertical_text (QPointF const& position, Qt::Alignment flags, QString const& text, std::optional<Shadow> = {});

  private:
	/**
	 * Apply alignment flags to given rectangle.
	 */
	static void
	apply_alignment (QRectF& rect, Qt::Alignment flags);

  private:
	Cache&	_cache;
	QPointF	_position_correction;
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
	return std::tuple (font, color.rgba(), shadow_width) < std::tuple (other.font, other.color.rgba(), other.shadow_width);
}


inline bool
TextPainter::Cache::Font::operator== (Font const& other) const
{
	return font == other.font
		&& color == other.color
		&& shadow_width == other.shadow_width;
}


inline bool
TextPainter::Cache::Font::operator!= (Font const& other) const
{
	return !(*this == other);
}


inline
TextPainter::Cache::Last::Last (Font const& font, Glyphs* glyphs):
	font (font),
	glyphs (glyphs)
{ }

} // namespace xf

#endif

