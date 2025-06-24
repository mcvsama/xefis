/* vim:ts=4
 *
 * Copyleft 2025  Michał Gawron
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
#include "keys_values_widget.h"

// Xefis:
#include <xefis/config/all.h>

// Neutrino:
#include <neutrino/qt/qstring.h>

// Standard:
#include <cstddef>


namespace xf {

KeysValuesWidget::KeysValuesWidget (std::u8string_view const title, QWidget* parent):
	QGroupBox (parent),
	_layout (new QGridLayout (this))
{
	setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);
	setTitle (nu::to_qstring (title));
}


void
KeysValuesWidget::add (std::u8string_view const title, QLabel& value_label)
{
	auto const row = _layout->rowCount();
	_layout->addWidget (new QLabel (nu::to_qstring (title)), row, 0);
	_layout->addWidget (&value_label, row, 1);
}

} // namespace xf

