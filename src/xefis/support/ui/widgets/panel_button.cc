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
#include "panel_button.h"

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QtWidgets/QLayout>

// Standard:
#include <cstddef>


namespace xf {

PanelButton::PanelButton (QWidget* parent, Panel* panel, LEDColor color, Socket<bool> click_socket, Socket<bool> toggle_socket, Socket<bool> led_socket):
	PanelWidget (parent, panel),
	_click_socket (click_socket),
	_toggle_socket (toggle_socket),
	_led_socket (led_socket)
{
	if (_led_socket.configured())
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
	set_led_enabled (*_led_socket);
}


void
PanelButton::write()
{
	if (_click_socket.configured())
		_click_socket.write (_button->isDown() || _button->isChecked());

	if (_button->isDown() && _toggle_socket.configured())
		_toggle_socket.write (!_toggle_socket.read (false));
}

} // namespace xf

