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

#ifndef XEFIS__SUPPORT__UI__WINDOW_H__INCLUDED
#define XEFIS__SUPPORT__UI__WINDOW_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>


namespace v2 {
using namespace xf; // XXX

class Window: public QWidget
{
	Q_OBJECT

  public:
	// Ctor
	explicit
	Window();

	// Dtor
	~Window();

	/**
	 * Set window full-screen.
	 */
	void
	set_fullscreen();

	/**
	 * Reference the widget containing instruments.
	 */
	QWidget*
	instruments_panel() const noexcept;

  private slots:
	/**
	 * Show configurator widget in current window. If shown in any other windows,
	 * hide them there.
	 */
	void
	show_configurator();

  private:
	/**
	 * Unparent all children that are managed by ModuleManager.
	 */
	void
	unparent_modules();

  private:
	QStackedWidget*	_stack;
	QWidget*		_instruments_panel;
	QWidget*		_configurator_panel;
};


inline QWidget*
Window::instruments_panel() const noexcept
{
	return _instruments_panel;
}

} // namespace v2

#endif

