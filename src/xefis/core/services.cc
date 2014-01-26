/* vim:ts=4
 *
 * Copyleft 2012…2014  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

// Standard:
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Qt:
#include <QtGui/QFontDatabase>
#include <QtGui/QFontInfo>
#include <QtCore/QDir>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "services.h"


namespace Xefis {

signed int					Services::_detected_cores = -1;
Unique<CallOutDispatcher>	Services::_call_out_dispatcher;
QFont						Services::_instrument_font ("sans");
QFont						Services::_panel_font ("sans");


void
CallOutDispatcher::customEvent (QEvent* event)
{
	Services::CallOutEvent* coe = dynamic_cast<Services::CallOutEvent*> (event);
	if (coe)
	{
		coe->accept();
		coe->call_out();
	}
}


void
Services::initialize()
{
	_call_out_dispatcher = std::make_unique<CallOutDispatcher>();

	auto add_fonts_from = [](QString dirname) -> void {
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


void
Services::deinitialize()
{
}


unsigned int
Services::detected_cores()
{
	if (_detected_cores != -1)
		return _detected_cores;

	_detected_cores = 0;
	std::ifstream cpuinfo ("/proc/cpuinfo");
	std::string line;
	while (cpuinfo.good())
	{
		std::getline (cpuinfo, line);
		std::istringstream s (line);
		std::string name, colon;
		s >> name >> colon;
		if (name == "processor" && colon == ":")
			++_detected_cores;
	}
	return _detected_cores;
}


std::vector<const char*>
Services::features()
{
	std::vector<const char*> features;

	return features;
}


Services::CallOutEvent*
Services::call_out (boost::function<void()> callback)
{
	CallOutEvent* e = new CallOutEvent (callback);
	QApplication::postEvent (_call_out_dispatcher.get(), e);
	return e;
}

} // namespace Xefis

