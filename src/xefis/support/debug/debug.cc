/* vim:ts=4
 *
 * Copyleft 2026  Michał Gawron
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
#include "debug.h"

// Xefis:
#include <xefis/config/all.h>

// Qt:
#include <QCheckBox>
#include <QLabel>
#include <QWidget>

// Standard:
#include <cstddef>
#include <map>


namespace xf {

QVBoxLayout&
get_debug_window_layout()
{
	static thread_local std::unique_ptr<QWidget> debug_widget;

	if (!debug_widget)
	{
		debug_widget = std::make_unique<QWidget>();

		auto* label = new QLabel ("Debug controls");
		auto font = label->font();
		font.setPointSize (font.pointSize() * 1.2);
		label->setFont (font);

		auto* layout = new QVBoxLayout (&*debug_widget);
		layout->addWidget (label);

		debug_widget->show();
	}

	return dynamic_cast<QVBoxLayout&> (*debug_widget->layout());
}

} // namespace xf
