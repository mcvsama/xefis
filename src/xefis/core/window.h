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

#ifndef XEFIS__CORE__WINDOW_H__INCLUDED
#define XEFIS__CORE__WINDOW_H__INCLUDED

// Standard:
#include <cstddef>
#include <memory>
#include <set>

// Qt:
#include <QtWidgets/QWidget>
#include <QtWidgets/QLayout>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStackedLayout>
#include <QtWidgets/QGroupBox>
#include <QtXml/QDomElement>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/property.h>
#include <xefis/core/xefis.h>

// Local:
#include "module.h"


namespace xf {

class Panel;
class ConfigReader;
class Instrument;

class Window: public QWidget
{
	Q_OBJECT

  public:
	/**
	 * Decorator for instruments. Allows replacing one instrument
	 * with another (or just newly created instance).
	 */
	class InstrumentDecorator: public QWidget
	{
	  public:
		// Ctor
		explicit
		InstrumentDecorator (QWidget* parent);

		/**
		 * Set instrument to be decorated.
		 * Old is overwritten without any other action.
		 */
		void
		set_instrument (Instrument*);

	  private:
		Instrument*	_instrument	= nullptr;
		QBoxLayout*	_layout;
	};

  private:
	/**
	 * Stack of layouts. Multiple instruments can be put in here
	 * and be shown according to an integer property.
	 */
	class Stack
	{
	  public:
		// Ctor
		explicit
		Stack (Time delay);

	  public:
		PropertyInteger			property;
		Unique<QStackedLayout>	layout;
		Unique<QTimer>			timer;

	  private:
		Unique<QWidget>			_black_widget;
	};

	typedef std::set<Shared<Stack>>							Stacks;
	typedef std::map<Module::Pointer, InstrumentDecorator*>	Decorators;

  public:
	// Ctor
	explicit
	Window (Xefis*, ConfigReader*, QDomElement const&);

	// Dtor
	~Window();

	/**
	 * Needed for layouts to update visible widgets, etc.
	 */
	void
	data_updated (Time const& update_time);

	/**
	 * Get pen scaling factor configured for this window.
	 */
	float
	pen_scale() const;

	/**
	 * Get font scaling factor configured for this window.
	 */
	float
	font_scale() const;

	/**
	 * Return decorator object for given instrument.
	 * May return nullptr.
	 */
	InstrumentDecorator*
	get_decorator_for (Module::Pointer const&) const;

  private:
	/**
	 * Unparent all children that are managed by ModuleManager.
	 */
	void
	unparent_modules();

	void
	process_window_element (QDomElement const& window_element);

	Panel*
	process_panel_element (QDomElement const& panel_element, QWidget* parent_widget);

	QWidget*
	process_group_element (QDomElement const& group_element, QWidget* parent_widget, Panel* panel);

	QWidget*
	process_widget_element (QDomElement const& widget_element, QWidget* parent_widget, Panel* panel);

	QLayout*
	process_layout_element (QDomElement const& layout_element, QWidget* parent_widget, Panel* panel);

	void
	process_item_element (QDomElement const& item_element, QLayout* layout, QWidget* parent_widget, Panel* panel, Shared<Stack> stack);

	void
	configurator_taken();

	static std::array<int, 4>
	parse_margin (QDomElement const& element, QString const& attribute_name);

	static std::array<int, 4>
	parse_padding (QDomElement const& element, QString const& attribute_name);

  private slots:
	void
	show_configurator();

  private:
	Xefis*			_xefis;
	ConfigReader*	_config_reader;
	QStackedWidget*	_stack;
	QWidget*		_instruments_panel;
	QWidget*		_configurator_panel;
	QPoint			_mouse_pos;
	Stacks			_stacks;
	Decorators		_decorators;
};

} // namespace xf

#endif

