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

#ifndef XEFIS__UTILITY__TEXT_LAYOUT_H__INCLUDED
#define XEFIS__UTILITY__TEXT_LAYOUT_H__INCLUDED

// Standard:
#include <cstddef>
#include <vector>

// Qt:
#include <QtCore/QString>
#include <QtGui/QFont>
#include <QtGui/QFontMetrics>
#include <QtGui/QColor>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/text_painter.h>


namespace xf {

class TextLayout
{
  public:
	enum BackgroundMode
	{
		Whole,		// Whole background rectangle will have background.
		PerLine,	// Each line (with presumably different width) have background painted separately.
	};

  private:
	/**
	 * Represents one fragment of text with individual font,
	 * color and text.
	 */
	class Fragment
	{
	  public:
		// Ctor
		explicit
		Fragment (QString const&, QFont const&, QColor const&, QPen const&, double line_height_factor);

		/**
		 * Return fragment width.
		 */
		double
		width() const noexcept;

		/**
		 * Return fragment height, which is actually
		 * font height.
		 */
		double
		height() const noexcept;

		/**
		 * Return font metrics for this fragment.
		 */
		QFontMetricsF const&
		metrics() const noexcept;

		/**
		 * Paint text at given top-left position.
		 */
		void
		paint (QPointF top_left, TextPainter& painter) const;

	  private:
		QString			_text;
		QFont			_font;
		QColor			_color;
		QPen			_box_pen;
		double			_width;
		double			_height;
		QFontMetricsF	_metrics;
		double			_line_height_factor;
	};

	typedef std::vector<Fragment> Fragments;

	/**
	 * Represents one line of text consisting of Fragments.
	 */
	class Line
	{
	  public:
		// Ctor
		explicit
		Line (double line_height_factor);

		/**
		 * Compute total width of line - sum of all fragments'
		 * widths.
		 */
		double
		width() const;

		/**
		 * Compute total height of line, which is height
		 * of the biggest font used.
		 */
		double
		height() const;

		/**
		 * Add fragment to the line.
		 */
		void
		add_fragment (Fragment const&);

		/**
		 * Paint line in given location.
		 */
		void
		paint (QPointF top_left, TextPainter& painter) const;

	  private:
		Fragments	_fragments;
		double		_line_height_factor;
	};

	typedef std::vector<Line> Lines;

  public:
	// Ctor
	TextLayout();

	/**
	 * Set default text alignment for all lines.
	 * Only horizontal part of this alignment is used
	 * (left/right/centered).
	 */
	void
	set_alignment (Qt::Alignment);

	/**
	 * Set background brush.
	 */
	void
	set_background (QBrush, QSizeF margin = { 0.0, 0.0 });

	/**
	 * Set background filling mode.
	 */
	void
	set_background_mode (BackgroundMode mode);

	/**
	 * Set line height factor.
	 */
	void
	set_line_height_factor (double factor);

	/**
	 * Add new line to the layout.
	 */
	void
	add_new_line();

	/**
	 * Add empty lines to the layout.
	 * \param	font is used to compute empty line height.
	 * \param	number is number of empty lines to add.
	 */
	void
	add_skips (QFont const& font, unsigned int number);

	/**
	 * Add text fragment to current line.
	 */
	void
	add_fragment (const char* text, QFont const& font, QColor const& color, QPen const& box_pen = Qt::NoPen);

	/**
	 * Add text fragment to current line.
	 */
	void
	add_fragment (QString const& text, QFont const& font, QColor const& color, QPen const& box_pen = Qt::NoPen);

	/**
	 * Add text fragment to current line.
	 */
	void
	add_fragment (std::string const& text, QFont const& font, QColor const& color, QPen const& box_pen = Qt::NoPen);

	/**
	 * Compute total width of laid out text.
	 */
	double
	width() const;

	/**
	 * Compute total height of laid out text.
	 */
	double
	height() const;

	/**
	 * Return size of the layout block - calls width() and height().
	 */
	QSizeF
	size() const;

	/**
	 * Paint result on painter.
	 * \param	position is the block position (see alignment below).
	 * \param	alignment is the alignment of the whole layout block.
	 * \param	painter is the painter to use.
	 */
	void
	paint (QPointF position, Qt::Alignment alignment, TextPainter& painter) const;

  private:
	Qt::Alignment	_default_line_alignment		= Qt::AlignLeft | Qt::AlignTop;
	QBrush			_background					= Qt::NoBrush;
	QSizeF			_background_margin			= { 0.0, 0.0 };
	BackgroundMode	_background_mode			= Whole;
	Lines			_lines;
	double			_line_height_factor			= 1.0;
};


inline
TextLayout::Fragment::Fragment (QString const& text, QFont const& font, QColor const& color, QPen const& box_pen, double line_height_factor):
	_text (text),
	_font (font),
	_color (color),
	_box_pen (box_pen),
	_metrics (_font),
	_line_height_factor (line_height_factor)
{
	_width = _metrics.width (text);
	double const hardcoded_additional_factor = 0.9;
	_height = _line_height_factor * hardcoded_additional_factor * _metrics.height();
}


inline double
TextLayout::Fragment::width() const noexcept
{
	return _width;
}


inline double
TextLayout::Fragment::height() const noexcept
{
	return _height;
}


inline QFontMetricsF const&
TextLayout::Fragment::metrics() const noexcept
{
	return _metrics;
}


inline void
TextLayout::Line::add_fragment (Fragment const& fragment)
{
	_fragments.push_back (fragment);
}


inline
TextLayout::TextLayout()
{
	add_new_line();
}


inline void
TextLayout::set_alignment (Qt::Alignment alignment)
{
	_default_line_alignment = alignment;
}


inline void
TextLayout::set_background (QBrush brush, QSizeF margin)
{
	_background = brush;
	_background_margin = margin;
}


inline void
TextLayout::set_background_mode (BackgroundMode mode)
{
	_background_mode = mode;
}


inline void
TextLayout::set_line_height_factor (double factor)
{
	_line_height_factor = factor;
}


inline void
TextLayout::add_new_line()
{
	_lines.emplace_back (_line_height_factor);
}


inline void
TextLayout::add_fragment (const char* text, QFont const& font, QColor const& color, QPen const& box_pen)
{
	add_fragment (QString (text), font, color, box_pen);
}


inline void
TextLayout::add_fragment (std::string const& text, QFont const& font, QColor const& color, QPen const& box_pen)
{
	add_fragment (QString::fromStdString (text), font, color, box_pen);
}


inline QSizeF
TextLayout::size() const
{
	return QSizeF (width(), height());
}

} // namespace xf

#endif

