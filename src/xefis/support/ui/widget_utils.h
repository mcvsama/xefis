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

#ifndef XEFIS__SUPPORT__UI__WIDGET_UTILS_H__INCLUDED
#define XEFIS__SUPPORT__UI__WIDGET_UTILS_H__INCLUDED

// Qt:
#include <QPalette>
#include <QWidget>


namespace xf {

inline QPalette::ColorGroup
widget_color_group (QWidget const& widget)
{
	if (!widget.isEnabled())
		return QPalette::Disabled;

	return widget.isActiveWindow() ? QPalette::Active : QPalette::Inactive;
}

} // namespace xf

#endif
