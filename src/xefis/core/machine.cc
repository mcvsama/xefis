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
#include "machine.h"

// Xefis:
#include <xefis/config/all.h>
#include <xefis/core/components/configurator/configurator_widget.h>

// Standard:
#include <cstddef>


namespace xf {

Machine::Machine (Xefis& xefis):
	_xefis (xefis)
{ }


void
Machine::show_configurator()
{
	if (!_configurator_widget)
	{
		_configurator_widget = std::make_unique<ConfiguratorWidget> (*this, nullptr);
		auto lh = default_line_height (_configurator_widget.get());
		_configurator_widget->resize (50 * lh, 30 * lh);
	}

	_configurator_widget->show();
}

} // namespace xf

