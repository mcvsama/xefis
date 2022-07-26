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
#include "panel_widget.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/panel.h>

// Standard:
#include <cstddef>


namespace xf {

PanelWidget::PanelWidget (QWidget* parent, Panel* panel):
	QWidget (parent),
	_panel (panel)
{
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	_panel->register_panel_widget (this);
}


PanelWidget::~PanelWidget()
{
	_panel->unregister_panel_widget (this);
}


void
PanelWidget::data_updated()
{ }

} // namespace xf

