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

// Standard:
#include <cstddef>

// Xefis:
#include <xefis/config/all.h>
#include <xefis/application/services.h>

// Local:
#include "instrument_widget.h"


namespace Xefis {

InstrumentWidget::InstrumentWidget (QWidget* parent):
	QWidget (parent)
{
	setCursor (QCursor (QPixmap (XEFIS_SHARED_DIRECTORY "/images/cursors/crosshair.png")));
}

} // namespace Xefis

