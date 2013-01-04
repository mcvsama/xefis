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

// Standard:
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Qt:
#include <QtGui/QFontDatabase>
#include <QtGui/QFontInfo>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "services.h"


namespace Xefis {

signed int			Services::_detected_cores = -1;
CallOutDispatcher*	Services::_call_out_dispatcher;
QFont				Services::_instrument_font ("sans");


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
	_call_out_dispatcher = new CallOutDispatcher();

	// Try to select best font for instruments:
	for (QString font: { "Black", "Bold", "BoldCondensed", "Condensed", "Light", "Medium", "Regular", "Thin" })
		QFontDatabase::addApplicationFont ("share/fonts/Roboto/Roboto-" + font + ".ttf");

	for (auto font_family: { "Roboto", "Bitstream Vera Sans Mono", "Ubuntu Mono", "Droid Sans", "Trebuchet MS", "monospace" })
	{
		QFont font (font_family);
		QFontInfo font_info (font);
		if (font_info.exactMatch())
		{
			_instrument_font = font;
			_instrument_font.setHintingPreference (QFont::PreferNoHinting);
			break;
		}
	}
}


void
Services::deinitialize()
{
	delete _call_out_dispatcher;
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
	QApplication::postEvent (_call_out_dispatcher, e);
	return e;
}

} // namespace Xefis

