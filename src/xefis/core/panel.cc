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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/services.h>
#include <xefis/core/xefis.h>
#include <xefis/utility/qdom.h>
#include <xefis/support/ui/widgets/panel_widget.h>

// Local:
#include "panel.h"


namespace xf {

Panel::Panel (QWidget* parent, Xefis* xefis):
	QWidget (parent),
	_xefis (xefis)
{
	setBackgroundRole (QPalette::Window);
	setAutoFillBackground (true);
	setFont (xf::Services::panel_font());
	setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Expanding);

	QPalette pal = palette();
	for (QPalette::ColorGroup color_group: { QPalette::Disabled, QPalette::Active, QPalette::Inactive })
	{
		for (QPalette::ColorRole color_role: { QPalette::Window, QPalette::Base, QPalette::AlternateBase, QPalette::Button })
			pal.setColor (color_group, color_role, pal.color (color_group, color_role).darker (300));
		for (QPalette::ColorRole color_role: { QPalette::WindowText, QPalette::Text, QPalette::ButtonText, QPalette::BrightText })
			pal.setColor (color_role, Qt::white);
	}
	pal.setColor (QPalette::Window, pal.color (QPalette::Window).darker (150));
	setPalette (pal);

	setBackgroundRole (QPalette::Window);
	setAutoFillBackground (true);

	_timer = new QTimer (this);
	_timer->setInterval (100);
	_timer->setSingleShot (false);
	QObject::connect (_timer, SIGNAL (timeout()), this, SLOT (read()));
	_timer->start();
}


Panel::~Panel()
{
	// Delete children manually, so that they have a chance to call unregister_panel_widget().
	for (QObject* child: children())
		delete child;
}


void
Panel::register_panel_widget (PanelWidget* panel_widget)
{
	_panel_widgets.insert (panel_widget);
}


void
Panel::unregister_panel_widget (PanelWidget* panel_widget)
{
	_panel_widgets.erase (panel_widget);
}


void
Panel::read()
{
	for (PanelWidget* pw: _panel_widgets)
		pw->data_updated();
}

} // namespace xf

