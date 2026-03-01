/* vim:ts=4
 *
 * Copyleft 2013  Michał Gawron
 * Marduk Unix Labs, http://mulabs.org/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Visit http://www.gnu.org/licenses/gpl-3.0.html for more information on licensing.
 */

#ifndef XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__CONFIG_WIDGET_H__INCLUDED
#define XEFIS__CORE__COMPONENTS__MODULE_CONFIGURATOR__CONFIG_WIDGET_H__INCLUDED

// Xefis:
#include <xefis/config/all.h>
#include <xefis/support/ui/widget.h>

// Standard:
#include <cstddef>


namespace xf::configurator {

class ConfigWidget: public xf::Widget
{
  protected:
	static constexpr si::Frequency kDataRefreshRate = 5_Hz;

  protected:
	using Widget::Widget;
};

} // namespace xf::configurator

#endif
