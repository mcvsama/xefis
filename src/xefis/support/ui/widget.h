/* vim:ts=4
 *
 * Copyleft 2022  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__SUPPORT__UI__WIDGET_H__INCLUDED
#define XEFIS__SUPPORT__UI__WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/qt/qutils.h>

// Qt:
#include <QWidget>

// Standard:
#include <cstddef>
#include <memory>


namespace xf {

class Widget: public QWidget
{
  public:
	// Ctor
	explicit
	Widget (QWidget* parent = nullptr, Qt::WindowFlags = Qt::Widget);

  public:
	/**
	 * Return simple uniform-color widget.
	 */
	static QWidget*
	create_color_widget (QColor, QWidget* parent);

	/**
	 * Return label with colored strip.
	 * \param	strip_position
	 *			Qt::AlignTop, Qt::AlignBottom, Qt::AlignLeft or Qt::AlignRight.
	 */
	QWidget*
	create_colored_strip_label (QString const& label, QColor color, Qt::Alignment strip_position, QWidget* parent) const;
};

} // namespace xf

#endif

