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

// Standard:
#include <cstddef>
#include <memory>

// Qt:
#include <QtXml/QDomElement>
#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QWidget>
#include <QtCore/QTimer>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/utility/qdom.h>
#include <xefis/utility/numeric.h>

// Local:
#include "mouse.h"


XEFIS_REGISTER_MODULE_CLASS ("io/mouse", Mouse)


Mouse::Mouse (xf::ModuleManager* module_manager, QDomElement const& config):
	Module (module_manager, config)
{
	parse_settings (config, {
		{ "dead-zone.x", _dead_zone_x, false },
		{ "dead-zone.y", _dead_zone_y, false },
		{ "speed.x", _speed_x, false },
		{ "speed.y", _speed_y, false },
		{ "acceleration.x", _acceleration_x, false },
		{ "acceleration.y", _acceleration_y, false },
	});

	parse_properties (config, {
		{ "axis-x", _axis_x, true },
		{ "axis-y", _axis_y, true },
		{ "button", _button, true },
	});

	_speed_x *= 20.f;
	_speed_y *= 20.f;

	QTimer* check_timer = new QTimer (this);
	check_timer->setInterval (1000.f / 50.f);
	QObject::connect (check_timer, SIGNAL (timeout()), this, SLOT (check()));
	check_timer->start();
}


void
Mouse::check()
{
	using xf::sgn;

	try {
		// Mouse move:
		float const dx = remove_dead_zone (_axis_x.read (0), _dead_zone_x);
		float const dy = remove_dead_zone (_axis_y.read (0), _dead_zone_y);
		QPoint pos (QCursor::pos());
		QCursor::setPos (pos.x() + _speed_x * sgn (dx) * std::pow (dx, _acceleration_x),
						 pos.y() + _speed_y * sgn (dy) * std::pow (dy, _acceleration_y));

		// Mouse press:
		// Unimplementable as QApplication::widgetAt() is broken now in Qt 5.0.
	}
	catch (xf::Exception const& e)
	{
		log() << "Exception when processing mouse position update." << std::endl;
		log() << e << std::endl;
	}
}


inline float
Mouse::remove_dead_zone (float input, float dead_deflection)
{
	if (std::abs (input) < dead_deflection)
		return 0.0;
	return input - xf::sgn (input) * dead_deflection;
}

