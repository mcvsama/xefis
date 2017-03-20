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

#ifndef XEFIS__CORE__PANEL_H__INCLUDED
#define XEFIS__CORE__PANEL_H__INCLUDED

// Standard:
#include <cstddef>
#include <set>

// Qt:
#include <QtCore/QTimer>
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

class Xefis;
class PanelWidget;


class Panel: public QWidget
{
	Q_OBJECT

  public:
	// Ctor
	explicit
	Panel (QWidget* parent, Xefis*);

	// Dtor
	~Panel();

	/**
	 * Register PanelWidget on this panel.
	 */
	void
	register_panel_widget (PanelWidget*);

	/**
	 * Unregister PanelWidget from this panel.
	 */
	void
	unregister_panel_widget (PanelWidget*);

  private slots:
	void
	read();

  private:
	Xefis*					_xefis;
	QTimer*					_timer;
	std::set<PanelWidget*>	_panel_widgets;
};

} // namespace xf

#endif

