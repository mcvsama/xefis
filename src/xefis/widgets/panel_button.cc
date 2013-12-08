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

// Qt:
#include <QtWidgets/QLayout>

// Xefis:
#include <xefis/config/all.h>

// Local:
#include "panel_button.h"


namespace Xefis {

PanelButton::PanelButton (QWidget* parent, Panel* panel, LEDColor color, PropertyBoolean controlled_property):
	PanelButton (parent, panel, color, controlled_property, PropertyBoolean())
{ }


PanelButton::PanelButton (QWidget* parent, Panel* panel, LEDColor color, PropertyBoolean controlled_property, PropertyBoolean led_property):
	PanelWidget (parent, panel),
	_controlled_property (controlled_property),
	_led_property (led_property)
{
	if (_led_property.configured())
	{
		switch (color)
		{
			case Green:
				_icon_on = QIcon (Resources::Icons16::led_green_on());
				break;

			case Amber:
				_icon_on = QIcon (Resources::Icons16::led_amber_on());
				break;
		}

		_icon_off = QIcon (Resources::Icons16::led_off());
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
	_controlled_property.write (_button->isDown() || _button->isChecked());
	signal_data_updated();
}

} // namespace Xefis

