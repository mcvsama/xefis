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
#include <set>

// Qt:
#include <QtWidgets/QWidget>
#include <QtWidgets/QLayout>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStackedLayout>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/application.h>
#include <xefis/core/property.h>


namespace Xefis {

class ConfigReader;

class Window: public QWidget
{
	Q_OBJECT

	struct Stack
	{
		PropertyInteger	property;
		QStackedLayout*	layout;
	};

	typedef std::set<Stack*> Stacks;

  public:
	// Ctor
	Window (Application*, ConfigReader*, QDomElement const&);

	// Dtor
	~Window();

	/**
	 * Needed for layouts to update visible widgets, etc.
	 */
	void
	data_updated (Time const& update_time);

  private:
	QLayout*
	process_layout_element (QDomElement const& layout_element, QWidget* instruments_panel);

	void
	process_item_element (QDomElement const& item_element, QLayout* layout, QWidget* instruments_panel, Stack* stack);

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
	Stacks			_stacks;
};

} // namespace Xefis

#endif

