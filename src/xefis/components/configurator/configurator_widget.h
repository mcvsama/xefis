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

#ifndef XEFIS__COMPONENTS__CONFIGURATOR__CONFIGURATOR_WIDGET_H__INCLUDED
#define XEFIS__COMPONENTS__CONFIGURATOR__CONFIGURATOR_WIDGET_H__INCLUDED

// Standard:
#include <cstddef>

// Qt:
#include <QtWidgets/QWidget>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/components/property_tree/property_tree_widget.h>
#include <xefis/core/window.h>


namespace Xefis {

class ConfiguratorWidget: public QWidget
{
	Q_OBJECT

  public:
	// Ctor
	ConfiguratorWidget (QWidget* parent);

	Window*
	owning_window() const;

	void
	set_owning_window (Window*);

  private slots:
	void
	read_properties();

  private:
	Xefis::PropertyTreeWidget*	_property_tree_widget	= nullptr;
	Window*						_owning_window			= nullptr;
};


inline Window*
ConfiguratorWidget::owning_window() const
{
	return _owning_window;
}


inline void
ConfiguratorWidget::set_owning_window (Window* window)
{
	_owning_window = window;
}

} // namespace Xefis

#endif

