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

// Qt:
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "panel_button.h"


namespace xf {

PanelButton::PanelButton (QWidget* parent, Panel* panel, LEDColor color, v1::PropertyBoolean click_property, v1::PropertyBoolean toggle_property, v1::PropertyBoolean led_property):
	PanelWidget (parent, panel),
	_click_property (click_property),
	_toggle_property (toggle_property),
	_led_property (led_property)
{
	if (_led_property.configured())
	{
		switch (color)
		{
			case Green:
				_icon_on = QIcon (resources::icons16::led_green_on());
				break;

			case Amber:
				_icon_on = QIcon (resources::icons16::led_amber_on());
				break;

			case Red:
				_icon_on = QIcon (resources::icons16::led_red_on());
				break;

			case White:
				_icon_on = QIcon (resources::icons16::led_white_on());
				break;

			case Blue:
				_icon_on = QIcon (resources::icons16::led_blue_on());
				break;
		}

		_icon_off = QIcon (resources::icons16::led_off());
	}

	_button = new QPushButton (this);
	_button->setFixedSize (40, 25);
	QObject::connect (_button, SIGNAL (pressed()), this, SLOT (write()));
	QObject::connect (_button, SIGNAL (released()), this, SLOT (write()));

	QVBoxLayout* layout = new QVBoxLayout (this);
	layout->setMargin (0);
	layout->setSpacing (0);
	layout->addWidget (_button, 0, Qt::AlignCenter);

	read();
}


void
PanelButton::data_updated()
{
	read();
}


void
PanelButton::set_led_enabled (bool enabled)
{
	_button->setIcon (enabled ? _icon_on : _icon_off);
}


void
PanelButton::read()
{
	set_led_enabled (*_led_property);
}


void
PanelButton::write()
{
	if (_click_property.configured())
		_click_property.write (_button->isDown() || _button->isChecked());

	if (_button->isDown() && _toggle_property.configured())
		_toggle_property.write (!_toggle_property.read (false));
}

} // namespace xf

