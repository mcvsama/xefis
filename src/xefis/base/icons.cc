/* vim:ts=4
 *
 * Copyleft 2024  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Local:
#include "icons.h"

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>
#include <QString>
#include <QSvgRenderer>

// Standard:
#include <cstddef>


namespace xf::icons {

QPixmap
from_png_file (QString const& png_file) noexcept
{
	QPixmap pixmap;
	QString key = "icon.png." + png_file;

	if (!QPixmapCache::find (key, &pixmap))
	{
		pixmap = QPixmap (png_file);
		QPixmapCache::insert (key, pixmap);
	}

	return pixmap;
}


QPixmap
from_svg_file (QString const& svg_file, std::optional<unsigned int> size_px_opt) noexcept
{
	auto size_px = size_px_opt.value_or (64u);
	QPixmap pixmap;
	QString key = "icon/svg/" + svg_file;

	if (!QPixmapCache::find (key, &pixmap))
	{
		QSvgRenderer svg (svg_file);
		pixmap = QPixmap (QSize (static_cast<int> (size_px), static_cast<int> (size_px)));
		pixmap.fill (Qt::transparent);
		QPainter painter (&pixmap);
		svg.render (&painter);
		QPixmapCache::insert (key, pixmap);
	}

	return pixmap;
}

} // namespace xf::icons

