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
#include <xefis/core/services.h>
#include <xefis/core/stdexcept.h>

// Local:
#include "qutils.h"


namespace xf {

extern void
setup_appereance (QTreeWidgetItem& item)
{
	QSize s = item.sizeHint (0);

	if (!item.treeWidget())
		throw InvalidArgument ("setup_appereance (QTreeWidgetItem&) requires item to be inserted into a tree");

	s.setHeight (1.75 * Services::default_font_size (item.treeWidget()->physicalDpiY()));
	item.setSizeHint (0, s);
}

} // namespace xf

