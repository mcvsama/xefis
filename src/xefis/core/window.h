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

#ifndef XEFIS__CORE__WINDOW_H__INCLUDED
#define XEFIS__CORE__WINDOW_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QWidget>
#include <QtWidgets/QLayout>
#include <QtWidgets/QStackedWidget>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/application.h>


namespace Xefis {

class ConfigReader;

class Window: public QWidget
{
	Q_OBJECT

  public:
	// Ctor
	Window (Application*, ConfigReader*, QDomElement const&);

  private:
	void
	process_layout_element (QDomElement const& layout_element, QBoxLayout* layout, QWidget* instruments_panel, int stretch = 0);

	void
	process_item_element (QDomElement const& item_element, QBoxLayout* layout, QWidget* instruments_panel);

	void
	configurator_taken();

  private slots:
	void
	show_configurator();

  private:
	Application*	_application;
	QStackedWidget*	_stack;
	QWidget*		_instruments_panel;
	QWidget*		_configurator_panel;
	ConfigReader*	_config_reader;
	QPoint			_mouse_pos;
};

} // namespace Xefis

#endif

