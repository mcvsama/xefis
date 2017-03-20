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

#ifndef XEFIS__SUPPORT__UI__WIDGETS__PANEL_BUTTON_H__INCLUDED
#define XEFIS__SUPPORT__UI__WIDGETS__PANEL_BUTTON_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QPushButton>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <xefis/support/ui/widgets/panel_widget.h>


namespace xf {

class PanelButton: public PanelWidget
{
	Q_OBJECT

  public:
	/**
	 * Color of the LED.
	 */
	enum LEDColor
	{
		Green,
		Amber,
		Red,
		White,
		Blue,
	};

  public:
	/**
	 * Create button with LED indicator.
	 */
	explicit
	PanelButton (QWidget* parent, Panel*, LEDColor, PropertyBoolean click_property, PropertyBoolean toggle_property, PropertyBoolean led_property);

  protected:
	void
	data_updated() override;

  private:
	/**
	 * Enable or disable built-in LED.
	 */
	void
	set_led_enabled (bool);

  private slots:
	/**
	 * Read property and update state.
	 */
	void
	read();

	/**
	 * Write to controlled properties.
	 */
	void
	write();

  private:
	QPushButton*	_button					= nullptr;
	PropertyBoolean	_click_property;
	PropertyBoolean	_toggle_property;
	PropertyBoolean	_led_property;
	QIcon			_icon_on;
	QIcon			_icon_off;
};

} // namespace xf

#endif

