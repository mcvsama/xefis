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

// Local:
#include "instrument.h"


namespace v2 {

Instrument::Instrument (std::string const& instance):
	Module (instance),
	QWidget (nullptr)
{
	setFont (xf::Services::instrument_font());
	setCursor (QCursor (Qt::CrossCursor));
}

} // namespace v2

