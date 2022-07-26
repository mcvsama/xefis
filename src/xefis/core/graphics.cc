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

// Local:
#include "graphics.h"

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QDir>
#include <QFontDatabase>

// Standard:
#include <cstddef>


namespace xf {

Graphics::Graphics (Logger const& logger):
	_logger (logger.with_scope ("<graphics>"))
{
	setup_fonts();
}


void
Graphics::setup_fonts()
{
	auto add_fonts_from = [](QString dirname) {
		for (QString entry: QDir (dirname).entryList ({ "*.ttf", "*.otf" }))
			QFontDatabase::addApplicationFont (dirname + "/" + entry);
	};

	// Try to select best font for instruments:
	add_fonts_from ("share/fonts");

	_instrument_font = QFont ("Crystal");
	_instrument_font.setBold (false);
	_instrument_font.setStretch (110);
	_instrument_font.setHintingPreference (QFont::PreferFullHinting);

	_panel_font = QFont ("Century Gothic");
	_panel_font.setBold (false);
	_panel_font.setStretch (110);
	_panel_font.setHintingPreference (QFont::PreferFullHinting);
	_panel_font.setPixelSize (11.0);
}

} // namespace xf

