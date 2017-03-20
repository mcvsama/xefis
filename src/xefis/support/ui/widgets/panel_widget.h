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

#ifndef XEFIS__SUPPORT__UI__WIDGETS__PANEL_WIDGET_H__INCLUDED
#define XEFIS__SUPPORT__UI__WIDGETS__PANEL_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>


namespace xf {

class Panel;


class PanelWidget: public QWidget
{
  public:
	explicit
	PanelWidget (QWidget* parent, Panel*);

	~PanelWidget();

	/**
	 * Notify all children about data update.
	 */
	virtual void
	data_updated();

  private:
	Panel* _panel;
};

} // namespace xf

#endif

